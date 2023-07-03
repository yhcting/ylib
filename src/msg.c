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
#include <string.h>
#include <errno.h>

#include "lib.h"
#include "common.h"
#include "msg.h"
#include "ypool.h"



#define DEFAULT_MSG_POOL_SIZE 100  /* Should this be configurable? */


static struct ypool *pool_;


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static INLINE void
minit_to_zero(struct ymsg_ *m) {
	memset(m, 0, sizeof(*m));
}

static INLINE void
mfree_data(struct ymsg_ *m) {
	if (likely(YMSG_TYP_INVALID != m->m.type
		&& m->m.dfree
		&& m->m.data)
	) { (*m->m.dfree)(m->m.data); }
	/* to avoid freeing multiple times. */
	m->m.dfree = NULL;
	m->m.data = NULL;
}

static void
mdestroy(struct ymsg_ *m) {
	if (unlikely(!m))
		return;
	mfree_data(m);
	yfree(m);
}


/******************************************************************************
 *
 * Message pool
 *
 *****************************************************************************/
static struct ymsg_ *
pool_get(void) {
	struct ymsg_ *m;
	struct ylistl_link *lk;
	lk = ypool_get(pool_);
	if (unlikely(!lk))
		return NULL;
	m = containerof(lk, struct ymsg_, lk);
	minit_to_zero(m);
	return m;
}

static bool
pool_put(struct ymsg_ *m) {
	return ypool_put(pool_, &m->lk);
}

unused static void
pool_clear(void) {
	struct ymsg_ *m;
	while ((m = pool_get()))
		mdestroy(m);
}


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
struct ymsg *
ymsg_create(void) {
	struct ymsg_ *m = pool_get();
	if (unlikely(!m))
		m = (struct ymsg_ *)ycalloc(1, sizeof(*m));
	if (unlikely(!m))
		return NULL;
	ylistl_init_link(&m->lk);
	_msg_magic_set(m);
	dfpr(".");
	return &m->m;
}

void
ymsg_destroy(struct ymsg *ym) {
	struct ymsg_ *m = msg_mutate(ym);
	mfree_data(m);
	if (unlikely(!pool_put(m)))
		mdestroy(m);
}


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
#ifdef CONFIG_TEST
/*
 * This function is used for testing and debugging.
 */
void
msg_clear(void) {
	pool_clear();
}
#endif /* CONFIG_TEST */


static int
minit(const struct ylib_config *cfg) {
	int capacity = cfg && (cfg->ymsg_pool_capacity > 0)
		? cfg->ymsg_pool_capacity
		: DEFAULT_MSG_POOL_SIZE;
	pool_ = ypool_create(capacity);
	return pool_ ? 0 : -ENOMEM;
}

static void
mexit(void) {
	pool_clear();
	ypool_destroy(pool_);
}

LIB_MODULE(msg, minit, mexit);
