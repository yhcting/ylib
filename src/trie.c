/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015
 * Younghyung Cho. <yhcting77@gmail.com>
 * All rights reserved.
 *
 * This file is part of ylib
 *
 * This program is licensed under the FreeBSD license
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of the FreeBSD Project.
 *****************************************************************************/


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
#include <errno.h>

#include "common.h"
#include "ytrie.h"


/* use 4 bit trie node */
struct node {
	/*
	 * We need two node to represent 1 ASCII character!
	 * (If 8bit is used, every node should have 256 pointers.
	 *   And this is memory-burden.
	 *  4bits-way (16 pointers-way) is good choice, in my opinion.)
	 */
	struct node *n[16];
	void *v;
};

struct ytrie {
	struct node rt; /* root. sentinel */
	void (*vfree)(void *); /* callback for free value */
};

static INLINE struct node *
alloc_node(void) {
	struct node *n = ycalloc(1, sizeof(*n));
	if (unlikely(!n))
		/*
		 * if allocing such a small size of memory fails,
		 *  we can't do anything!
		 * Just ASSERT IT!
		 */
		return NULL;
	return n;
}

static INLINE void
free_node(struct node *n, void(*vfree)(void *), int recursive) {
	if (recursive) {
		register int i;
		for (i = 0; i < 16; i++)
			if (n->n[i])
				free_node(n->n[i], vfree, TRUE);
	}
	if (n->v && vfree)
		(*vfree)(n->v);
	yfree(n);
}

static INLINE int
remove_empty_leaf(struct node *n) {
	register int i;
	/*
	 * Check followings
	 *    This node doens't have 'value'
	 *    This node is leaf.
	 */
	for (i = 0; i < 16; i++)
		if (n->n[i])
			break;
	if (i >= 16 /* Is this leaf? */
	    && !n->v) { /* Doesn't this have 'value'? */
		/* there is no 'value'. So, free callback is not required */
		free_node(n, NULL, FALSE);
		return 1; /* true */
	} else
		return 0;/* false */
}

