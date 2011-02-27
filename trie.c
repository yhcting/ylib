/*
 * To store global symbols, Trie is used as an main data structure.
 * But, having 256 pointers for every node is heavy-memory-burden.
 * So, we choose 4-bit-way; One ASCII character(8 bit) is represented
 *  by 2 node (4bit + 4bit) - front and back node.
 * We can also get sorted result from Trie.
 * Why Trie is chosen (versus Hash, Tree etc)?
 * Pros
 *    - To search symbol is fast.
 *    - It is efficient(in terms of performance)
 *       and easy to implement Auto-Completion.
 * Cons
 *    - Memory overhead.
 *
 * Implementation example
 *
 * ex. symbol '4' has 'DATA'
 *    '4' == 0x34 == 0011 0100b
 * +---+----+--
 * |R  |0000| ^
 * |o  +----+ |
 * |o   ...   | 16 pointers
 * |t  +----+ |      <front node>
 * |   |0011|-|----> +---+----+
 * |   +----+ |      |    ... |
 * |    ...   v      |   +----+      <back node>
 * +---+----+---     |   |0100|----> +---+----+
 *                   |   +----+      | D |0000|
 *                   |    ...        | A +----+
 *                                   | T | ...|
 *                                   | A |    |
 *                                   +---+----+
 */
#include <memory.h>
#include <string.h>

#include "common.h"
#include "ytrie.h"


/* use 4 bit trie node */
struct _node {
	/*
	 * We need two node to represent 1 ASCII character!
	 * (If 8bit is used, every node should have 256 pointers.
	 *   And this is memory-burden.
	 *  4bits-way (16 pointers-way) is good choice, in my opinion.)
	 */
	struct _node*  n[16];
	void*          v;
};

struct ytrie {
	struct _node          rt;     /* root. sentinel */
	void           (*fcb)(void*); /* callback for free value */
};

static inline struct _node*
_alloc_node() {
	struct _node* n = ymalloc(sizeof(struct _node));
	if (!n)
		/*
		 * if allocing such a small size of memory fails,
		 *  we can't do anything!
		 * Just ASSERT IT!
		 */
		yassert(0);
	memset(n, 0, sizeof(struct _node));
	return n;
}

static inline void
_free_node(struct _node* n, void(*fcb)(void*) ) {
	if (n->v)
		if (fcb)
			(*fcb)(n->v);
	yfree(n);
}

static inline void
_delete_node(struct _node* n, void(*fcb)(void*) ) {
	register int i;
	for (i=0; i<16; i++)
		if (n->n[i])
			_delete_node(n->n[i], fcb);
	_free_node(n, fcb);
}

static inline int
_delete_empty_leaf(struct _node* n) {
	register int i;
	/*
	 * Check followings
	 *    This node doens't have 'value'
	 *    This node is leaf.
	 */
	for (i=0; i<16; i++)
		if (n->n[i])
			break;
	if (i >= 16 /* Is this leaf? */
	    && !n->v) { /* Doesn't this have 'value'? */
		/* there is no 'value'. So, free callback is not required */
		_free_node(n, NULL);
		return 1; /* true */
	} else
		return 0;/* false */
}

/*
 * @return : see '_delete_key(...)'
 */
static inline int
_delete_empty_byte_leaf(struct _node* n, unsigned char c) {
	register int  fi = c>>4;
	register int  bi = c&0x0f;
	if (_delete_empty_leaf(n->n[fi]->n[bi])) {
		/*
		 * node itself is deleted!
		 * So, front node should also be checked!
		 */
		n->n[fi]->n[bi] = NULL;
		if (_delete_empty_leaf(n->n[fi])) {
			n->n[fi] = NULL;
			/* this node itself should be checked! */
			return 1;
		} else
			return 0; /* all done */
	} else
		return 0; /* all done */
}

/*
 * @return:
 *     1 : needed to be checked that the node @t is empty leaf or not!
 *     0 : all done successfully
 *     -1: cannot find node(symbol is not in trie)
 */
static int
_delete_key(struct _node* n, const unsigned char* p,
            unsigned int sz, void(*fcb)(void*)) {
	register int         fi = *p>>4;
	register int         bi = *p&0x0f;
	if (sz>0) {
		/* check that there is front & back node */
		if (n->n[fi] && n->n[fi]->n[bi]) {
			switch(_delete_key(n->n[fi]->n[bi], p+1, sz-1, fcb)) {
			case 1:    return _delete_empty_byte_leaf(n, *p);
			case 0:    return 0;
			}
		}
		/* Cannot find symbol! */
		return -1;
	} else {
		/*
		 * End of stream.
		 * We need to check this node has valid data.
		 * If yes, delete value!
		 * If not, symbol name is not valid one!
		 */
		if (n->v) {
			if (fcb)
				(*fcb)(n->v);
			n->v = NULL;
			return 1;
		} else
			return -1;
	}
}

