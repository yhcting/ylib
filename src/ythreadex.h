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
 * @file ythreadex.h
 * @brief Header
 */

#ifndef __YTHREADEx_h__
#define __YTHREADEx_h__

#include "ydef.h"



/*
 * Thread state transition
 * =======================
 *
 * Notation
 * --------
 * [] : state
 * () : action
 *
 *
 * --> [ READY ] ----------+(cancel)
 *         |               |
 *         v               |
 *     [ STARTED ]         |
 *          | <on_started> |
 *        +-+-+            |
 *        |   |(cancel)    v
 *        |   +--> [ CANCELLING ]
 *        v               | <on_cancelling>
 *     [ DONE ]           +----->[ CANCELLED ]
 *        | <on_done>                 | <on_cancelled>
 *        v                           v
 *   [ TERMINATED ]        [ TERMINATED_CANCELLED ]
 *
 *
 * <on_progress_init> and <on_progress> MUST be called only at STARTED and CANCELLING state.
 *
 * NOTE:
 * - All callbacks are run on Owner's context.
 * - Order of callbacks are guaranteed.
 *   So, only following 3-cases are allowed.
 *     . <on_cancel> -> <on_cancelled>
 *     . <on_started> -> <on_cancel> -> <on_cancelled>
 *     . <on_started> -> <on_post_run>
 */



struct ythreadex;
struct ymsghandler;


/**
 * Thread priority(NOT Effective(not implemented), yet)
 */
enum ythreadex_priority {
	YTHREADEX_HIGHER,
	YTHREADEX_HIGH,
	YTHREADEX_NORMAL,
	YTHREADEX_LOW,
	YTHREADEX_LOWER,
};

/**
 * State of thread
 */
enum ythreadex_state {
	YTHREADEX_READY, /**< before new thread is running */
	YTHREADEX_STARTED, /**< new thread is started. */
	YTHREADEX_CANCELLING, /**< thread is cancelling */
        /**
         * New thread is cancelled. But, post processing(sending
         *   \a on_cancelled notification message to owner thread
         *   is not done yet.
         */
	YTHREADEX_CANCELLED,
        /**
         * New thread is done. But, post processing(sending
         *   \a on_post_run notification message to owner thread
         *   is not done yet.
         */
	YTHREADEX_DONE,
        /**
         * All jobs are done.
         */
	YTHREADEX_TERMINATED,
        /**
         * All jobs are done after cancel.
         */
	YTHREADEX_TERMINATED_CANCELLED,
};

/**
 * Listener interface of threadex.
 */
struct ythreadex_listener {
        /**
         * \a ythreadex enter STARTED state.
         */
	void (*on_started)(struct ythreadex *);
        /**
         * This message is sent after DONE state.
         */
	void (*on_done)(struct ythreadex *, void *result, int errcode);
	/**
	 * \a started is TRUE if cancel is requrested after thread is started
	 */
	void (*on_cancelling)(struct ythreadex *, bool started);
        /**
         * This message is sent after CANCELLED state.
         */
	void (*on_cancelled)(struct ythreadex *, int errcode);
        /**
         * Progress is ready.
         */
	void (*on_progress_init)(struct ythreadex *, long max_prog);
        /**
         * Progress notification
         */
	void (*on_progress)(struct ythreadex *, long prog);
};


/**
 * Once \a result is assigned in side \a run (user-thread-function),
 *   ythreadex module has responsibility for freeing memory by calling
 *   \a free_result.
 *
 * @param name DEEP-COPIED value is used. Can be NULL.
 * @param owner Passed by reference. Can be NULL.
 * @param priority Priority
 * @param listener DEEP-COPIED struct is used. Can be NULL.
 * @param arg argument used at \a run. Can be NULL.
 * @param free_arg Function to free \a arg. Can be NULL.
 * @param free_result Function to free return of \a run. Can be NULL.
 * @param run Routine run at new thread. Not NULL.
 *            It MUST return 0 if success, otherwise -errno.
 *            In case of not-success, result of threadex is ignored.
 * @return NULL if fails.
 */
YYEXPORT struct ythreadex *
ythreadex_create(const char *name,
		 struct ymsghandler *owner,
		 enum ythreadex_priority priority,
		 const struct ythreadex_listener *listener,
		 void *arg,
		 void (*free_arg)(void *),
		 void (*free_result)(void *),
		 int (*run)(struct ythreadex *, void **result));

/**
 * Destroy instance.
 * This MUST be called when thread state is TERMINATED(_CANCELLED).
 * If thread is in READY state, call this fuction after {@link ythreadex_cancel}
 * This function frees all memories allocated. But it doens't do anything
 *   to background thread.
 * @return 0 if success otherwise -errno.
 *         (Ex. thread is in invalid state, -EPERM is returned.)
 */
YYEXPORT int
ythreadex_destroy(struct ythreadex *);

/**
 * @return 0 if success. Otherwise -errno
 *         (ex. -EPERM if thread is already started.)
 */
YYEXPORT int
ythreadex_start(struct ythreadex *);

