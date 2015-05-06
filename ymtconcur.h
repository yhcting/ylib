/******************************************************************************
 * Copyright (C) 2014
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
 *     * Additional argument set at 'struct ymtconcurjob.arg'
 *
 *   Output:
 *     return value of job thread.
 *
 *   Clean up at the end of thread:
 *     Clean argument by calling 'free_arg' callback function.
 *     Clean return value of [job A] and [job B] by calling 'free_ret' callback
 *	 function.
 *****************************************************************************/

#ifndef __YMTCONCUr_h__
#define __YMTCONCUr_h__

#define YMTCONCUR_JOB_NAME_LEN 63

struct ymtconcur;
struct yvertex;
typedef struct yvertex * ymtconcurjob_node_t;

struct ymtconcurjobarg {
	/* indicating job name. - See ymtconcurjob. */
	const char *name;
	const void *data; /* data returned from the job */
};

struct ymtconcurjob {
	char	name[YMTCONCUR_JOB_NAME_LEN + 1];
	/*
	 * @out output value from function.
	 * return
	 *     0 : success
	 *    <0 : error
	 */
	int    (*run_func)(void **out,
			   const void *arg,
			   /* Array of arguments which ends with
			    *	empty data - '{0, 0}'.
			    * This is value passed from previous jobs.
			    */
			   const struct ymtconcurjobarg *jargs);
	void   *arg; /* argument passed in addition to previous return data */
	void  (*free_arg)(void *);
	void  (*free_out)(void *);
};

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static inline int
ymtconcur_is_empty_jobarg(const struct ymtconcurjobarg *a) {
	return !a->name && !a->data;
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/

/**
 * ymtconcur is created and initialized.
 * @maxjobs <=0 means 'infinite'
 * return
 *     NULL if fails. Otherwise success.
 */
struct ymtconcur *
ymtconcur_create(u32 maxjobs);

/**
 * return
 *     0: success
 *    <0: fails (ex. NOMEM)
 */
int
ymtconcur_init(struct ymtconcur *, u32 maxjobs);

/**
 * ymtconcur itself isn't destroied.
 */
void
ymtconcur_clean(struct ymtconcur *);

/**
 * ymtconcur is cleaned and destroied.
 */
void
ymtconcur_destroy(struct ymtconcur *);

/**
 * @v
 *    base vertex. New job is added to the next of this vertex.
 *    Return value of 'add_job' function - like this function - can be used
 *	as this vertex value.
 * @job
 *    job to add. Deep-copied value is used.
 *    So, memory indicated by the pointer can be free safely.
 * return
 *     vertex that corresponds with newly added job.
 */
ymtconcurjob_node_t
ymtconcur_add_job(struct ymtconcur *,
		  const struct ymtconcurjob *job);

/**
 * set(replace) existing job.
 *
 * return
 *     0: success
 *    <0: fails (ex. NOMEM)
 */
int
ymtconcur_jobnode_set_job(ymtconcurjob_node_t jobnode,
			  const struct ymtconcurjob *job);

/**
 * return
 *     0: success
 *    <0: fails (ex. NOMEM)
 */
int
ymtconcur_add_dependency(struct ymtconcur *,
			 ymtconcurjob_node_t target,
			 ymtconcurjob_node_t dependent);


/**
 * return
 *     0: success
 *    <0: fails (ex. NOMEM)
 */
int
ymtconcur_remove_dependency(struct ymtconcur *,
			    ymtconcurjob_node_t target,
			    ymtconcurjob_node_t dependent);

/**
 * This is blocking function call.
 * return
 *	0: success
 *     <0: fails
 * @out return value of target vertex's job
 *	See, comments of 'ymtconcur_add_end_job' for additional information.
 * @targetv target vertex. Doing this target vertex's job and all it's
 *	      dependent jobs.
 */
int
ymtconcur_run(struct ymtconcur *,
	      void **out,
	      ymtconcurjob_node_t target);

#endif /* __YMTCONCUr_h__ */
