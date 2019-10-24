#ifndef _LOOKUP_
#define _LOOKUP_

#include <inttypes.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>//exit()
#include <string.h>//strerror

#define MAX_HASH_TABLE (1<<20)
#define MAX_KEY (1<<20)
#define MAX_TEST (1<<14)
#define NOT_FOUND (2U)

#define PREFETCH(x) __builtin_prefetch(x)

#define DOASSERT(x)\
        if((x) == false){\
          printf("Error (%s:%s():%d):%s\n", __FILE__, __func__, __LINE__, strerror(errno));\
          exit(1);\
        }

//hash table bucket entry
struct BucketEntry{
  uint64_t m_fpt;
  uint8_t m_port; 
}__attribute__((packed));
typedef struct BucketEntry BucketEntry;

//hash bucket
struct HashBuck{
  BucketEntry * m_entry;
}__attribute__((packed));
typedef struct HashBuck HashBuck;

//Key
struct Key_t{
  uint64_t m_key;
  int m_len;
}__attribute__((packed));
typedef struct Key_t Key_t;

//find function adapter
typedef int (*Find)(const Key_t * key, int len, uint8_t * ports);

//benchmark API
void benchmark(Find findFunc, const char * debugStr);

//init hash table, rule list and test data list
void init();

//lookup api
int findNormal(const Key_t * key, int len, uint8_t * ports);//normal version
int findGprefetch(const Key_t * key, int len, uint8_t * ports);//group prefetch version
int findGopt(const Key_t * key, int len, uint8_t * ports);//G-opt version

//flush all cache
static inline void flushCache()
{
  FILE *fp = fopen("/proc/sys/vm/drop_caches", "w");
  DOASSERT(fp != NULL);

  fprintf(fp, "3");

  fclose(fp);
}


#endif
