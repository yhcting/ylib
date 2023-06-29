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
 * @file threadex.h
 * @brief Header to use ythreadex in inside library
 */

#pragma once

#include "ythreadex.h"
#include "def.h"


#define THREADEX_MAX_NAME 32


struct ythreadex {
	/* -------- READ ONLY values(set only once --------*/
	pthread_t thread;
	long id;
	char name[THREADEX_MAX_NAME];
	struct ymsghandler *owner;
	enum ythreadex_priority priority;
	struct ythreadex_listener listener;
	void (*free_arg)(void *);
	void (*free_result)(void *);
	int (*run)(struct ythreadex *, void **result);

	/* ---------- Dynamically updated values ----------*/
	enum ythreadex_state state;
	pthread_mutex_t state_lock;
	int errcode; /**< error value returned from 'run' */
	void *arg; /**< argument passed to thread */
	void *result; /**< result from thread(run) */
};


/**
 * @return 0 if success. @c -errno if fails.
 */
int
threadex_init(
	struct ythreadex *,
	const char *name,
	struct ymsghandler *owner,
	enum ythreadex_priority priority,
	const struct ythreadex_listener *listener,
	void *arg,
	void (*free_arg)(void *),
	void (*free_result)(void *),
	int (*run)(struct ythreadex *, void **result));

/**
 * @return 0 if success. @c -errno if fails.
 */
int
threadex_clean(struct ythreadex *);
