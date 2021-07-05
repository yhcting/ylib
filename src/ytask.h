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
 * @file ytask.h
 * @brief Header
 */

#ifndef __YTASk_h__
#define __YTASk_h__

#include "ydef.h"
#include "ylistl.h"
#include "ythreadex.h"


/**
 * Inheritted struct from ythreadex.
 */
struct ytask;

/**
 * Listener attached to ytask module.
 * Only one listener can be attached to one ytask instance.
 * @see ytask_create
 */
struct ytask_listener {
        /**
         * @c STARTED ythreadex state, and BEFORE notifying to listeners.
         * NULL is allowed.
         */
	void (*on_early_started)(struct ytask *);
        /**
         * @c STARTED ythreadex state, and AFTER notifying to listeners.
         * NULL is allowed.
         */
	void (*on_late_started)(struct ytask *);
        /**
         * @c DONE ythreadex state, and BEFORE notifying to listeners.
         * NULL is allowed.
         */
	void (*on_early_done)(struct ytask *, void *result, int errcode);
        /**
         * @c DONE ythreadex state, and AFTER notifying to listeners.
         * NULL is allowed.
         */
	void (*on_late_done)(struct ytask *, void *result, int errcode);
        /**
         * @c CANCELLING ythreadex state, and BEFORE notifying to listeners.
         * NULL is allowed.
         */
	void (*on_early_cancelling)(struct ytask *, bool started);
        /**
         * @c CANCELLING ythreadex state, and AFTER notifying to listeners.
         * NULL is allowed.
         */
	void (*on_late_cancelling)(struct ytask *, bool started);
        /**
         * @c CANCELLED ythreadex state, and BEFORE notifying to listeners.
         * NULL is allowed.
         */
	void (*on_early_cancelled)(struct ytask *, int errcode);
        /**
         * @c CANCELLED ythreadex state, and AFTER notifying to listeners.
         * NULL is allowed.
         */
	void (*on_late_cancelled)(struct ytask *, int errcode);
        /**
         * @c BEFORE notifying progress init to listeners.
         * NULL is allowed.
         */
	void (*on_early_progress_init)(struct ytask *, long max_prog);
        /**
         * @c AFTER notifying progress init to listeners.
         * NULL is allowed.
         */
	void (*on_late_progress_init)(struct ytask *, long max_prog);
        /**
         * @c BEFORE notifying progress to listeners.
         * NULL is allowed.
         */
	void (*on_early_progress)(struct ytask *, long prog);
        /**
         * @c AFTER notifying progress to listeners.
         * NULL is allowed.
         */
	void (*on_late_progress)(struct ytask *, long prog);
};


/**
 * Handle created and returned inside library
 */
struct ytask_event_listener_handle;

/**
 * Event listener registered to ytask.
 * Multiple event listeners can be registered to one ytask instance.
 * Shallow-copied struct is used inside library.
 * @see ytask_add_event_listener.
 */
struct ytask_event_listener {
        /**
         * Entered STARTED state.
         */
	void (*on_started)(struct ytask_event_listener *, struct ytask *);
        /**
         * This message is sent after DONE state.
         */
	void (*on_done)(
		struct ytask_event_listener *,
		struct ytask *,
		void *result,
		int errcode);
	/**
	 * @p started is TRUE if cancel is requrested after task is started
	 */
	void (*on_cancelling)(
		struct ytask_event_listener *,
		struct ytask *,
		bool started);
        /**
         * This message is sent after CANCELLED state.
         */
	void (*on_cancelled)(
		struct ytask_event_listener *,
		struct ytask *, int errcode);
        /**
         * Progress is ready.
         */
	void (*on_progress_init)(
		struct ytask_event_listener *,
		struct ytask *,
		long max_prog);
        /**
         * Progress notification
         */
	void (*on_progress)(
		struct ytask_event_listener *,
		struct ytask *,
		long prog);
	/**
	 * Function to free extra data. This can be NULL.
	 * NULL means, nothing to do for freeing extra data.
	 * Argument of this function is
	 * `&(struct ytask_event_listener).extra`.
	 * So, argument itself MUST NOT be freed in it.
	 */
	void (*free_extra)(void *, unsigned int extrasz);
	unsigned int extrasz; /**< size of extra data in bytes */
	char extra[0]; /**< extra user data */
};

