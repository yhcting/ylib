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

/**
 * @file msg.h
 * @brief Header to use ymsg in inside library
 */

#ifndef __MSg_h__
#define __MSg_h__

#include "ymsg.h"
#include "def.h"

struct ymsghandler;

/*
 * char MAGIC[] = { 'w', 'x', 'y', 'z' };
 * '*((u32 *)MAIGC)' is easier but to avoid 'strict-aliasing warning'
 * below way is used.
 */
static const u32 MSG_MAGIC = 'y' << 24 | 'm' << 16 | 's' << 8 | 'g';



/* internal ymsg structure */
struct ymsg_ {
#ifdef CONFIG_DEBUG
	u32 magic0;
#endif /* CONFIG_DEBUG */
	struct ymsg m;
#ifdef CONFIG_DEBUG
	u32 magic1;
	u64 when; /**< Nsec since boot */
#endif /* CONFIG_DEBUG */
	/** For internal use. DO NOT access 'lk' at out side library! */
	struct ymsghandler *handler;
	struct ylistl_link lk;
};


/******************************************************************************
 *
 * DEBUG
 *
 *****************************************************************************/
#ifdef CONFIG_DEBUG

static INLINE void
_msg_magic_verify(struct ymsg_ *m) {
	yassert(m
		&& MSG_MAGIC == m->magic0
		&& MSG_MAGIC == m->magic1);
}

static INLINE void
_msg_magic_set(struct ymsg_ *m) {
	m->magic0 = m->magic1 = MSG_MAGIC;
}

#else /* CONFIG_DEBUG */

static INLINE void
_msg_magic_verify(struct ymsg_ *m __unused) {}
static INLINE void
_msg_magic_set(struct ymsg_ *m __unused) {}

#endif /* CONFIG_DEBUG */


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static INLINE void
msg_set_handler(struct ymsg_ *m, struct ymsghandler *h) {
	m->handler = h;
}

static INLINE struct ymsg_ *
msg_mutate(struct ymsg *ym) {
	struct ymsg_ *m = ym? containerof(ym, struct ymsg_, m): NULL;
	_msg_magic_verify(m);
	return m;
}


#endif /* __MSg_h__ */
