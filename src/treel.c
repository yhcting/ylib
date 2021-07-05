/******************************************************************************
 * Copyright (C) 2015
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

#include "common.h"
#include "ytreel.h"
#include "ylist.h"

struct ytreeli {
	int (*next)(struct ytreeli *);
	void (*clean)(struct ytreeli *);
	const struct ytreel_link *lc, *ln; /* Link Current, Link Next */
	int nerr; /* 0 if ln is successfuly set. Otherwise -errno */
	union {
		struct ylist *l;
		const struct ytreel_link *top;
	};
};

/******************************************************************************
 *
 * Init
 *
 *****************************************************************************/
static int
preot_init(struct ytreeli *itr, const struct ytreel_link *toplk) {
	if (unlikely(!(itr->l = ylist_create(0, NULL))))
		return -ENOMEM;
	itr->ln = toplk;
	return 0;
}

static int
levelot_init(struct ytreeli *itr, const struct ytreel_link *toplk) {
	if (unlikely(!(itr->l = ylist_create(0, NULL))))
		return -ENOMEM;
	itr->ln = toplk;
	return 0;
}

static int
l2rpostot_init(struct ytreeli *itr, const struct ytreel_link *toplk) {
	const struct ytreel_link *lk = toplk;
	itr->top = toplk;
	while (ytreel_has_child(lk))
		lk = ytreel_first_child(lk);
	itr->ln = lk;
	return 0;
}

static int
r2lpostot_init(struct ytreeli *itr, const struct ytreel_link *toplk) {
	const struct ytreel_link *lk = toplk;
	itr->top = toplk;
	while (ytreel_has_child(lk))
		lk = ytreel_last_child(lk);
	itr->ln = lk;
	return 0;
}

/******************************************************************************
 *
 * Clean
 *
 *****************************************************************************/
static void
preot_clean(struct ytreeli *itr) {
	ylist_destroy(itr->l);
}

static void
levelot_clean(struct ytreeli *itr) {
	ylist_destroy(itr->l);
}

static void
postot_clean(struct ytreeli *itr) {
	/* nothing to do */
}

/******************************************************************************
 *
 * Tree traversal
 *
 *****************************************************************************/
static int
preot_next(struct ytreeli *itr, bool l2r) {
	struct ylistl_link *pos;
	if (unlikely(itr->nerr))
		/* itr->ln is invalid value */
		return itr->nerr;
	yassert(itr->ln);
	itr->lc = itr->ln;

	/* Prefetch next link */
#define push_siblings()							\
	int r = ylist_push(itr->l,					\
			   containerof(pos, struct ytreel_link, sibling)); \
	if (unlikely(r)) {						\
		itr->nerr = r;						\
		goto done;						\
	}

	if (likely(l2r)) {
		ylistl_foreach_backward(pos, &itr->ln->child) {
			push_siblings();
		}
	} else {
		ylistl_foreach(pos, &itr->ln->child) {
			push_siblings();
		}
	}
#undef push_siblings

	if (unlikely(ylist_is_empty(itr->l)))
		itr->nerr = -ENOENT;
	else {
		struct ytreel_link *lk =
			(struct ytreel_link *)ylist_pop(itr->l);
		itr->nerr = unlikely(!lk) ? -EINVAL : 0;
		itr->ln = lk;
	}

 done:
	if (unlikely(itr->nerr))
		itr->ln = NULL;
	return 0;
}

static int
l2rpreot_next(struct ytreeli *itr) {
	return preot_next(itr, TRUE);
}

static int
r2lpreot_next(struct ytreeli *itr) {
	return preot_next(itr, FALSE);
}