/* >>> @cond */
/*
 * This function is ONLY for INTERNAL use.
 */
static YYINLINE struct ythreadex *
__ytask_super(struct ytask *tsk) {
        return (struct ythreadex *)tsk;
}
/* <<< @endcond */

/**
 * Once @c result is assigned inside @p run (user-thread-function),
 * ythreadex module has responsibility for freeing memory by calling
 * @p free_result.
 *
 * @param name See @ref ythreadex_create
 * @param owner See @ref ythreadex_create
 * @param priority See @ref ythreadex_create
 * @param listener DEEP-COPIED struct is used. Can be NULL.
 * @param arg See @ref ythreadex_create
 * @param free_arg See @ref ythreadex_create
 * @param free_result See @ref ythreadex_create
 * @param run See @ref ythreadex_create
 * @param pthdcancel To cancel task, @c pthread_cancel is used.
 * @return NULL if fails.
 */
YYEXPORT struct ytask *
ytask_create3(
	const char *name,
	struct ymsghandler *owner,
	enum ythreadex_priority priority,
	const struct ytask_listener *listener,
	void *arg,
	void (*free_arg)(void *),
	void (*free_result)(void *),
	int (*run)(struct ytask *, void **result),
	bool pthdcancel);

/**
 * @see ytask_create3
 */
static YYINLINE struct ytask *
ytask_create2(
	const char *name,
	struct ymsghandler *owner,
	const struct ytask_listener *listener,
	void *arg,
	void (*free_arg)(void *),
	void (*free_result)(void *),
	int (*run)(struct ytask *, void **result),
	bool pthdcancel
) {
	return ytask_create3(name, owner, YTHREADEX_NORMAL, listener,
		arg, free_arg, free_result, run, pthdcancel);
}

/**
 * @see ytask_create2
 */
static YYINLINE struct ytask *
ytask_create(
	const char *name,
	struct ymsghandler *owner,
	void *arg,
	void (*free_arg)(void *),
	void (*free_result)(void *),
	int (*run)(struct ytask *, void **result)
) {
	return ytask_create2(name, owner, NULL,
		arg, free_arg, free_result, run, FALSE);
}

/**
 * Destroy ytask instance.
 *
 * @return 0 if success otherwise @c -errno.
 *	(Ex. thread is in invalid state(ex. not-terminated yet.),
 *	-EPERM is returned.)
 */
YYEXPORT int
ytask_destroy(struct ytask *);

/**
 * @see ythreadex_start
 */
static YYINLINE int
ytask_start(struct ytask *tsk) {
        return ythreadex_start(__ytask_super(tsk));
}

/**
 * @see ythreadex_start_sync
 */
static YYINLINE int
ytask_start_sync(struct ytask *tsk) {
        return ythreadex_start_sync(__ytask_super(tsk));
}

/**
 * Wait until given task is done and get result.
 * @see ythreadex_join
 */
static YYINLINE int
ytask_join(struct ytask *tsk, void **retval) {
        return ythreadex_join(__ytask_super(tsk), retval);
}

/**
 * Cancel task.
 * @c pthdcancel value passed at @ref ytask_create, is used.
 * @see ythreadex_cancel
 */
YYEXPORT int
ytask_cancel(struct ytask *tsk);

/**
 * It is recommended to call this initialization function only once.
 * But, it's totally up to client with it's own risk.
 *
 * @param maxprog non-zero value.
 * @return 0 if success. Otherwise @c -errno (-EINVAL if `maxprog == 0`).
 */
YYEXPORT int
ytask_publish_progress_init(struct ytask *, long maxprog);

/**
 * -EPERM may be returned if trying to publish again in short time.
 *
 * @return 0 if success. Otherwise @c -errno.
 */
YYEXPORT int
ytask_publish_progress(struct ytask *, long prog);

/**
 * Get state of task. @see ythreadex_get_state.
 */
static YYINLINE enum ythreadex_state
ytask_get_state(struct ytask *tsk) {
        return ythreadex_get_state(__ytask_super(tsk));
}

