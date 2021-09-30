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

#include "ymsg.h"

/** Message queue object supporting Multi-Thread */
struct ymsgq;

/**
 * Create message-queue object.
 *
 * @param capacity Max number of msgs that Q can have.
 *	'<= 0' means 'unlimited capacity'
 * @return NULL if fails.
 */
YYEXPORT struct ymsgq *
ymsgq_create(int capacity);

/**
 * Destroy message-queue object.
 */
YYEXPORT void
ymsgq_destroy(struct ymsgq *);

/**
 * Get event fd file descriptor for this message Q.
 */
YYEXPORT int
ymsgq_evfd(const struct ymsgq *);

/**
 * Enqueue message to message queue
 *
 * @return 0 if success. Otherwise @c -errno
 * 	(EAGAIN means 'Q is already full')
 */
YYEXPORT int
ymsgq_en(struct ymsgq *, struct ymsg *);

/**
 * Dequeue message from message queue.
 * This is blocking function.
 * So, if queue is empty, caller is blocked.
 *
 * @return NULL if fails.
 */
YYEXPORT struct ymsg *
ymsgq_de(struct ymsgq *);

/**
 * Get size(number of messsage) of message queue.
 *
 * @return Size
 */
YYEXPORT uint32_t
ymsgq_sz(const struct ymsgq *);

#endif /* __YMSGq_h__ */
