#include <memory.h>

#include "yhash.h"
#include "ylistl.h"
/* crc is used as hash function */
#include "ycrc.h"

#define _MAX_HBITS 32
#define _MIN_HBITS 4

/* hash node */
struct _hn {
    struct ylistl_link    lk;
    unsigned char*        key;   /* full key */
    unsigned int          keysz; /* size of key */
    unsigned int          hv32;  /* 32bit hash value */
    void*                 v;     /* user value */
};

struct yhash {
    struct ylistl_link*   map;
    unsigned int          sz;          /* hash size */
    unsigned char         mapbits;     /* bits of map table = 1<<mapbits */
    void                (*fcb)(void*); /* free callback */
};

static inline unsigned int
_hmapsz(const struct yhash* h) {
    return (1<<h->mapbits);
}

static inline unsigned int
_hv__(unsigned int mapbits, unsigned int hv32) {
    return hv32 >> (32 - mapbits);
}

static inline unsigned int
_hv_(const struct yhash* h, unsigned int hv32) {
    return _hv__(h->mapbits, hv32);
}

static inline unsigned int
_hv(const struct yhash* h, const struct _hn* n) {
    return _hv_(h, n->hv32);
}


/* Modify hash space to 'bits'*/
static struct yhash*
_hmodify(struct yhash* h, unsigned int bits) {
    int                 i;
    struct _hn         *n, *tmp;
    struct ylistl_link* oldmap;
    unsigned int        oldmapsz;

    if (bits < _MIN_HBITS) bits = _MIN_HBITS;
    if (bits > _MAX_HBITS) bits = _MAX_HBITS;

    if (h->mapbits == bits) return h; /* nothing to do */
    
    oldmap = h->map;
    oldmapsz = _hmapsz(h);

    h->mapbits = bits; /* map size is changed here */
    h->map = ymalloc(sizeof(struct ylistl_link) * _hmapsz(h));
    yassert(h->map);
    for (i=0; i<_hmapsz(h); i++) ylistl_init_link(&h->map[i]);
    /* re assign hash nodes */
    for (i=0; i<oldmapsz; i++) {
        ylistl_foreach_item_removal_safe(n, tmp, &oldmap[i], struct _hn, lk) {
            ylistl_del(&n->lk);
            ylistl_add_last(&h->map[_hv(h, n)], &n->lk);
        }
    }
    yfree(oldmap);
    return h;
}

static struct _hn*
_hfind(const struct yhash* h, const unsigned char* key, unsigned int keysz) {
    struct _hn*          n;
    unsigned int         hv32 = ycrc32(0, key, keysz);
    struct ylistl_link*  hd = &h->map[_hv_(h, hv32)];
    ylistl_foreach_item(n, hd, struct _hn, lk) {
        if ( keysz == n->keysz
             && n->hv32 == hv32
             && 0 == memcmp(key, n->key, keysz) ) break;
    }
    return (&n->lk == hd)? NULL: n;
}


static struct _hn*
_ncreate(const unsigned char* key, unsigned int keysz, void* v) {
    struct _hn* n = ymalloc(sizeof(struct _hn));
    yassert(n);
    n->key = ymalloc(keysz);
    yassert(n->key);
    memcpy(n->key, key, keysz);
    n->keysz = keysz;
    n->hv32 = ycrc32(0, key, keysz);
    n->v = v;
    ylistl_init_link(&n->lk);
    return n;
}

static void
_ndestroy(const struct yhash* h, struct _hn* n) {
    yassert(n->key && n->keysz>0);
    yfree(n->key);
    if (h->fcb) (*h->fcb)(n->v);
    yfree(n);
}

struct yhash*
yhash_create(void(*fcb)(void*)) {
    int           i;
    struct yhash* h = ymalloc(sizeof(struct yhash));
    yassert(h);
    h->sz = 0;
    h->mapbits = _MIN_HBITS;
    h->map = (struct ylistl_link*)ymalloc(sizeof(struct ylistl_link) * _hmapsz(h));
    yassert(h->map);
    for (i=0; i<_hmapsz(h); i++) ylistl_init_link(&h->map[i]);
    h->fcb = fcb;
    return h;
}

int
yhash_destroy(struct yhash* h) {
    int          i;
    struct _hn  *n, *tmp;
    for (i=0; i<_hmapsz(h); i++) {
        ylistl_foreach_item_removal_safe(n, tmp, &h->map[i], struct _hn, lk) {
            ylistl_del(&n->lk);
            _ndestroy(h, n);
        }
    }
    yfree(h->map);
    yfree(h);
    return 0;
}

unsigned int
yhash_sz(struct yhash* h) {
    return h->sz;
}

struct yhash*
yhash_add(struct yhash* h,
          const unsigned char* key, unsigned int keysz,
          void* v) {
    struct _hn* n;
    /* we need to expand hash map size if hash seems to be full */
    if (h->sz > _hmapsz(h)) _hmodify(h, h->mapbits+1);

    n = _ncreate(key, keysz, v);
    ylistl_add_last(&h->map[_hv(h, n)], &n->lk);
    h->sz++;
    return h;
}

struct yhash*
yhash_del(struct yhash* h,
          const unsigned char* key, unsigned int keysz) {
    struct _hn* n = _hfind(h, key, keysz);
    if (n) {
        ylistl_del(&n->lk);
        _ndestroy(h, n);
        h->sz--;
        if ( h->sz < _hmapsz(h)/4 ) _hmodify(h, h->mapbits-1);
    }
    return h;
}

void*
yhash_find(struct yhash* h,
           const unsigned char* key, unsigned int keysz) {
    struct _hn* n = _hfind(h, key, keysz);
    return n? n->v: NULL;
}