/**
 * @c:     (char)     character
 * @n:     (struct _node*) node to move down
 * @fi:    (int)      variable to store front index
 * @bi:    (int)      variable to store back index
 * @felse: <code>     code in 'else' brace of front node
 * @belse: <code>     code in 'else' brace of back node
 */
#define _move_down_node(c, n, fi, bi, felse, belse)     \
	do {						\
		(fi) = (c)>>4; (bi) = (c)&0x0f;		\
		if ((n)->n[fi])				\
			(n) = (n)->n[fi];		\
		else {					\
			felse;				\
		}					\
		if ((n)->n[bi])				\
			(n) = (n)->n[bi];		\
		else {					\
			belse;				\
		}					\
	} while (0)
/*
 * if node exists, it returns node. If not, it creates new one and returns it.
 * @return: back node of last character of symbol.
 *          If fails to get, NULL is returned.
 */
static struct _node*
_get_node(struct ytrie* t,
          const unsigned char* key, unsigned int sz,
          int bcreate) {
	register struct _node*             n = &t->rt;
	register const unsigned char* p = key;
	register const unsigned char* pend = p+sz;
	register int                  fi, bi; /* front index */

	if (bcreate) {
		while (p<pend) {
			_move_down_node(*p, n, fi, bi,
					n = n->n[fi] = _alloc_node(),
					n = n->n[bi] = _alloc_node());
			p++;
		}
	} else {
		while (p<pend) {
			_move_down_node(*p, n, fi, bi,
					return NULL, return NULL);
			p++;
		}
	}
	return n;
}

static int
_equal_internal(const struct _node* n0, const struct _node* n1,
		int(*cmp)(const void*, const void*)) {
	register int i;
	if (n0->v && n1->v) {
		if (0 == cmp(n0->v, n1->v))
			return 0; /* NOT same */
	} else if (n0->v || n1->v)
		return 0; /* NOT same */

	/* compare child nodes! */
	for (i=0; i<16; i++) {
		if (n0->n[i] && n1->n[i]) {
			if (0 == _equal_internal(n0->n[i], n1->n[i], cmp))
				return 0; /* NOT same */
		} else if (n0->n[i] || n1->n[i])
			return 0; /* NOT same */
	}
	return 1; /* n0 and n1 is same */
}

static struct _node*
_node_clone(const struct _node* n,
	    void* user,
	    void*(*clonev)(void*, const void*)) {
	register int i;
	struct _node* r = _alloc_node();
	if (n->v)
		r->v = clonev(user, n->v);
	for (i=0; i<16; i++)
		if (n->n[i])
			r->n[i] = _node_clone(n->n[i], user, clonev);
	return r;
}

/*
 * @return: see '_walk_node'
 */
static int
_walk_internal(void* user, struct _node* n,
               int(cb)(void*, const unsigned char*, unsigned int,void*),
	       /* bsz: excluding space for trailing 0 */
               unsigned char* buf, unsigned int bsz,
               unsigned int bitoffset) {
	register unsigned int i;

	if (n->v) {
		/* value should exists at byte-based-node */
		yassert(0 == bitoffset%8);
		/* if (buf) { buf[bitoffset>>3] = 0 ; } */
		/* keep going? */
		if (!(*cb)(user, buf, bitoffset/8, n->v))
			return 0;
	}

	for (i=0; i<16; i++) {
		if (n->n[i]) {
			int   r;
			if (buf) {
				unsigned char* p =  buf + (bitoffset>>3);
				/* check that buffer is remained enough */
				if (bitoffset>>3 == bsz)
					/* not enough buffer.. */
					return -1;

				if (!(bitoffset%8))
					/*
					 * multiple of 8
					 * - getting index for front node
					 */
					*p = (i<<4);
				else {
					/*
					 * multiple of 4
					 * - getting index for back node
					 */
					*p &= 0xf0;
					*p |= i;
				}
			}
			/* go to next depth */
			r = _walk_internal(user,
					   n->n[i],
					   cb,
					   buf,
					   bsz,
					   bitoffset + 4);
			if (r<=0)
				return r;
		}
	}
	return 1;
}

void**
ytrie_getref(struct ytrie* t,
	     const unsigned char* key, unsigned int sz) {
	struct _node* n;
	yassert(key);
	if (0 == sz)
		return NULL; /* 0 length string */
	n = _get_node(t, key, sz, FALSE);
	return n? &n->v: NULL;
}

void*
ytrie_get(struct ytrie* t, const unsigned char* key, unsigned int sz) {
	void** pv = ytrie_getref(t, key, sz);
	return pv? *pv: NULL;
}

