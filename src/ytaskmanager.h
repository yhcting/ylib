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
 * @file ytaskmanager.h
 * @brief Header
 */

#ifndef __YTASKMANAGEr_h__
#define __YTASKMANAGEr_h__

#include <pthread.h>

#include "ydef.h"

/**
 * Queue type inside task manager.
 */
enum ytaskmanager_qtype {
	YTASKMANAGERQ_INVALIDQ, /**< invalid queue */
	YTASKMANAGERQ_READY,
	YTASKMANAGERQ_RUN,
};

/**
 * Event emitted in case that internal Queue state is changed.
 */
enum ytaskmanager_qevent {
	YTASKMANAGERQ_ADDED_TO_READY,
	YTASKMANAGERQ_REMOVED_FROM_READY,
	YTASKMANAGERQ_MOVED_TO_RUN,
	YTASKMANAGERQ_REMOVED_FROM_RUN,
};

struct ytaskmanager;
struct ymsghandler;
struct ytask;

/**
 * Queue event listener struct.
 * At this moment, only one function is in the struct.
 * But, this is for future expansion.
 */
struct ytaskmanager_qevent_listener {
	/**
	 * Callback function executed at owner's context.
	 */
 	void (*on_event)(struct ytaskmanager *,
			 enum ytaskmanager_qevent,
			 int readyq_sz, /**< Size of readyQ */
			 int runq_sz, /**< Size of runQ */
			 struct ytask *);
};

/**
 * Creating taskmanager instance.
 *
 * @param owner Context owner in where listener is executed.
 * @param slots Maximum number of slots in where task can run.
 *              That is, this value is max-number of concurrency.
 *              < 0 means not-limited.
 * @return NULL if fails.
 */
struct ytaskmanager *
ytaskmanager_create(struct ymsghandler *owner,
		    int slots);

/**
 * -EPERM may be return in case that there are some non-terminated tasks in
 *   the manager.
 *
 * @return 0 for success. Otherwise -errno.
 */
int
ytaskmanager_destroy(struct ytaskmanager *);

/**
 * Get owner.
 */
struct ymsghandler *
ytaskmanager_get_owner(struct ytaskmanager *);

/**
 * Get number of concurrency slots. See {@link ytaskmanager_create}
 */
int
ytaskmanager_get_slots(struct ytaskmanager *);


/**
 * Tag is destroied by using \a tagfree when task is destroied.
 *
 * @return # of newly added tag (0 means overwritten). -errno if fails.
 */
int
ytaskmanager_add_tag(struct ytaskmanager *,
		     const char *key,
		     void *tag,
		     void (*tagfree)(void *));

/**
 * Get tag. But tag is still in the task.
 * That is, tag is destroied too, when task is destroied.
 * @return Tag object
 */
void *
ytaskmanager_get_tag(struct ytaskmanager *,
		     const char *key);

/**
 * Removes tag.
 *
 * @return Number of deleted tags (0 means nothing deleted).
 *         -errno if fails.
 */
int
ytaskmanager_remove_tag(struct ytaskmanager *,
			const char *key);

/**
 * Get task-queue position in where the task is currently staying.
 *
 * @return Queue type
 */
enum ytaskmanager_qtype
ytaskmanager_contains(struct ytaskmanager *, struct ytask *);

/**
 * Number of tasks in the task manager(ready + run).
 */
int
ytaskmanager_size(struct ytaskmanager *);

/**
 * ytask can be added to only one ytaskmanager!
 */
int
ytaskmanager_add_task(struct ytaskmanager *, struct ytask *);

/**
 * Cancel task.
 * If task is in the ready queue, task is removed immediately.
 */
int
ytaskmanager_cancel_task(struct ytaskmanager *, struct ytask *);

/**
 * @param arg Argument passed to \a match function
 * @param match Return whether task matches requirement.
 *              TRUE is returned by \a match, the task becomes final return
 *                value of this interface function.
 * @return Task matched.
 */
struct ytask *
ytaskmanager_find_task(struct ytaskmanager *,
		       void *arg,
		       bool (*match)(struct ytask *, void *));

/**
 * Add queue event listener.
 *
 * @param owner Handler in where listener is executed.
 * @return Handle of event listener. NULL is returned if fails.
 */
void *
ytaskmanager_add_qevent_listener
(struct ytaskmanager *,
 struct ymsghandler *owner,
 const struct ytaskmanager_qevent_listener *);

/**
 * Add queue event listener executed on taskmanager's msghandler context.
 * See {@link ytaskmanager_add_qevent_listener}
 */
void *
ytaskmanager_add_qevent_listener2
(struct ytaskmanager *tm,
 const struct ytaskmanager_qevent_listener *qel) {
	return ytaskmanager_add_qevent_listener
		(tm, ytaskmanager_get_owner(tm), qel);
}

/**
 * @param el_handle Handle returned by {@link ytaskmanager_add_qevent_listener}
 * @return 0 if success. Otherwise -errno.
 */
int
ytaskmanager_remove_qevent_listener
(struct ytaskmanager *, void *el_handle);


#endif /* __YTASKMANAGEr_h__ */
