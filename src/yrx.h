/******************************************************************************
 * Copyright (C) 2023
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
 * @file yrx.h
 * @brief Inspired from "ReactiveX". Pipe operations are not supported yet.
 * At this moment, this is just simple listener structure.
 *
 */

#pragma once

#include "ydef.h"

struct yrx;
/** Subscription */
struct yrxsb;

YYEXPORT struct yrx *
yrx_create(void);


YYEXPORT int
yrx_next(struct yrx *, const void *);


YYEXPORT int
yrx_error(struct yrx *, const void *);


/**
 * Call complete callback of subscribers and then destroy all resources.
 * struct @ref yrx and all subscribed struct @ref yrxrb are not available
 * anymore. That is, it's not required(nor possible) unsubscribing
 * struct @ref yrxrb.
 *
 * @return 0 if success. Otherwise -errno.
 */
YYEXPORT int
yrx_complete(struct yrx *);


/**
 * All paramters are NULLable except for struct yrx *
 */
YYEXPORT struct yrxsb *
yrx_subscribe(
	struct yrx *,
	void *ctx,
	void (*freectx)(void *),
	void (*next)(void *ctx, const void *v),
	void (*error)(void *ctx, const void *v),
	void (*complete)(void *ctx));

/**
 * Unsubscribe it. struct @ref yrxsub is not available anymore.
 *
 * @return 0 if success. Otherwise -errno.
 */
YYEXPORT int
yrx_unsubscribe(struct yrxsb *);
