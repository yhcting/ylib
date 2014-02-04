/******************************************************************************
 *    Copyright (C) 2011, 2012, 2013, 2014
 *    Younghyung Cho. <yhcting77@gmail.com>
 *
 *    This file is part of ylib
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License
 *    (<http://www.gnu.org/licenses/lgpl.html>) for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.	If not, see <http://www.gnu.org/licenses/>.
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
		/* message code */
		long   code;
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
	       long    code,
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
		   && m->free))
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
 * return 0 for success, otherwise error number
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