int
ytrie_walk(struct ytrie* t, void* user,
	   const unsigned char* from, unsigned int fromsz,
	   /* return 1 for keep going, 0 for stop and don't do anymore */
	   int(cb)(void*, const unsigned char*, unsigned int, void*)) {
	char             buf[YTRIE_MAX_KEY_LEN + 1];
	struct _node*    n = _get_node(t, from, fromsz, FALSE);

	yassert(t && from);
	if (n)
		return _walk_internal(user,
				      n,
				      cb,
				      (unsigned char*)buf,
				      YTRIE_MAX_KEY_LEN,
				      0);
	else
		return -1;
}

int
ytrie_insert(struct ytrie* t,
	     const unsigned char* key,
	     unsigned int sz, void* v) {
	struct _node*  n;

	yassert(t && key);
	if (0 == sz)
		return -1; /* 0 length symbol */
	if (!v || (sz >= YTRIE_MAX_KEY_LEN) )
		return -1; /* error case */

	n = _get_node(t, key, sz, TRUE);
	if (n->v) {
		if (t->fcb)
			(*t->fcb)(n->v);
		n->v = v;
		return 1; /* overwritten */
	} else {
		n->v = v;
		return 0; /* newly created */
	}
}

struct ytrie*
ytrie_create(void(*fcb)(void*)) {
	struct ytrie* t = ymalloc(sizeof(struct ytrie));
	yassert(t);
	memset(t, 0, sizeof(*t));
	t->fcb = fcb;
	return t;
}

void
ytrie_clean(struct ytrie* t) {
	register int i;
	for (i=0; i<16; i++) {
		if (t->rt.n[i]) {
			_delete_node(t->rt.n[i], t->fcb);
			t->rt.n[i] = NULL;
		}
	}
}

void
ytrie_destroy(struct ytrie* t) {
	ytrie_clean(t);
	yfree(t);
}


int
ytrie_delete(struct ytrie* t, const unsigned char* key, unsigned int sz) {
	yassert(t && key);
	switch(_delete_key(&t->rt, key, sz, t->fcb)) {
        case 1:
        case 0:    return 0;
        default:   return -1;
	}
}

void(*ytrie_fcb(const struct ytrie* t))(void*) {
	return t->fcb;
}

int
ytrie_equal(const struct ytrie* t0, const struct ytrie* t1,
	    int(*cmp)(const void*, const void*)) {
	return _equal_internal(&t0->rt, &t1->rt, cmp);
}

int
ytrie_copy(struct ytrie* dst, const struct ytrie* src, void* user,
	   void*(*clonev)(void*,const void*)) {
	register int i;
	ytrie_clean(dst);
	dst->fcb = src->fcb;
	for (i=0; i<16; i++) {
		if (src->rt.n[i])
			dst->rt.n[i] = _node_clone(src->rt.n[i], user, clonev);
	}
	/* return value of this function is reserved for future use */
	return 0;
}

struct ytrie*
ytrie_clone(const struct ytrie* t,
	    void* user,
	    void*(*clonev)(void*, const void*)) {
	struct ytrie*    r = ytrie_create(t->fcb);
	ytrie_copy(r, t, user, clonev);
	return r;
}

int
ytrie_auto_complete(struct ytrie* t,
		    const unsigned char* start_with, unsigned int sz,
		    unsigned char* buf, unsigned int bufsz) {
#define YTRIEBranch 0
#define YTRIELeaf   1
#define YTRIEFail   2

	int                   ret = -1;
	register struct _node*     n;
	register unsigned int i;
	unsigned int          j, cnt, bi;
	unsigned char         c;

	yassert(t && start_with && buf);

	/* move to prefix */
	n = _get_node(t, start_with, sz, FALSE);
	if (!n)
		goto bail;

	/* find more possbile prefix */
	bi = 0;
	while (n) {
		cnt = j = c = 0;
		for (i=0; i<16; i++)
			if (n->n[i]) {
				cnt++;
				j=i;
			}

		if (cnt > 1 || ((1 == cnt) && n->v)) {
			ret = YTRIEBranch;
			break;
		} else if (0 == cnt) {
			yassert(n->v);
			ret = YTRIELeaf;
			break;
		}
		c = j<<4;
		n = n->n[j];

		/* Sanity check! */
		/* we don't expect that this can have value */
		yassert(!n->v);

		cnt = j = 0;
		for (i=0; i<16; i++) {
			if (n->n[i]) {
				cnt++;
				j=i;
			}
		}
		if (cnt > 1) {
			ret = YTRIEBranch;
			break;
		} else if (0 == cnt)
			/* This is unexpected case on this trie algorithm */
			yassert(0);
		c |= j;
		n = n->n[j];

		if (bi >= (bufsz-1))
			return -1;
		else {
			buf[bi] = c;
			bi++;
		}
	}

	buf[bi] = 0; /* add trailing 0 */
	return ret;

 bail:
	return YTRIEFail;

#undef YTRIEBranch
#undef YTRIELeaf
#undef YTRIEFail

}
