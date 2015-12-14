/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015
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
 * @file ymsgq.h
 * @brief Header to use simple message queue module.
 */

#ifndef __YMSGq_h__
#define __YMSGq_h__

#include <stdint.h>
#include <string.h>

#include "ydef.h"
#include "ylistl.h"

/** Message queue object supporting Multi-Thread */
struct ymq;

/* Message type */
enum {
	/* TYP_INVALID SHOULD be 0. 0 is natual initial value of empty data
	 *   structure
	 */
	YMSG_TYP_INVALID = 0, /**< Invalid message type */
	YMSG_TYP_DATA, /**< Message contains 'data' */
	YMSG_TYP_EXEC, /**< Message to execute 'action' */
};

/* Priority */
enum {
	YMSG_PRI_VERY_HIGH = 0, /**< Highest priority */
	YMSG_PRI_HIGH,
	YMSG_PRI_NORMAL,
	YMSG_PRI_LOW,
	YMSG_PRI_VERY_LOW, /**< Lowest priority */
	YMSG_PRI_NR, /**< number of priority */
};

/**
 * Message structure.
 * Message type can be one of followings.
 * data({@link YMSG_TYP_DATA}) or exec({@link YMSG_TYP_EXEC})
 */
struct ymsg {
	uint64_t when; /**< Nsec since boot */
	uint8_t type; /**< Message type [0-255] */
	uint8_t pri; /**< Priority [0-255] */
	uint16_t opt; /**< Option - reserved for future use */
	void *data; /**< Data or argument of action */
	void (*dfree)(void *); /**< Function to free \a data */
	union {
		/** Message code - int type to use as switch argument. */
		int code;
		/** Callback function to execute with argument \a data */
		void (*run)(void *, void (*)(void *));
	};
};


/******************************************************************************
 *
 * Y Message
 *
 *****************************************************************************/
/**
 * Initialise message object.
 *
 * @param m Message object
 * @param type Type
 * @param pri Priority
 * @param opt Options
 * @param data Data
 */
static YYINLINE void
ymsg_set_common(struct ymsg *m,
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

/**
 * Initialise data-message object.
 *
 * @param m Message object
 * @param pri Priority
 * @param code Message code
 * @param data Data
 */
static YYINLINE void
ymsg_set_data(struct ymsg *m,
	      uint8_t pri,
	      int code,
	      void *data) {
	ymsg_set_common(m, YMSG_TYP_DATA, pri, 0, data);
	m->code = code;
}

/**
 * Initialise exec-message object.
 *
 * @param m Message object
 * @param pri Priority
 * @param data Argument data
 * @param run Function to run
 */
static YYINLINE void
ymsg_set_exec(struct ymsg *m,
	      uint8_t pri,
	      void *data,
	      void (*run)(void *, void (*)(void *))) {
	ymsg_set_common(m, YMSG_TYP_EXEC, pri, 0, data);
	m->run = run;
}

/**
 * Create message object.
 * DO NOT free returned ymsg object directly!
 * Returned ymsg object SHOULD BE destroied by using {@link ymsg_destroy}
 *
 * @return NULL if fails(ex. ENOMEM)
 */
struct ymsg *
ymsg_create(void);

/**
 * Destroy message object created by {@link ymsg_create}.
 */
void
ymsg_destroy(struct ymsg *);

/******************************************************************************
 *
 * Y Message Queue
 *
 *****************************************************************************/
/**
 * Create message-queue object.
 *
 * @return NULL if fails.
 */
YYEXPORT struct ymq *
ymq_create(void);

/**
 * Destroy message-queue object.
 */
YYEXPORT void
ymq_destroy(struct ymq *);

/**
 * Enqueue message to message queue
 *
 * @return 0 if success. Otherwise -errno
 *         (EPERM means 'Q is already full')
 */
YYEXPORT int
ymq_en(struct ymq *, struct ymsg *);

/**
 * Dequeue message from message queue.
 * This is blocking function.
 * So, if message queue is empty, wait until message is available.
 *
 * @return NULL if fails.
 */
YYEXPORT struct ymsg *
ymq_de(struct ymq *);

/**
 * Get size(number of messsage) of message queue.
 *
 * @return Size
 */
YYEXPORT uint32_t
ymq_sz(const struct ymq *);

#endif /* __YMSGq_h__ */
