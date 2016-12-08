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
#include <pthread.h>

#include "common.h"
#include "ypool.h"


struct ypool {
	pthread_spinlock_t lock;
	int sz;
	int capacity;
	struct ylistl_link hd;
};

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static void
init_lock(struct ypool *p) {
	fatali0(pthread_spin_init(&p->lock, PTHREAD_PROCESS_PRIVATE));
}

static inline void
lock(struct ypool *p) {
	fatali0(pthread_spin_lock(&p->lock));
}

static inline void
unlock(struct ypool *p) {
	fatali0(pthread_spin_unlock(&p->lock));
}

static void
destroy_lock(struct ypool *p) {
	fatali0(pthread_spin_destroy(&p->lock));
}


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
struct ypool *
ypool_create(int capacity) {
        int r __unused;
        struct ypool *p;
        p = ycalloc(1, sizeof(*p));
        if (unlikely(!p))
                return NULL;
	init_lock(p);
	ylistl_init_link(&p->hd);
	p->capacity = capacity;
	return p;
}

void
ypool_destroy(struct ypool *p) {
	destroy_lock(p);
        yfree(p);
}

struct ylistl_link *
ypool_get(struct ypool *p) {
	struct ylistl_link *lk = NULL;
	lock(p);
	if (unlikely(ylistl_is_empty(&p->hd)))
		goto done_unlock;
	lk = ylistl_remove_first(&p->hd);
	yassert(p->sz > 0);
	p->sz--;
 done_unlock:
	unlock(p);
        return lk;
}

bool
ypool_put(struct ypool *p, struct ylistl_link *lk) {
	bool r = FALSE;
	lock(p);
	if (likely(p->sz < p->capacity)) {
		ylistl_add_last(&p->hd, lk);
		p->sz++;
		r = TRUE;
	}
	unlock(p);
	return r;
}