static INLINE int
remove_empty_byte_leaf(struct node *n, u8 c) {
	register int fi = c >> 4;
	register int bi = c & 0x0f;
	if (remove_empty_leaf(n->n[fi]->n[bi])) {
		/*
		 * node itself is removed!
		 * So, front node should also be checked!
		 */
		n->n[fi]->n[bi] = NULL;
		if (remove_empty_leaf(n->n[fi])) {
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
remove_key(struct node *n, const u8 *p,
	   u32 sz, void(*vfree)(void *)) {
	register int fi = *p >> 4;
	register int bi = *p & 0x0f;
	if (sz > 0) {
		/* check that there is front & back node */
		if (n->n[fi] && n->n[fi]->n[bi]) {
			switch(remove_key(n->n[fi]->n[bi],
					  p + 1,
					  sz - 1,
					  vfree)) {
			case 1:
				return remove_empty_byte_leaf(n, *p);
			case 0:
				return 0;
			}
		}
		/* Cannot find symbol! */
		return -1;
	} else {
		/*
		 * End of stream.
		 * We need to check this node has valid data.
		 * If yes, remove value!
		 * If not, key is not valid one!
		 */
		if (n->v) {
			if (vfree)
				(*vfree)(n->v);
			n->v = NULL;
			return 1;
		} else
			return -1;
	}
}

/*
 * @param c (char) character
 * @param n (struct node *) node to move down
 * @param fi (int) variable to store front index
 * @param bi (int) variable to store back index
 * @param felse (statement) code in 'else' brace of front node
 * @param belse (statement) code in 'else' brace of back node
 */
#define move_down_node(c, n, fi, bi, felse, belse)	\
	do {						\
		(fi) = (c) >> 4; (bi) = (c) & 0x0f;	\
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
 * If node exists, it returns node. If not, it creates new one and returns it.
 * @return back node of last character of symbol.
 *	If fails to get, NULL is returned.
 */
static struct node *
get_node(struct ytrie *t, const u8 *key, u32 sz, int bcreate) {
	register struct node *n = &t->rt;
	register const u8 *p = key;
	register const u8 *pend = p+sz;
	register int fi, bi; /* front index */

	if (bcreate) {
		while (p < pend) {
			move_down_node(*p, n, fi, bi,
				       n = n->n[fi] = alloc_node(); \
				       if (unlikely(!n)) { return NULL; },
				       n = n->n[bi] = alloc_node(); \
				       if (unlikely(!n)) { return NULL; });
			p++;
		}
	} else {
		while (p < pend) {
			move_down_node(*p, n, fi, bi,
				       return NULL, return NULL);
			p++;
		}
	}
	return n;
}

static bool
equal_internal(
	const struct node *n0,
	const struct node *n1,
	int(*cmp)(const void *, const void *)
) {
	register int i;
	if (n0->v && n1->v) {
		if (0 == cmp(n0->v, n1->v))
			return FALSE; /* NOT same */
	} else if (n0->v || n1->v)
		return FALSE; /* NOT same */

	/* compare child nodes! */
	for (i = 0; i < 16; i++) {
		if (n0->n[i] && n1->n[i]) {
			if (0 == equal_internal(n0->n[i], n1->n[i], cmp))
				return FALSE; /* NOT same */
		} else if (n0->n[i] || n1->n[i])
			return FALSE; /* NOT same */
	}
	return TRUE; /* n0 and n1 is same */
}

static struct node *
node_clone(
	const struct node *n,
	void *tag,
	void *(*clonev)(void *, const void *)
) {
	register int i;
	struct node *r = alloc_node();
	if (unlikely(!r))
		return NULL;
	if (n->v)
		r->v = clonev(tag, n->v);
	for (i = 0; i < 16; i++)
		if (n->n[i])
			r->n[i] = node_clone(n->n[i], tag, clonev);
	return r;
}

/*
 *
 */
static int
iterate_internal(
	void *tag,
	struct node *n,
	int(cb)(void *, const u8 *, u32, void *),
	/* bsz: excluding space for trailing 0 */
	u8 *buf,
	u32 bsz,
	u32 bitoffset
) {
	register u32 i;

	if (n->v) {
		/* value should exists at byte-based-node */
		yassert(0 == bitoffset % 8);
		/* if (buf) { buf[bitoffset>>3] = 0 ; } */
		/* keep going? */
		if (!(*cb)(tag, buf, bitoffset / 8, n->v))
			return 0;
	}

	for (i = 0; i < 16; i++) {
		if (n->n[i]) {
			int   r;
			if (buf) {
				u8 *p =  buf + (bitoffset >> 3);
				/* check that buffer is remained enough */
				if (bitoffset >> 3 == bsz)
					/* Not enough buffer.
					 * TODO
					 * Useful error value should be used.
					 */
					return -EINVAL;

				if (!(bitoffset % 8))
					/*
					 * multiple of 8
					 * - getting index for front node
					 */
					*p = (i << 4);
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
			r = iterate_internal(
				tag,
				n->n[i],
				cb,
				buf,
				bsz,
				bitoffset + 4);
			if (r <= 0)
				return r;
		}
	}
	return 1;
}

void **
ytrie_getref(struct ytrie *t, const u8 *key, u32 sz) {
	struct node *n;
	yassert(key);
	if (!sz)
		return NULL; /* 0 length string */
	n = get_node(t, key, sz, FALSE);
	return n ? &n->v : NULL;
}

void *
ytrie_get(struct ytrie *t, const u8 *key, u32 sz) {
	void ** pv = ytrie_getref(t, key, sz);
	return pv ? *pv : NULL;
}

int
ytrie_iterate(
	struct ytrie *t,
	void *tag,
	const u8 *key,
	u32 keysz,
	/* return 1 for keep going, 0 for stop and don't do anymore */
	int(cb)(void *, const u8 *, u32, void *)
) {
	char buf[YTRIE_MAX_KEY_LEN + 1];
	struct node *n = get_node(t, key, keysz, FALSE);

	yassert(t && key);
	if (n)
		return iterate_internal(
			tag,
			n,
			cb,
			(u8 *)buf,
			YTRIE_MAX_KEY_LEN,
			0);
	else
		return -EINVAL;
}

int
ytrie_insert(struct ytrie *t, const u8 *key, u32 sz, void *v) {
	struct node *n;

	yassert(t && key);
	if (unlikely(!sz))
		return -1; /* 0 length symbol */
	if (unlikely(!v || (sz >= YTRIE_MAX_KEY_LEN)))
		return -1; /* error case */

	if (unlikely(!(n = get_node(t, key, sz, TRUE))))
		return -ENOMEM;
	if (n->v) {
		if (t->vfree)
			(*t->vfree)(n->v);
		n->v = v;
		return 1; /* overwritten */
	} else {
		n->v = v;
		return 0; /* newly created */
	}
}

struct ytrie *
ytrie_create(void (*vfree)(void *)) {
	struct ytrie *t = ycalloc(1, sizeof(*t));
	if (!t)
		return NULL;
	t->vfree = vfree;
	return t;
}

void
ytrie_reset(struct ytrie *t) {
	register int i;
	for (i = 0; i < 16; i++) {
		if (t->rt.n[i]) {
			free_node(t->rt.n[i], t->vfree, FALSE);
			t->rt.n[i] = NULL;
		}
	}
}

void
ytrie_destroy(struct ytrie *t) {
	ytrie_reset(t);
	yfree(t);
}


int
ytrie_remove(struct ytrie *t, const u8 *key, u32 sz) {
	yassert(t && key);
	switch(remove_key(&t->rt, key, sz, t->vfree)) {
	case 1:
	case 0: return 1;
	default: return 0;
	}
}

void (*
ytrie_vfree(const struct ytrie *t))(void *) {
	return t->vfree;
}

bool
ytrie_equal(
	const struct ytrie *t0,
	const struct ytrie *t1,
	int (*cmp)(const void *, const void *)
) {
	return equal_internal(&t0->rt, &t1->rt, cmp);
}

int
ytrie_copy(
	struct ytrie *dst,
	const struct ytrie *src,
	void *tag,
	void *(*clonev)(void *,const void *)
) {
	register int i;
	ytrie_reset(dst);
	dst->vfree = src->vfree;
	for (i=0; i<16; i++) {
		if (src->rt.n[i])
			dst->rt.n[i] = node_clone(src->rt.n[i], tag, clonev);
	}
	/* return value of this function is reserved for future use */
	return 0;
}

struct ytrie *
ytrie_clone(
	const struct ytrie *t,
	void *tag,
	void *(*clonev)(void *, const void *)
) {
	struct ytrie *r = ytrie_create(t->vfree);
	ytrie_copy(r, t, tag, clonev);
	return r;
}

int
ytrie_auto_complete(
	struct ytrie *t,
	const u8 *keyprefix,
	u32 keyprefixsz,
	u8 *buf,
	u32 bufsz
) {
#define YTRIEBranch 0
#define YTRIELeaf 1
#define YTRIEFail 2

	int ret = -EINVAL;
	register struct node *n;
	register u32 i;
	u32 j, cnt, bi;
	u8 c;

	yassert(t && keyprefix && buf);

	/* move to prefix */
	if (unlikely(!(n = get_node(t, keyprefix, keyprefixsz, FALSE))))
		goto fail;

	/* find more possbile prefix */
	bi = 0;
	while (n) {
		cnt = j = c = 0;
		for (i = 0; i < 16; i++)
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
		c = j << 4;
		n = n->n[j];

		/* Sanity check! */
		/* we don't expect that this can have value */
		yassert(!n->v);

		cnt = j = 0;
		for (i = 0; i < 16; i++) {
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
			return -EINVAL;
		else {
			buf[bi] = c;
			bi++;
		}
	}

	buf[bi] = 0; /* add trailing 0 */
	return ret;

 fail:
	return YTRIEFail;

#undef YTRIEBranch
#undef YTRIELeaf
#undef YTRIEFail
}
