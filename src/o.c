/******************************************************************************
 * Copyright (C) 2016
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
#include <limits.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "yo.h"
#include "ylib.h"
#include "lib.h"
#include "def.h"
#include "common.h"
#include "ylistl.h"
#include "ypool.h"


static const int DEFAULT_O_POOL_SIZE = 1000;

struct o {
        struct yo o;
        void (*ofree0)(void *);
        void (*ofree1)(void *);
        void (*ofree2)(void *);
        void (*ofree3)(void *);
	struct ylistl_link lk;
};


static struct ypool *_pool;


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static inline void
oclear(struct o *o) {
#define free_el(n)					\
	do {						\
		if (likely(o->ofree##n)) {		\
			(*o->ofree##n)(o->o.o##n);	\
		}					\
		o->ofree##n = NULL;			\
	} while (0)

	free_el(0);
	free_el(1);
	free_el(2);
	free_el(3);
#undef free_el
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static struct o *
pool_get(void) {
	struct ylistl_link *lk;
        lk = ypool_get(_pool);
        if (unlikely(!lk))
                return NULL;
        return containerof(lk, struct o, lk);
}

static bool
pool_put(struct o *o) {
        return ypool_put(_pool, &o->lk);
}

static void pool_clear(void) __unused;
static void
pool_clear(void) {
        struct o *o;
        while ((o = pool_get())) {
                oclear(o);
                yfree(o);
        }
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
struct yo *
yocreate3(void *o0, void (*ofree0)(void *),
	  void *o1, void (*ofree1)(void *),
	  void *o2, void (*ofree2)(void *),
	  void *o3, void (*ofree3)(void *)) {
	struct o *o = pool_get();
	if (unlikely(!o))
		o = (struct o *)ycalloc(1, sizeof(*o));
        else
                memset(o, 0, sizeof(*o));
        if (unlikely(!o))
                return NULL;
	o->o.o0 = o0;
	o->ofree0 = ofree0;
	o->o.o1 = o1;
	o->ofree1 = ofree1;
	o->o.o2 = o2;
	o->ofree2 = ofree2;
	o->o.o3 = o3;
	o->ofree3 = ofree3;
 	ylistl_init_link(&o->lk);
	return &o->o;
}

void
yodestroy(struct yo *yo) {
	struct o *o = (struct o *)yo;
        oclear(o);
	if (unlikely(!pool_put(o)))
                yfree(o);
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
#ifdef CONFIG_DEBUG
/*
 * This function is used for testing and debugging.
 */
void
o_clear(void) {
	pool_clear();
}
#endif /* CONFIG_DEBUG */

static int
minit(const struct ylib_config *cfg) {
	int capacity = cfg && (cfg->yo_pool_capacity > 0)?
		cfg->yo_pool_capacity:
		DEFAULT_O_POOL_SIZE;
        _pool = ypool_create(capacity);
        return _pool? 0: -ENOMEM;
}

static void
mexit(void) {
        pool_clear();
        ypool_destroy(_pool);
}

LIB_MODULE(o, minit, mexit);
