#ifndef _LOOKUP_
#define _LOOKUP_

#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

#define MAX_HASH_TABLE (1<<20)
#define MAX_KEY (1<<20)
#define MAX_TEST (1<<14)
#define NOT_FOUND (2U)

#define PREFETCH(x) __builtin_prefetch(x)

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

//find function
typedef int (*Find)(const Key_t * key, int len, uint8_t * ports);

void init();
void benchmark(Find findFunc, const char * debugStr);
int findNormal(const Key_t * key, int len, uint8_t * ports);
int findGprefetch(const Key_t * key, int len, uint8_t * ports);

//flush all cache
static inline void flushCache()
{
  FILE *fp = fopen("/proc/sys/vm/drop_caches", "w");
  assert(fp != NULL);

  fprintf(fp, "3");

  fclose(fp);
}


#endif
