#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>//memset

#include "lookup.h"
#include "hashTool.h"

#define TIMER_INIT()\
        struct timeval begin_t, end_t;\
        uint64_t dura_t;\

#define TIMER_START()\
        gettimeofday(&begin_t, NULL);

#define TIMER_STOP()\
        gettimeofday(&end_t, NULL);\
        dura_t = ((int64_t)end_t.tv_sec * 1000000 + end_t.tv_usec) - ((int64_t)begin_t.tv_sec * 1000000 + begin_t.tv_usec);\
        printf("Speed %.2lfM/s\n", ((double)MAX_TEST / ((double)(dura_t) / 1000000)) / (1<<20) );


//global var
HashBuck g_hashtable[MAX_HASH_TABLE];
Key_t RuleList[MAX_KEY];
Key_t testList[MAX_TEST];
uint8_t portList[MAX_TEST];

//init hash table and test data
void init()
{
  uint64_t fpt;
  uint64_t index;
  int idx;

  //init random number seed
  srand(time(NULL));

  //clear hash table
  for(int i = 0; i < MAX_HASH_TABLE; ++i){
    g_hashtable[i].m_entry = NULL;
  }
  
  //init rule
  for(int i = 0 ; i < MAX_KEY; ++i){
    RuleList[i].m_key = (uint64_t)rand();
    RuleList[i].m_len = sizeof(RuleList[i].m_key);

    //insert into hash table
    fpt = doHash(&(RuleList[i].m_key), RuleList[i].m_len);
    index = fpt % MAX_HASH_TABLE;
    if(g_hashtable[index].m_entry == NULL){
      g_hashtable[index].m_entry = (BucketEntry*)malloc(sizeof(BucketEntry));
    }
    g_hashtable[index].m_entry->m_fpt = fpt;
    g_hashtable[index].m_entry->m_port = 1U;
  }

  //init test data
  for(int i = 0; i < MAX_TEST; ++i){
    idx = rand() % MAX_KEY; 
    testList[i].m_key = RuleList[idx].m_key;
    testList[i].m_len = RuleList[idx].m_len;
  }
}

#define BATCH (32)
void benchmark(Find findFunc, const char * debugStr){
  printf("[%s Test]\n", debugStr);
  int i = 0;

  //flush cache
  flushCache();
  TIMER_INIT();
  
  TIMER_START();
  for(i = 0; i < MAX_TEST; i += BATCH){
    //printf("Batch#%d...", i);
    findFunc(testList + i, BATCH, portList + i);
    //printf("DONE!\n");
  }
  TIMER_STOP();
}

/*
 * lookup one by one
 * */
int findNormal(const Key_t * key, int len, uint8_t * ports)
{
  int i = 0, ret = 0;
  
  for(i = 0 ; i < len; ++i){
    int idx = doHash( &(key[i].m_key), key[i].m_len) % MAX_HASH_TABLE;
    BucketEntry * bucket_t = g_hashtable[idx].m_entry;

    if(bucket_t != NULL ){
      ports[i] = bucket_t->m_port;
      ret += 1;
    }
    else{
      ports[i] = NOT_FOUND;
    }
  }

  return ret;
}

/*
 * Group prefetch lookup
 * */
int findGprefetch(const Key_t * key, int len, uint8_t * ports)
{
  int * idxs = (int*)malloc(len * sizeof(int));
  int i, ret = 0;
  BucketEntry ** buckets = (BucketEntry**)malloc(len * sizeof(BucketEntry*));

  assert(idxs != NULL);
  assert(buckets != NULL);

  //stage 1
  for(i = 0; i < len; ++i){
    idxs[i] = doHash( &(key[i].m_key), key[i].m_len) % MAX_HASH_TABLE;
    PREFETCH(&(g_hashtable[idxs[i]]));
  }

  //stage 2
  for(i = 0; i < len; ++i){
    buckets[i] = g_hashtable[idxs[i]].m_entry;
    PREFETCH(buckets[i]);
  }

  //stage 3
  for(i =0 ; i < len; ++i){
    if(buckets[i] != NULL){
      ports[i] = buckets[i]->m_port;
      ret += 1;
    }
    else{
      ports[i] = NOT_FOUND;
    }
  }

  return 0;
}

/*
 * G-opt lookup
 * */
#define G_PSS(addr, label) do{\
          PREFETCH(addr);\
          g_labels[I] = &&label;\
          I = (I + 1) % len;\
          goto *g_labels[I];\
        }while(0);

#define SET_BIT(mask, idx)  ((uint64_t)(mask) | (1U << idx))

int findGopt(const Key_t * key, int len, uint8_t * ports)
{
  int ret = 0;
  int * idxs = (int*)malloc(len * sizeof(int));
  BucketEntry ** buckets = (BucketEntry**)malloc(len * sizeof(BucketEntry*));
  assert(idxs != NULL);
  assert(buckets != NULL);

  //G-opt control variables
  int I = 0;
  uint64_t g_mask = 0;
  uint64_t end_mask = (((uint64_t)(1) << len) -1 );
  //void **g_labels = (void**)malloc(len * sizeof(void*));
  void *g_labels[len];

  for(int i = 0 ; i < len; ++i){
    g_labels[i] = &&g_label_0;
  }

g_label_0:
  idxs[I] = doHash(&(key[I].m_key), key[I].m_len) % MAX_HASH_TABLE;
  G_PSS(&g_hashtable[idxs[I]], g_label_1);

g_label_1:
  buckets[I] = g_hashtable[idxs[I]].m_entry;
  if(buckets[I] != NULL){
    //printf("%dth pkt...\n", I);
    ret += 1;
    G_PSS(buckets[I], g_label_2);

g_label_2:
    ports[I] = buckets[I]->m_port;
  } else {
    ports[I] = NOT_FOUND;
  }

g_end:
  g_labels[I] = &&g_end;
  g_mask = SET_BIT(g_mask, I);
  if( g_mask == end_mask){
    //printf("mask = %lu, target = %lu, DONE!\n", g_mask, end_mask);
    goto END;
  }
  //else{
  //  printf("mask = %lu, target = %lu, go on\n", g_mask, end_mask);
  //}

  I = (I + 1) % len;
  goto *g_labels[I];

END:
  return ret;
}