static int
levelot_next(struct ytreeli *itr) {
	struct ylistl_link *pos;
	if (unlikely(itr->nerr))
		/* itr->ln is invalid value */
		return itr->nerr;
	yassert(itr->ln);
	itr->lc = itr->ln;

	ylistl_foreach(pos, &itr->ln->child) {
		int r = ylist_enq(itr->l,
				  containerof(pos,
					      struct ytreel_link,
					      sibling));
		if (unlikely(r)) {
			itr->nerr = r;
			goto done;
		}
	}

	if (unlikely(ylist_is_empty(itr->l)))
		itr->nerr = -ENOENT;
	else {
		struct ytreel_link *lk =
			(struct ytreel_link *)ylist_deq(itr->l);
		itr->nerr = unlikely(!lk) ? -EINVAL : 0;
		itr->ln = lk;
	}

 done:
	if (unlikely(itr->nerr))
		itr->ln = NULL;
	return 0;
}

static int
postot_next(struct ytreeli *itr, bool l2r) {
	const struct ytreel_link *lk;
	bool (*has_next)(const struct ytreel_link *);
	struct ytreel_link *(*next)(const struct ytreel_link *);
	struct ytreel_link *(*child)(const struct ytreel_link *);

	if (unlikely(itr->nerr))
		/* itr->ln is invalid value */
		return itr->nerr;
	yassert(itr->ln);
	itr->lc = itr->ln;

	if (likely(l2r)) {
		has_next = &ytreel_has_next;
		next = &ytreel_next;
		child = &ytreel_first_child;
	} else {
		has_next = &ytreel_has_prev;
		next = &ytreel_prev;
		child = &ytreel_last_child;
	}

	/* Prefetch next link */
	lk = itr->lc;
	if (lk != itr->top) {
		if (likely((*has_next)(lk))) {
			lk = (*next)(lk);
			while (ytreel_has_child(lk))
				lk = (*child)(lk);
		} else {
			yassert(ytreel_has_parent(lk));
			lk = lk->parent;
		}
	} else
		itr->nerr = -ENOENT;
	itr->ln = (unlikely(itr->nerr)) ? NULL : lk;
	return 0;
}

static int
l2rpostot_next(struct ytreeli *itr) {
	return postot_next(itr, TRUE);
}

static int
r2lpostot_next(struct ytreeli *itr) {
	return postot_next(itr, FALSE);
}


/******************************************************************************
 *
 * Interfaces
 *
 *****************************************************************************/
struct otinterface {
	int (*init)(struct ytreeli *, const struct ytreel_link *);
	void (*clean)(struct ytreeli *);
	int (*next)(struct ytreeli *);
} otis[YTREELI_OT_TYPE_COUNT] = {
	/* L2R PRE OT */
	{&preot_init, &preot_clean, &l2rpreot_next},
	/* R2L PRE OT */
	{&preot_init, &preot_clean, &r2lpreot_next},
	/* LEVEL OT */
	{&levelot_init, &levelot_clean, &levelot_next},
	/* L2R POST OT */
	{&l2rpostot_init, &postot_clean, &l2rpostot_next},
	/* R2L POST OT */
	{&r2lpostot_init, &postot_clean, &r2lpostot_next},
};

struct ytreeli *
ytreeli_create(const struct ytreel_link *toplk, enum ytreeli_ot_type type) {
	struct ytreeli *itr;
	struct otinterface *oti;
	itr = (struct ytreeli *)ymalloc(sizeof(*itr));
	if (unlikely(!itr))
		return NULL;
	oti = &otis[type];

	itr->next = oti->next;
	itr->clean = oti->clean;
	itr->lc = NULL;
	itr->nerr = 0;
	if (unlikely((*oti->init)(itr, toplk))) {
		yfree(itr);
		return NULL;
	}
	return itr;
}

void
ytreeli_destroy(struct ytreeli *itr) {
	(*itr->clean)(itr);
	yfree(itr);
}

int
ytreeli_next(struct ytreeli *itr) {
	return (*itr->next)(itr);
}

struct ytreel_link *
ytreeli_get(struct ytreeli *itr) {
	return (struct ytreel_link *)itr->lc;
}

bool
ytreeli_has_next(struct ytreeli *itr) {
	return !itr->nerr;
}
