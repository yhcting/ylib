/******************************************************************************
 * Copyright (C) 2021
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

/**
 * @file yhashl.h
 * @brief Header file to use simple hash module
 *
 * This is NOT MT(Multithread)-safe.
 */

#ifndef __YHASHl_h__
#define __YHASHl_h__

#include <stddef.h>
#include "ylistl.h"

/** Hash function type */
typedef uint32_t (*yhashl_hfunc_t)(const void *);
/**
 * Function to compare two keys.
 * Return 0 if two keys are equal. Otherwise if not equal
 */
typedef int (*yhashl_keyeq_t)(const void *, const void *);

/* Predefined default functions */

/** Default hash function for pointer and number. */
#define YHASHL_HFUNC_PTR ((yhashl_hfunc_t)(void *)1)
/** Default hash function for string. */
#define YHASHL_HFUNC_STR ((yhashl_hfunc_t)(void *)2)
/** Default key equal-function for pointer and number */
#define YHASHL_KEYEQ_PTR ((yhashl_keyeq_t)(void *)1)
/** Default key equal-function for string */
#define YHASHL_KEYEQ_STR ((yhashl_keyeq_t)(void *)2)

/**
 * hash node structure.
 * All member are @c protected (excluded at documents)
 */
struct yhashl_node {
	/* @cond */
	struct ylistl_link lk;
	const void *key;
	uint32_t hv32; /* 32bit hash value */
	/* @endcond */
};

/**
 * Hash structure.
 * All member are @c protected (excluded at documents)
 */
struct yhashl {
	/* @cond */
	struct ylistl_link *map;
	uint8_t mapbits;
	uint32_t sz;
	yhashl_hfunc_t h;
	yhashl_keyeq_t keq;
	/* @endcond */
};


/**
 * Size of hash map. It's different from hash size.
 * You may rarely need to know this.
 */
static YYINLINE u32
yhashl_hmapsz(const struct yhashl *h) {
	return 1 << h->mapbits;
}

static YYINLINE void
yhashl_init_node(struct yhashl_node *n) {
	ylistl_init_link(&n->lk);
	n->key = NULL;
	n->hv32 = 0;
}

/**
 * Get key value of this node.
 */
static YYINLINE const void *
yhashl_node_key(struct yhashl_node *n) {
	return n->key;
}

YYEXPORT struct yhashl *
yhashl_create(yhashl_hfunc_t hfunc, yhashl_keyeq_t keyeq);

/**
 * @param initbits <= 32. 2 ^ initbits is used as initial map size.
 */
YYEXPORT struct yhashl *
yhashl_create2(
	yhashl_hfunc_t hfunc,
	yhashl_keyeq_t keqfunc,
	uint32_t initbits);

EXPORT void
yhashl_destroy(struct yhashl *);

/** Number of items(nodes) in hash */
static YYINLINE uint32_t
yhashl_sz(const struct yhashl *h) {
	return h->sz;
}

/**
 * Change hash map size.
 *
 * @return 0 for success othersize -errno
 */
YYEXPORT int
yhashl_hremap(struct yhashl *, u32 mapbits);

/**
 * Key is ALWAYS stored by SHALLOW-COPY.
 * If node having same key is found, then key is NOT replaced.
 * If key should be freed and non-NULL is returned, both key and value should
 * be freed.
 *
 * @return If node of same key already exists, it will be returned.
 *	Otherwise NULL.
 */
YYEXPORT struct yhashl_node *
yhashl_set(struct yhashl *h, const void *key, struct yhashl_node *);

/**
 * @p n should be node in hash @p h. If it's not, behavior is not defined.
 */
YYEXPORT void
yhashl_remove_node(struct yhashl *h, struct yhashl_node *n);

/**
 * Remove node from hash and return it.
 *
 * @return removed node
 */
YYEXPORT struct yhashl_node *
yhashl_remove(struct yhashl *h, const void *key);

YYEXPORT struct yhashl_node *
yhashl_get(const struct yhashl *h, const void *key);

/**
 * Visit all hash nodes
 *
 * @param h (struct yhashl *)
 * @param cur (struct yhashl_node *) Cursor
 */
#define yhashl_foreach(h, cur)						\
	for (unsigned int i___ = 0; i___ < yhashl_hmapsz(h); i___++)	\
		ylistl_foreach_item(cur, &h->map[i___], struct yhashl_node, lk)

/**
 * Visit all hash nodes with removal-safe.
 *
 * @param h (struct yhashl *)
 * @param cur (struct yhashl_node *) Cursor
 * @param tmp (struct yhashl_node *) Temporary storage.
 */
#define yhashl_foreach_safe(h, cur, tmplk)				\
	for (unsigned int i___ = 0; i___ < yhashl_hmapsz(h); i___++)	\
		ylistl_foreach_item_safe(cur, tmp, &h->map[i___],	\
			struct yhashl_node, lk)


#endif /* __YHASHl_h__ */
