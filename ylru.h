#ifndef __YLRu_h__
#define __YLRu_h__

#include "ydef.h"

struct ylru;

struct ylru_cb {
	/* free user data evicted from cache. */
	void  (*free)(void *);
	/* create data if cache miss */
	void *(*create)(unsigned int *data_size, /* size of created data */
			const void *key, unsigned int keysz);
};

EXPORT struct ylru *
ylru_create(unsigned int maxsz,
	    const struct ylru_cb *cbs);

/**
 * Make cache empty.
 * Cache itself is NOT destroied.
 */
EXPORT void
ylru_clean(struct ylru *);

EXPORT void
ylru_destroy(struct ylru *);

/**
 * @data_size: size of user defined unit. NOT memory size of data.
 *     This value is used to calculate lru cache size.
 * @return: 0 for success, otherwise error number
 */
EXPORT int
ylru_put(struct ylru *,
	 const void *key, unsigned int keysz,
	 void *data, unsigned int data_size);

/**
 * NOTE: value is remove from cache!
 * So, after using it, user SHOULD put it again for future use.
 * @data_size: NULL is ignored.
 * @return: NULL if cache miss and fail to create new data by 'ylru_cb.create'
 */
EXPORT void *
ylru_get(struct ylru *,
	 unsigned int *data_size, /* returned data size */
	 const void *key, unsigned int keysz);

/**
 * Get size of cached data.
 * This is based on 'size' of 'ylru_put'.
 */
EXPORT unsigned int
ylru_sz(struct ylru *);


#endif /* __YLRu_h__ */