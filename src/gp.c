/******************************************************************************
 * Copyright (C) 2016, 2021
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
#include "def.h"
#include "common.h"
#include "ygp.h"

#ifndef __GNUC__
#error This module uses GNU C Extentions for atomic operations.
#endif

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
int
ygpinit(struct ygp *gp, void *container, void (*container_free)(void *)) {
	if (unlikely(!container || !gp))
		return -EINVAL;
	gp->container = container;
	gp->container_free = container_free;
	gp->refcnt = 0;
	return 0;
}

void
ygpdestroy(struct ygp *gp) {
	if (likely(gp->container_free))
		(*gp->container_free)(gp->container);
}

int
ygpref_cnt(const struct ygp *gp) {
	return gp->refcnt;
}

int
ygpput(struct ygp *gp) {
	int refcnt = __atomic_add_fetch(&gp->refcnt, -1, __ATOMIC_SEQ_CST);
	yassert(0 <= refcnt);
	if (unlikely(refcnt <= 0))
		ygpdestroy(gp);
	return refcnt;
}

int
ygpget(struct ygp *gp) {
	int refcnt = __atomic_add_fetch(&gp->refcnt, 1, __ATOMIC_SEQ_CST);
	yassert(0 < refcnt);
	return refcnt;
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
}
#endif /* CONFIG_DEBUG */

static int
minit(const struct ylib_config *cfg) {
	return 0;
}

static void
mexit(void) {
}

LIB_MODULE(gp, minit, mexit);
