/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014
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

#ifndef __YMSGq_h__
#define __YMSGq_h__

#include <stdint.h>
#include <string.h>

#include "ycommon.h"
#include "ylistl.h"

struct ymq; /* Message Queue (Supports Multi-Thread) */

/* Message type */
enum {
	/* TYP_INVALID SHOULD be 0. 0 is natual initial value of empty data
	 *   structure
	 */
	YMSG_TYP_INVALID = 0,
	YMSG_TYP_DATA,
	YMSG_TYP_EXEC,
};

/* Priority */
enum {
	YMSG_PRI_VERY_HIGH = 0, /* Highest priority */
	YMSG_PRI_HIGH,
	YMSG_PRI_NORMAL,
	YMSG_PRI_LOW,
	YMSG_PRI_VERY_LOW, /* Lowest priority */
	YMSG_PRI_NR, /* number of priority */
};

/*
 * Message type can be one of followings.
 * - data :
 * - command : ex. function pointer.
 */
struct ymsg {
	/* For internal use. DO NOT access 'lk' at out side library! */
	struct ylistl_link lk;
	uint64_t when; /* nsec since boot */
	uint8_t  type; /* message type [0-255] */
	uint8_t  pri;  /* priority [0-255] */
	uint16_t opt;  /* option - reserved. */
	void    *data; /* data or argument of action */
	void   (*free)(void *); /* function to free passing data/arg */
	union {
		/* message code - int type to use as switch argument. */
		int    code;
		/* execute with given argument */
		void (*run)(void *, void (*)(void *));
	} u;
};


/* ============================================================================
 *
 * Y Message
 *
 * ==========================================================================*/
static inline void
ymsg_init_common(struct ymsg *m,
		 uint8_t type,
		 uint8_t pri,
		 uint16_t opt,
		 void *data) {
	m->when = 0; /* set to invalid value */
	m->type = type;
	m->pri = pri;
	m->opt = opt; /* reserved */
	m->data = data;
}

static inline void
ymsg_init_data(struct ymsg *m,
	       uint8_t pri,
	       int     code,
	       void   *data) {
	ymsg_init_common(m, YMSG_TYP_DATA, pri, 0, data);
	m->u.code = code;
}

static inline void
ymsg_init_exec(struct ymsg *m,
	       uint8_t pri,
	       void   *data,
	       void  (*run)(void *, void (*)(void *))) {
	ymsg_init_common(m, YMSG_TYP_EXEC, pri, 0, data);
	m->u.run = run;
}

static inline struct ymsg *
ymsg_create(void) {
	return (struct ymsg *)ycalloc(1, sizeof(struct ymsg));
}

static inline void
ymsg_destroy(struct ymsg *m) {
	if (likely(YMSG_TYP_INVALID != m->type
		   && m->free
		   && m->data))
		(*m->free)(m->data);
	yfree(m);
}

/* ============================================================================
 *
 * Y Message Queue
 *
 * ==========================================================================*/

EXPORT struct ymq *
ymq_create(void);

EXPORT void
ymq_destroy(struct ymq *);

/**
 * return 0 for success, otherwise -(error number)
 *     EPERM : Q is already full.
 */
EXPORT int
ymq_en(struct ymq *, struct ymsg *);

/**
 * Blocking function.
 * NULL for error.
 */
EXPORT struct ymsg *
ymq_de(struct ymq *);

EXPORT unsigned int
ymq_sz(const struct ymq *);

#endif /* __YMSGq_h__ */