/**
 * @return 0 if success. Otherwise -errno
 *         (ex. -EPERM if thread is already started.)
 */
YYEXPORT int
ythreadex_start_sync(struct ythreadex *);

/**
 * Wait until thread is finished.
 * This is ythreadex version of pthread_join.
 *
 * @return 0 if success. Otherwise -errno.
 *         (ex. -EPERM if thread is NOT started yet or started by
 *          {@link ythreadex_start_sync}.)
 */
YYEXPORT int
ythreadex_join(struct ythreadex *, void **retval);

/**
 * Publishing progress is only allowed at \a YTHREADEX_STARTED state.
 * -EPERM may be returned if progress is not initialized or not in
 * STARTED state.
 *
 * @return 0 if success. Otherwise -errno
 */
YYEXPORT int
ythreadex_publish_progress_init(struct ythreadex *, long maxprog);

/**
 * Publishing progress is only allowed at \a YTHREADEX_STARTED state.
 * -EPERM may be returned if progress is not initialized or not in
 * STARTED state.
 *
 * @return 0 if success. Otherwise -errno
 */
YYEXPORT int
ythreadex_publish_progress(struct ythreadex *, long prog);


/**
 * User SHOULD be very careful to use pthread_cancel(pthdcancel = TRUE).
 * pthread_cancel may lead to thread being exited in the middle of execution.
 * In this case, memory allocated in user-thread-function doesn't have any
 *   chance to be freed.
 * So, user SHOULD consider this case in the thread function
 * (Using pthread_push_cleanup may be one opion.)
 *
 * @param pthdcancel If it is True, \a pthread_cancel is applied.
 * @return 0 if success. Otherwise -errno
 *         (ex -EPERM if thread is NOT in READY or STARTED.)
 */
YYEXPORT int
ythreadex_cancel(struct ythreadex *, bool pthdcancel);

/**
 * Get state of thread.
 */
YYEXPORT enum ythreadex_state
ythreadex_get_state(struct ythreadex *);

/**
 * Get argument of thread. This is the value \a arg passed via
 *   {@link ythreadex_create}
 */
YYEXPORT void *
ythreadex_get_arg(struct ythreadex *);
/**
 * Getting result of thread.
 * Value may be unstable while thread is in
 *   READY, READY, STARTED or CANCELLING state.
 * Therefore read value while thread is in DONE, CANCELLED, or
 *   TERMINATED(_CANCELLED) state.
 *
 * If thread is cancelled, result value is undefined.
 *
 * Caller MUST NOT free result object.
 * result-object is freed at {@link ythreadex_destroy}.
 */
YYEXPORT void *
ythreadex_get_result(struct ythreadex *);

/**
 * Getting error code of thread.
 * Value may be unstable while thread is in
 *   READY, READY, STARTED or CANCELLING state.
 * Therefore read value while thread is in DONE, CANCELLED, or
 *   TERMINATED(_CANCELLED) state.
 */
YYEXPORT int
ythreadex_get_errcode(struct ythreadex *);

/**
 * Get priority of thread. This is the value \a priority passed via
 *   {@link ythreadex_create}
 */
YYEXPORT enum ythreadex_priority
ythreadex_get_priority(struct ythreadex *);

/**
 * Get owner of thread. This is the value \a owner passed via
 *   {@link ythreadex_create}
 */
YYEXPORT struct ymsghandler *
ythreadex_get_owner(struct ythreadex *);

/**
 * Get name of thread. This is the value \a name passed via
 *   {@link ythreadex_create}
 */
YYEXPORT const char *
ythreadex_get_name(struct ythreadex *);

/**
 * Get ythreadex-unique-id. All ythreadex has different id.
 */
YYEXPORT long
ythreadex_get_id(struct ythreadex *);

/**
 * Is in active state?
 * Active state includes STARTED, CANCELLING, CANCELLED, DONE
 */
static YYINLINE bool
ythreadex_is_active(enum ythreadex_state state) {
	switch (state) {
	case YTHREADEX_STARTED:
	case YTHREADEX_CANCELLING:
	case YTHREADEX_CANCELLED:
	case YTHREADEX_DONE:
		return TRUE;
	default:
		return FALSE;
	}
}

/**
 * Is thread cancel?
 * If thread is one of CANCELLING, CANCELLED, TERMINATED_CANCELLED, TRUE is
 *   returned.
 */
static YYINLINE bool
ythreadex_is_cancel(enum ythreadex_state state) {
	switch (state) {
	case YTHREADEX_CANCELLING:
	case YTHREADEX_CANCELLED:
	case YTHREADEX_TERMINATED_CANCELLED:
		return TRUE;
	default:
		return FALSE;
	}
}

/**
 * Is thread terminated?
 * TRUE if thread is TERMINATED or TERMINATED_CANCELLED state.
 */
static YYINLINE bool
ythreadex_is_terminated(enum ythreadex_state state) {
	switch (state) {
	case YTHREADEX_TERMINATED:
	case YTHREADEX_TERMINATED_CANCELLED:
		return TRUE;
	default:
		return FALSE;
	}
}

#endif /* __YTHREADEx_h__ */
