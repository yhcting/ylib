#ifndef __YHASh_h__
#define __YHASh_h__

struct yhash;

/**
 * @fcb : callback to free user value(item)
 *        (NULL means, item doesn't need to be freed.)
 */
extern struct yhash*
yhash_create(void(*fcb)(void*));

/**
 * @return : reserved for future use.
 */
extern int
yhash_destroy(struct yhash* h);

/**
 * @return : number of elements in hash.
 */
extern unsigned int
yhash_sz(struct yhash* h);

/**
 * @v      : user value(item)
 * @return : return hash @h - self.
 */
extern struct yhash*
yhash_add(struct yhash* h,
          const unsigned char* key, unsigned int keysz,
          void* v);

/**
 * If these is no matched item, nothing happened.
 * @v      : user value(item)
 * @return : return hash @h - self.
 */
extern struct yhash*
yhash_del(struct yhash* h,
          const unsigned char* key, unsigned int keysz);

/**
 * @return : NULL if fails.
 */
extern void*
yhash_find(struct yhash* h,
           const unsigned char* key, unsigned int keysz);

#endif /* __YHASh_h__ */
