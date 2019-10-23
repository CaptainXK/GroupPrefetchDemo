#include <stdio.h>
#include "hashTool.h"

#define SEED (0x12345678)

uint64_t murmurHash64B (const void * key, int len, uint32_t seed);

uint64_t doHash(const void * key, int len)
{
  assert(key != NULL);

  return murmurHash64B(key, len, SEED);
}

uint64_t murmurHash64B (const void * key, int len, uint32_t seed)   //http://blog.csdn.net/wisage/article/details/7104866
{
	const uint32_t m = 0x5bd1e995;
	const int r = 24;

	uint32_t h1 = seed ^ len;
	uint32_t h2 = 0;

	const uint32_t * data = (const uint32_t *)key;

	while(len >= 8)
	{
		uint32_t k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;

		uint32_t k2 = *data++;
		k2 *= m; k2 ^= k2 >> r; k2 *= m;
		h2 *= m; h2 ^= k2;
		len -= 4;
	}

	if(len >= 4)
	{
		uint32_t k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;
	}

	switch(len)
	{
	case 3: h2 ^= ((unsigned char*)data)[2] << 16;
	case 2: h2 ^= ((unsigned char*)data)[1] << 8;
	case 1: h2 ^= ((unsigned char*)data)[0];
			h2 *= m;
	};

	h1 ^= h2 >> 18; h1 *= m;
	h2 ^= h1 >> 22; h2 *= m;
	h1 ^= h2 >> 17; h1 *= m;
	h2 ^= h1 >> 19; h2 *= m;

	uint64_t h = h1;

	h = (h << 32) | h2;

	return h;
}
