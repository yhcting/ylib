/******************************************************************************
 * Copyright (C) 2016, 2023
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
 * @file task.h
 * @brief Header to use ytask in inside library
 */

#pragma once

#include <pthread.h>

#include "ytask.h"
#include "def.h"
#include "threadex.h"
#include "ylistl.h"
#include "yhash.h"
#include "ygp.h"


struct ytask {
	/* -------- READ ONLY values(set only once --------*/
	struct ythreadex t; /* MUST be on TOP of the struct */
	bool pthdcancel; /* See ytask_create at ytask.h */
	struct ytask_listener listener;
	/* ---------- Dynamically updated values ----------*/
	struct yhash *tagmap;
	pthread_mutex_t tagmap_lock;
	struct ylistl_link elhhd; /**< header of event listener handle */
	pthread_mutex_t elh_lock;
	struct ygp gp;
	struct {
		u64 interval; /**< interval between progress. ms */
		u64 pubtm; /**< last publish time. ms */
		long max; /**< max progress value */
		long prog; /**< current progress value */
#ifdef CONFIG_DEBUG
		volatile bool init; /**< progress is initialized */
#endif /* CONFIG_DEBUG */
	} prog;
	/* ------------------------------------------------
	 * Values used only inside library.
	 * These are to support other library modules.
	 *
	 * [NOTE]
	 * In terms of loose-coupling, Using internal hashmap like 'tagmap'
	 *   is better.
	 * But, it's slow and requires more memory space.
	 */
	/** Values used by taskmanager. */
	void *tmtag;
	/** Values used by taskdepman. */
	void *tdmtag;
};

struct ytask_event_listener_handle {
	struct ymsghandler *owner;
	/**
	 * Fields used inside task module. This value is initialized inside
	 * task module.
	 */
	struct ylistl_link lk;
	/**
	 * {@code el} MUST be at the end of this struct, because
	 * {@code struct ytask_event_listener} has extra bytes at the end of
	 * struct
	 */
	struct ytask_event_listener el;
};

/**
 * @return 0 if success. -errno if fails.
 */
int
task_init(
	struct ytask *,
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
 * @return 0 if success. -errno if fails.
 */
int
task_clean(struct ytask *);

static INLINE int
task_get(struct ytask *tsk) {
	return ygpget(&tsk->gp);
}

static INLINE int
task_put(struct ytask *tsk) {
	return ygpput(&tsk->gp);
}

static INLINE int
task_refcnt(struct ytask *tsk) {
	return ygpref_cnt(&tsk->gp);
}
