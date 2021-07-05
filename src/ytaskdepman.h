/******************************************************************************
 * Copyright (C) 2016, 2017
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
 * @file ytaskdepman.h
 * @brief Header
 */

#ifndef __YTASKDEPMAn_h__
#define __YTASKDEPMAn_h__

#include "ydef.h"

struct ymsghandler;
struct ytask;
struct ytaskdepman;

/**
 * Return code showing verification result.
 */
enum ytaskdepman_errcode {
	YTASKDEPMAN_OK = 0, /**< ytaskdepman is ready to start */
	YTASKDEPMAN_EMPTY, /**< No task available */
	YTASKDEPMAN_CIRCULAR_DEP, /**< Circular dependency is detected */
	YTASKDEPMAN_MULTI_ROOT, /**< More than one root tasks are detectded */
	/**
	 * Isolated task(Tasks there is NO dependency path from root task)
	 *   is detected.
	 */
	YTASKDEPMAN_ISOLATED_TASK,
};


/**
 * @c on_done passes issued task(tasks your are interested in.)
 * In case of success, it gives target task having no-error.
 * Otherwise, this gives the first task failed to run(having error code).`
 * That is, "No error at issued task" means "success".
 * And you can use it's result.
 * It case of 'cancelled' or error at @c ytaskdepman itself, @c on_done
 * gives NULL as issued task.
 * To tell cancelled or error, refer error code returned by
 * @ref ytaskdepman_get_errcode.
 *
 * @param slots Number of concurrent jobs run.
 * @param on_done callback executed after tasks are all done.
 * @return NULL if fails.
 */
YYEXPORT struct ytaskdepman *
ytaskdepman_create(
	struct ymsghandler *,
	void (*on_done)(struct ytaskdepman *, struct ytask *),
	int slots);

/**
 * -EPERM if it is under progress.
 *
 * @return 0 if success. Otherwise @c -errno.
 */
YYEXPORT int
ytaskdepman_destroy(struct ytaskdepman *);

/**
 * Get error code. 0 if success(done or cancelled).
 */
YYEXPORT int
ytaskdepman_get_errcode(struct ytaskdepman *);

/**
 * @return 0 if success. Otherwise @c -errno.
 */
YYEXPORT int
ytaskdepman_add_task(struct ytaskdepman *, struct ytask *);

/**
 * The function fails is ytaskdepman already started or task is not in the
 * @c ytaskdepman.
 *
 * @return 0 if success. Otherwise @c -errno.
 */
YYEXPORT int
ytaskdepman_remove_task(struct ytaskdepman *, struct ytask *);

/**
 * Add dependency between two tasks. @p target task run after @p prereq task.
 * -EPERM is returned if newly added dependency makes circular dependency.
 *
 * @param target Target task.
 * @param prereq Prerequisite task.
 * @return 0 if success. Otherwise @c -errno.
 */
YYEXPORT int
ytaskdepman_add_dependency(
	struct ytaskdepman *,
	struct ytask *target,
	struct ytask *prereq);

/**
 * Remove dependency between two tasks.
 * See @ref ytaskdepman_add_dependency for details of each parameters.
 */
YYEXPORT int
ytaskdepman_remove_dependency(
	struct ytaskdepman *,
	struct ytask *target,
	struct ytask *prereq);

/**
 * @param roottsk root(target) task. NULL is allowed.
 * @return enum @ref ytaskdepman_errcode value if success.
 * 	Otherwise @c -errno.
 */
YYEXPORT int
ytaskdepman_verify(struct ytaskdepman *, struct ytask **roottsk);

/**
 * Start tasks based on their dependencies.
 * If prerequisite condition is NOT satisfied, error code is returned.
 * For detail error information, use @ref ytaskdepman_verify
 */
YYEXPORT int
ytaskdepman_start(struct ytaskdepman *);


/**
 * Cancel all tasks.
 */
YYEXPORT int
ytaskdepman_cancel(struct ytaskdepman *);

#endif /* __YTASKDEPMAn_h__ */
