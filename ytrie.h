/*
 * Trie is tested at ylisp.
 * So, there is no specific test file....
 */

#ifndef __YTRIe_h__
#define __YTRIe_h__

#define YTRIE_MAX_KEY_LEN  1024

struct ytrie;

/**
 * get pointer of element.
 * (To replace element directly! - Only for performance reason.)
 */
extern void**
ytrie_getref(struct ytrie* t,
             const unsigned char* key, unsigned int sz);

/**
 * get element
 */
extern void*
ytrie_get(struct ytrie* t, const unsigned char* key, unsigned int sz);


/**
 * walk trie nodes.
 * @cb : callback that is called whenever walker visits element.
 *       If @cb returns 1, walker keeps going.
 *       But if @cb returns 0, walker stops and 'ytrie_walk' is done.
 */
extern int
ytrie_walk(struct ytrie* t, void* user,
            const unsigned char* from, unsigned int fromsz,
            /* return 1 for keep going, 0 for stop and don't do anymore */
           int(cb)(void*, const unsigned char*, unsigned int, void*));

/**
 * insert element
 */
extern int
ytrie_insert(struct ytrie* t, const unsigned char* key, unsigned int sz, void* v);

/**
 * @fcb : callback to free element.
 */
extern struct ytrie*
ytrie_create(void(*fcb)(void*));

/**
 * just clean contents of trie.
 */
extern void
ytrie_clean(struct ytrie*);

/**
 * trie itself is destroyed
 */
extern void
ytrie_destroy(struct ytrie*);

/**
 * delete node.
 */
extern int
ytrie_delete(struct ytrie*, const unsigned char* key, unsigned int sz);

/**
 * get free callback of trie
 */
extern void(*
ytrie_fcb(const struct ytrie*))(void*);

/**
 * @cmp : callback to compare element.
 *        (return value should be follows the one of 'strcmp')
 */
extern int
ytrie_equal(const struct ytrie*, const struct ytrie*,
            int(*cmp)(const void*, const void*));

/**
 * copy trie - deep copy
 */
extern int
ytrie_copy(struct ytrie* dst, const struct ytrie* src, void* user,
           void*(*clonev)(void*,const void*));

/**
 * @clonev : callback function that clones element.
 */
extern struct ytrie*
ytrie_clone(const struct ytrie*,
            void* user, void*(*clonev)(void*, const void*));

/**
 * @return
 *    0 : meets the branch. (there are more than one candidates
 *         that starts with @start_with)
 *    1 : meets leaf. (there is only one candidate)
 *    2 : fails. (ex. there is no candidates)
 */
extern int
ytrie_auto_complete(struct ytrie*,
                     const unsigned char* start_with, unsigned int sz,
                    unsigned char* buf, unsigned int bufsz);

#endif /* __YTRIe_h__ */
