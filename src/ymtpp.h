/******************************************************************************
 * Copyright (C) 2014, 2015
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
 * @file ymtpp.h
 * @brief \b [DRAFT] Header to use MultiThread Parallel Processing module
 *
 * WARNING This module is NOT STABLE yet.
 * This module helps to run jobs in multithread mode based on dependency
 * defined.
 * This can also control number of threads run in same time.
 */

/******************************************************************************
 * CONCEPTS
 * ========
 *
 * [job A] ---+
 *	      |
 * [job B] ---+--> [job C] <-- [Additional argument]
 *
 * <<job C>>
 *   Input:
 *     * (name, return value) list of each previous job.
 *	 In this case, [job A] and [job B]
 *     * Additional argument set at 'struct ymtppjob.arg'
 *
 *   Output:
 *     return value of job thread.
 *
 *   Clean up at the end of thread:
 *     Clean argument by calling 'free_arg' callback function.
 *     Clean return value of [job A] and [job B] by calling 'free_ret' callback
 *	 function.
 *****************************************************************************/

#ifndef __YMTPp_h__
#define __YMTPp_h__

#include "ydef.h"

/** Maximum length of job name */
#define YMTPP_JOB_NAME_LEN 31

/** Mtpp object */
struct ymtpp;

/**
 * Mtpp task object
 * Object SHOULD NOT be handled as 'struct yvertex' directly.
 */
typedef struct yvertex * ymtpp_task_t;

/** Job argument */
struct ymtppjobarg {
	const char *name; /**< Job name. See {@link ymtppjob::name} */
	const void *data; /**< Data returned from the job */
};

/**
 * Main structure of job.
 */
struct ymtppjob {
	char name[YMTPP_JOB_NAME_LEN + 1]; /**< Job name. */
	/**
	 * Function to run as a job.
	 * @param out value from function.
	 * @param arg argument
	 * @param jargs
	 *     job argument
	 *     Array of arguments which ends with empty data - '{0, 0}'.
	 *     This is value passed from previous jobs.
	 * @return 0 if success. Otherwise <0.
	 */
	int (*run)(void **out,
		   const void *arg,
		   const struct ymtppjobarg *jargs);
	void *arg; /**< argument passed in addition to previous return data */
	void (*afree)(void *); /**< function to free argument object */
	void (*ofree)(void *); /**< function to free returned object */
};

/******************************************************************************
 *
 * ymtpp_task
 *
 *****************************************************************************/
/**
 * Get mtppjob assigned to the task.
 */
struct ymtppjob *
ymtpp_task_job(ymtpp_task_t);

/******************************************************************************
 *
 * ymtpp
 *
 *****************************************************************************/
/**
 * Create module object and initialize it.
 *
 * @param maxjobs Maximum number of jobs that can be run at the same time.
 *                <=0 means 'infinite'
 * @return NULL if fails. Otherwise success.
 */
struct ymtpp *
ymtpp_create(uint32_t maxjobs);

/**
 * Destroy module object.
 */
void
ymtpp_destroy(struct ymtpp *);

/**
 * Create mtpp task object.
 */
ymtpp_task_t
ymtpp_create_task(struct ymtpp *);

/**
 * Destroy mtpp task created by {@link ymtpp_create_task}.
 */
void
ymtpp_destroy_task(struct ymtpp *, ymtpp_task_t);

/**
 * Add new job.
 *
 * @param t Task object created by {@link ymtpp_create_task}
 * @return 0 if success. Otherwise errno.
 */
int
ymtpp_add_task(struct ymtpp *, ymtpp_task_t t);

/**
 * Add job dependencies.
 * This means, \a target should be run after \a dependent is completed.
 *
 * @param target node that have dependency on \a dependent
 * @param dependent node blocking running job of \a target
 * @return 0 if success. Otherwise <0(ex. NOMEM)
 */
int
ymtpp_add_dependency(struct ymtpp *,
		     ymtpp_task_t target,
		     ymtpp_task_t dependent);


/**
 * Remove dependency.
 * See {@link ymtpp_add_dependency}
 */
int
ymtpp_remove_dependency(struct ymtpp *,
			ymtpp_task_t target,
			ymtpp_task_t dependent);

/**
 * Run job of \a target.
 * This is blocking function call.
 *
 * @param out
 *     Return value of \a target task's job.
 *     This value should be destroied by using {@link ymtppjob::ofree}
 * @param target
 *     Target task that should be run. Tasks that \a target is depending on,
 *     will be run according to dependency-relation.
 * @return 0 if success. Otherwise <0.
 */
int
ymtpp_run(struct ymtpp *,
	  void **out,
	  ymtpp_task_t target);

#endif /* __YMTPp_h__ */
