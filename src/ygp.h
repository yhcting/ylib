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
 * @file ygp.h
 * @brief Header for general smart object pointer. gp module keep tracking
 *	reference count of object with MT-safe way(lock).
 */

#pragma once

#include <pthread.h>

#include "ydef.h"


/**
 * Entire data struct of ygp struct.
 * Container can embed this struct and use ygp interfaces.
 * But DO NOT access internal values directly. Use them via interface!
 */
struct ygp {
	void *container; /**< Container object */
	void (*container_free)(void *); /**< Function to free container */
	int refcnt;  /**< reference count */
};

/**
 * Initialize ygp instance.
 */
YYEXPORT int
ygpinit(struct ygp *gp,
	void *container,
	void (*container_free)(void *));

/**
 * Put object. Reference counter is decreased.
 * If reference count is '0' and @c ofree is NOT NULL, object is freed by
 *  calling @c ofree.
 *
 * @return Reference count after put.
 */
YYEXPORT int
ygpput(struct ygp *);

/**
 * Increased reference count.
 *
 * @return Reference count after get.
 */
YYEXPORT int
ygpget(struct ygp *);


/**
 * Get current reference count.
 * Reference count is NOT changed.
 */
YYEXPORT int
ygpref_cnt(const struct ygp *);

/**
 * Destroy container and ygp instance in force.
 */
YYEXPORT void
ygpdestroy(struct ygp *);


/**
 * Get container of this ygp.
 */
static YYINLINE void *
ygpcontainer(struct ygp *gp) {
	return gp->container;
}
