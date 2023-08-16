/******************************************************************************
 * Copyright (C) 2023
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
#include <errno.h>

#include "common.h"
#include "yrx.h"
#include "ylistl.h"

struct yrx {
	/** List of subscribers - yrxsb */
	struct ylistl_link hd;
};

struct yrxsb {
	struct ylistl_link lk;
	struct yrx *rx;
	void *ctx;
	void (*freectx)(void *ctx);
	void (*next)(void *ctx, const void *v);
	void (*error)(void *ctx, const void *v);
	void (*complete)(void *ctx);
};

struct yrx *
yrx_create(void) {
	struct yrx *rx = ymalloc(sizeof(*rx));
	if (unlikely(!rx))
		return NULL;
	ylistl_init_link(&rx->hd);
	return rx;
}


static void
rx_destroy(struct yrx *rx) {
	yfree(rx);
}


struct yrxsb *
yrx_subscribe(
	struct yrx *rx,
	void *ctx,
	void (*freectx)(void *),
	void (*next)(void *ctx, const void *v),
	void (*error)(void *ctx, const void *v),
	void (*complete)(void *ctx)
) {
	struct yrxsb *sb;
	if (unlikely(!rx))
		return NULL;
	sb = ymalloc(sizeof(*sb));
	if (unlikely(!sb))
		return NULL;
	sb->rx = rx;
	sb->ctx = ctx;
	sb->freectx = freectx;
	sb->next = next;
	sb->error = error;
	sb->complete = complete;
	ylistl_add_last(&rx->hd, &sb->lk);
	return sb;
}


static void
rxsb_destroy(struct yrxsb *sb) {
	if (sb->freectx)
		(*sb->freectx)(sb->ctx);
	yfree(sb);
}


int
yrx_unsubscribe(struct yrxsb *sb) {
	if (unlikely(!sb))
		return -EINVAL;
	ylistl_remove(&sb->lk);
	rxsb_destroy(sb);
	return 0;
}


int
yrx_next(struct yrx *rx, const void *v) {
	struct yrxsb *cur;
	if (unlikely(!rx))
		return -EINVAL;
	ylistl_foreach_item(cur, &rx->hd, struct yrxsb, lk) {
		if (likely(cur->next))
			(*cur->next)(cur->ctx, v);
	}
	return 0;
}


int
yrx_error(struct yrx *rx, const void *v) {
	struct yrxsb *cur;
	if (unlikely(!rx))
		return -EINVAL;
	ylistl_foreach_item(cur, &rx->hd, struct yrxsb, lk) {
		if (likely(cur->error))
			(*cur->error)(cur->ctx, v);
	}
	return 0;
}


int
yrx_complete(struct yrx *rx) {
	struct yrxsb *cur, *tmp;
	if (unlikely(!rx))
		return -EINVAL;
	ylistl_foreach_item_safe(cur, tmp, &rx->hd, struct yrxsb, lk) {
		if (likely(cur->complete))
			(*cur->complete)(cur->ctx);
		rxsb_destroy(cur);
	}
	rx_destroy(rx);
	return 0;
}

