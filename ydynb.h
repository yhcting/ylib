#ifndef __YDYNb_h__
#define __YDYNb_h__

#include <memory.h>

#include "ydef.h"

/* DYNmaic Buffer */
typedef struct ydynb {
    unsigned int    limit;
    unsigned int    sz;
    unsigned char*  b;
}

static inline unsigned int
ydynb_limit(const struct ydynb* b) {
    return b->limit;
}

static inline unsigned int
ydynb_freesz(const struct ydynb* b) {
    return b->limit - b->sz;
}

static inline unsigned int
ydynb_sz(const struct ydynb* b) {
    return b->sz;
}

static inline unsigned char*
ydynb_buf(const struct ydynb* b) {
    return b->b;
}

static inline unsigned char*
ydynb_ptr(const struct ydynb* b) {
    return b->b + b->sz;
}

/*
 * @return: 0 if success.
 */
static inline int
ydynb_init(struct ydynb* b, unsigned int init_limit) {
    b->sz = 0;
    b->b = (unsigned char*)ymalloc(init_limit);
    if(b->b) { b->limit = init_limit; return 0; }
    else { b->limit = 0; return -1; }
}

static inline void
ydynb_reset(struct ydynb* b) {
    b->sz = 0;
}

static inline void
ydynb_clean(struct ydynb* b) {
    if(b->b) { yfree(b->b); }
    b->b = NULL;
    b->limit = b->sz = 0;
}

/*
 * increase buffer size by two times.
 * due to using memcpy, it cannot be static inline
 * @return: <0 if fails.
 */
static inline int
ydynb_expand(struct ydynb* b) {
    unsigned char* tmp = (unsigned char*)ymalloc(b->limit*2);
    if(tmp) {
        memcpy(tmp, b->b, b->sz);
        yfree(b->b);
        b->b = tmp;
        b->limit *= 2;
        return 0;
    } else {
        return -1;
    }
}

static inline int
ydynb_secure(struct ydynb* b, unsigned int sz_required) {
    while( sz_required > ydynb_freesz(b)
           && !ydynb_expand(b) ) {}
    return sz_required <= ydynb_freesz(b);
}


static inline int
ydynb_shrink(struct ydynb* b, unsigned int sz_to) {
    if( b->limit > sz_to  && b->sz < sz_to ) {
        unsigned char* tmp = (unsigned char*)ymalloc(sz_to);
        if(tmp) {
            yassert(b->b);
            memcpy(tmp, b->b, b->sz);
            yfree(b->b);
            b->b = tmp;
            b->limit = sz_to;
            return 0;
        }
    }
    return -1;
}

static inline int
ydynb_append(struct ydynb* b, const unsigned char* d, unsigned int dsz) {
    if( 0 > ydynb_secure(b, dsz) ) { return -1; }
    memcpy(ydynb_ptr(b), d, dsz);
    b->sz += dsz;
    return 0;
}

#endif /* __YDYNb_h__ */
