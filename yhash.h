#ifndef __YHASh_h__
#define __YHASh_h__

#include "yret.h"

struct yhash;

/**
 * @fcb : callback to free user value(item)
 *	  (NULL means, item doesn't need to be freed.)
 */
EXPORT struct yhash*
yhash_create(void(*fcb)(void*));

/**
 * @return : reserved for future use.
 */
EXPORT enum yret
yhash_destroy(struct yhash* h);

/**
 * @return : number of elements in hash.
 */
EXPORT unsigned int
yhash_sz(struct yhash* h);

/**
 * @v	   : user value(item)
 * @return : return hash @h - self.
 */
EXPORT struct yhash*
yhash_add(struct yhash* h,
	  const unsigned char* key, unsigned int keysz,
	  void* v);

/**
 * If these is no matched item, nothing happened.
 * @v	   : user value(item)
 * @return : return hash @h - self.
 */
EXPORT struct yhash*
yhash_del(struct yhash* h,
	  const unsigned char* key, unsigned int keysz);

/**
 * @return : NULL if fails.
 */
EXPORT void*
yhash_find(struct yhash* h,
	   const unsigned char* key, unsigned int keysz);

#endif /* __YHASh_h__ */