/**
 * @see ythreadex_get_arg.
 */
static YYINLINE void *
ytask_get_arg(struct ytask *tsk) {
        return ythreadex_get_arg(__ytask_super(tsk));
}

/**
 * @see ythreadex_get_result.
 */
static YYINLINE void *
ytask_get_result(struct ytask *tsk) {
        return ythreadex_get_result(__ytask_super(tsk));
}

/**
 * @see ythreadex_get_errcode.
 */
static YYINLINE int
ytask_get_errcode(struct ytask *tsk) {
        return ythreadex_get_errcode(__ytask_super(tsk));
}

/**
 * @see ythreadex_get_priority.
 */
static YYINLINE enum ythreadex_priority
ytask_get_priority(struct ytask *tsk) {
        return ythreadex_get_priority(__ytask_super(tsk));
}

/**
 * @see ythreadex_get_owner.
 */
static YYINLINE struct ymsghandler *
ytask_get_owner(struct ytask *tsk) {
        return ythreadex_get_owner(__ytask_super(tsk));
}

/**
 * @see ythreadex_get_name.
 */
static YYINLINE const char *
ytask_get_name(struct ytask *tsk) {
        return ythreadex_get_name(__ytask_super(tsk));
}

/**
 * @see ythreadex_get_id.
 */
static YYINLINE long
ytask_get_id(struct ytask *tsk) {
        return ythreadex_get_id(__ytask_super(tsk));
}

/**
 * Check with current state of given tsk.
 * @see ythreadex_is_active.
 */
static YYINLINE bool
ytask_is_active(struct ytask *tsk) {
	return ythreadex_is_active(ytask_get_state(tsk));
}

/**
 * Check with current state of given tsk.
 * @see ythreadex_is_cancel.
 */
static YYINLINE bool
ytask_is_cancel(struct ytask *tsk) {
	return ythreadex_is_cancel(ytask_get_state(tsk));
}

/**
 * Check with current state of given tsk.
 * @see ythreadex_is_terminated.
 */
static YYINLINE bool
ytask_is_terminated(struct ytask *tsk) {
	return ythreadex_is_terminated(ytask_get_state(tsk));
}

/**
 * Tag is destroied by using @p tagfree when task is destroied.
 *
 * @return # of newly added tag (0 means overwritten). @c -errno if fails.
 */
YYEXPORT int
ytask_add_tag(
	struct ytask *,
	const char *name,
	void *tag,
	void (*tagfree)(void *));

/**
 * Get tag. But tag is still in the task.
 * That is, tag is destroied too, when task is destroied.
 */
YYEXPORT void *
ytask_get_tag(struct ytask *, const char *);

/**
 * Remove tag.
 *
 * @return Number of deleted tags (0 means nothing deleted).
 *	@c -errno if fails.
 */
YYEXPORT int
ytask_remove_tag(struct ytask *, const char *);

/**
 * Add event listener.
 *
 * @param event_listener_owner context owner in where listener is executed.
 * @param yel DEEP-COPIED struct is used. Can be NULL.
 * @param progress_notice if TRUE, current progress is published to new
 *	new listener immediately.
 * @return handle for newly added event listener.
 *	NULL if fail to add event listener(May be ENOMEM?)
 */
YYEXPORT struct ytask_event_listener_handle *
ytask_add_event_listener2(
	struct ytask *,
	struct ymsghandler *event_listener_owner,
	const struct ytask_event_listener *yel,
	bool progress_notice);

/**
 * @see ytask_add_event_listener2
 */
static YYINLINE struct ytask_event_listener_handle *
ytask_add_event_listener(
	struct ytask *tsk,
	const struct ytask_event_listener *yel,
	bool progress_notice
) {
        return ytask_add_event_listener2(
		tsk,
		ytask_get_owner(tsk),
		yel,
		progress_notice);
}


/**
 * Remove event listener. After this function, {@code event_listener_handle} is
 * no more valule.
 */
YYEXPORT int
ytask_remove_event_listener(
	struct ytask *,
	struct ytask_event_listener_handle *);


#endif /* __YTASk_h__ */
