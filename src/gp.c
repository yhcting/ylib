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
#include <errno.h>

#include "lib.h"
#include "yo.h"
#include "def.h"
#include "common.h"
#include "ylistl.h"
#include "ypool.h"


static const int DEFAULT_O_POOL_SIZE = 100;

struct ygp {
        void *o;
        void (*ofree)(void *);
	pthread_spinlock_t lock;
        int refcnt;  /* reference count */
	struct ylistl_link lk;
};


static struct ypool *_pool;


/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
static inline void
init_lock(struct ygp *gp) {
	fatali0(pthread_spin_init(&gp->lock, PTHREAD_PROCESS_PRIVATE));
}

static inline void
lock(struct ygp *gp) {
	fatali0(pthread_spin_lock(&gp->lock));
}

static inline void
unlock(struct ygp *gp) {
	fatali0(pthread_spin_unlock(&gp->lock));
}

static inline void
destroy_lock(struct ygp *gp) {
	fatali0(pthread_spin_destroy(&gp->lock));
}

static int
inc_refcnt(struct ygp *gp) {
	int r;
	lock(gp);
	r = ++gp->refcnt;
	unlock(gp);
	return r;
}

static int
dec_refcnt(struct ygp *gp) {
	int r;
	lock(gp);
	r = --gp->refcnt;
	unlock(gp);
	return r;
}

/******************************************************************************
 * ISSUE:
 * Should spinlock be destroied and re-initialized? or, those are kept?
 * Resource(space) vs. Performance trade-off.
 * How big spinlock struct is? vs. How fast spinlock can be initialized.
 *
 * At this moment, gp is put in pool with alive-spinlock!
 *****************************************************************************/
static inline void
gpclear(struct ygp *gp) {
	if (likely(gp->ofree))
		(*gp->ofree)(gp->o);
	gp->ofree = NULL;
	gp->refcnt = 0;
}

static inline void
gpdestroy(struct ygp *gp) {
        gpclear(gp);
        destroy_lock(gp);
        yfree(gp);
}

static struct ygp *
pool_get(void) {
	struct ylistl_link *lk;
        lk = ypool_get(_pool);
        if (unlikely(!lk))
                return NULL;
        return containerof(lk, struct ygp, lk);
}

static bool
pool_put(struct ygp *gp) {
        return ypool_put(_pool, &gp->lk);
}

static void pool_clear(void) __unused;
static void
pool_clear(void) {
        struct ygp *gp;
        while ((gp = pool_get()))
                gpdestroy(gp);
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
struct ygp *
ygpcreate(void *o, void (*ofree)(void *)) {
	struct ygp *gp = pool_get();
	if (unlikely(!gp)) {
		gp = (struct ygp *)ycalloc(1, sizeof(*gp));
                if (unlikely(!gp))
                        return NULL;
                init_lock(gp);
        }
	yassert(!gp->refcnt);
	gp->o = o;
	gp->ofree = ofree;
	ylistl_init_link(&gp->lk);
	return gp;
}

void
ygpdestroy(struct ygp *gp) {
        gpclear(gp);
	if (unlikely(!pool_put(gp)))
                gpdestroy(gp);
}

void
ygpput(struct ygp *gp) {
	int refcnt = dec_refcnt(gp);
	yassert(0 <= refcnt);
	if (unlikely(refcnt <= 0))
		ygpdestroy(gp);
}

void
ygpget(struct ygp *gp) {
	int refcnt __unused;
	refcnt = inc_refcnt(gp);
	yassert(0 < refcnt);
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
gp_clear(void) {
	pool_clear();
}
#endif /* CONFIG_DEBUG */

static int
minit(const struct ylib_config *cfg) {
	int capacity = cfg && (cfg->ygp_pool_capacity > 0)?
		cfg->ygp_pool_capacity:
		DEFAULT_O_POOL_SIZE;
        _pool = ypool_create(capacity);
        return _pool? 0: -ENOMEM;
}

static void
mexit(void) {
        pool_clear();
        ypool_destroy(_pool);
}

LIB_MODULE(gp, minit, mexit);
