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
 * @file ymsglooper.h
 * @brief Header to use message looper.
 */

#ifndef __YMSGLOOPEr_h__
#define __YMSGLOOPEr_h__

#include <pthread.h>

#include "ydef.h"

struct ymsgq;

/** Message looper object(opaque) */
struct ymsglooper;

/**
 * State of msglopoer.
 */
enum ymsglooper_state {
	/**
	 * Prepared and msglooper is attached to the thread. But not in the
	 *   msg loop
	 */
	YMSGLOOPER_READY,
	/**
	 * In the message loop
	 */
	YMSGLOOPER_LOOP,
	/**
	 * In the message loop. But stop signal is delivered to the looper.
	 */
	YMSGLOOPER_STOPPING,
	/**
	 * It exits from message loop. So, messages in the message Q is NOT
	 *   handled anymore.
	 */
	YMSGLOOPER_TERMINATED,
};

/**
 * Prepare message looper for current thread.
 * If success, returned looper is assigned to current thread.
 *
 * @param msgq_capacity capacity of message Q.
 *                      '<= 0' means 'unlimited capacity'
 * @param destroy_on_exit destroy instance on exiting thread
 * @return 0 if success. Otherwise -errno
 *         (ex. -EPERM if there is looper already at current thread.)
 */
YYEXPORT int
ymsglooper_create(int msgq_capacity, bool destroy_on_exit);

/**
 * Destroy looper instance.
 * Looper SHOULD be in TERMINATED state.
 *
 * @return 0 if success otherwise -errno.
 *         (Ex. if looper state is NOT TERMINATED, -EPERM is returned.)
 */
YYEXPORT int
ymsglooper_destroy(struct ymsglooper *);

/**
 * Start loop of current-thread-looper.
 *
 * @return 0 if success. Otherwise -errno
 *         (ex. -EPERM if there is no looper at current thread.)
 */
YYEXPORT int
ymsglooper_loop(void);

/**
 * Start new msg looper thread(new thread is started and enter msg looper).
 * Returned value MUST NOT freed by client. It will be handled internally.
 *
 * @param destroy_on_exit destroy instance on exiting thread
 * @return NULL if fails. Otherwise, struct ymsglooper of newly started thread.
 */
YYEXPORT struct ymsglooper *
ymsglooper_start_looper_thread(bool destroy_on_exit);

/**
 * Get message Q of this looper.
 * This function MUST NOT be called when/after looper is finished(stopped).
 * (That is, it's available only while looping.)
 *
 * @return message Q. Tt SHOULD NOT be NULL.
 */
YYEXPORT struct ymsgq *
ymsglooper_get_msgq(struct ymsglooper *);

/**
 * Get thread that looper is belongs to.
 * This function MUST NOT be called when/after looper is finished(stopped).
 * (That is, it's available only while looping.)
 *
 * @return thread.
 */
YYEXPORT pthread_t
ymsglooper_get_thread(const struct ymsglooper *);

/**
 * Get message looper belonging to current thread.
 *
 * @return msglooper of current thread. NULL if there is no looper.
 */
YYEXPORT struct ymsglooper *
ymsglooper_get(void);

/**
 * Get state of the msglooper.
 */
YYEXPORT enum ymsglooper_state
ymsglooper_get_state(struct ymsglooper *);

/**
 * Stop looper. This is async function.
 *
 * @return 0 if success otherwise -errno.
 */
YYEXPORT int
ymsglooper_stop(struct ymsglooper *);



#endif /* __YMSGLOOPEr_h__ */
