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
 * @file ymsghandler.h
 * @brief message handler
 */

#ifndef __YMSGHANDLEr_h__
#define __YMSGHANDLEr_h__

#include <pthread.h>
#include "ydef.h"


struct ymsg;
struct ymsghandler;
struct ymsglooper;

/**
 * Create message handler.
 *
 * @param ml message looper in which this handler works, in terms of 'context'.
 * @param tag Custom tag.
 * @param tagfree Function to free {@code tag}
 * @param handle message handle function.
 *        'handle' MUST NOT change value or destroy(free) msg object!
 *        To use default handler, set to NULL.
 * @return NULL if fails.
 */
YYEXPORT struct ymsghandler *
ymsghandler_create(struct ymsglooper *ml,
		   void *tag,
		   void (*tagfree)(void *),
		   void (*handle)(struct ymsghandler *, const struct ymsg *));

/**
 * Destroy handler.
 */
YYEXPORT void
ymsghandler_destroy(struct ymsghandler *);

/**
 * Get tag set at {@link ymsghandler_create}.
 */
YYEXPORT void *
ymsghandler_get_tag(struct ymsghandler *);

/**
 * Get msglooper in where this handler belonging to.
*/
YYEXPORT struct ymsglooper *
ymsghandler_get_looper(struct ymsghandler *);

/**
 * Post data msg to this handler.
 */
YYEXPORT int
ymsghandler_post_data(struct ymsghandler *,
		      int code, void *data,
		      void (*dfree)(void *));

/**
 * Post data msg to this handler.
 */
YYEXPORT int
ymsghandler_post_data2(struct ymsghandler *,
		       int code, void *data,
		       void (*dfree)(void *),
		       uint8_t pri, uint32_t opt);

/**
 * Post exec.(callback) msg to this handler.
 */
YYEXPORT int
ymsghandler_post_exec(struct ymsghandler *,
		      void *arg, void (*argfree)(void *),
		      void (*run)(void *));

/**
 * Post exec.(callback) msg to this handler.
 */
YYEXPORT int
ymsghandler_post_exec2(struct ymsghandler *,
		       void *arg, void (*argfree)(void *),
		       void (*run)(void *),
		       uint8_t pri, uint32_t opt);

/**
 * If msghandler is run on current context, {@code run} is executed
 *   immediately.
 * Otherwise, this function works exactly same with
 *   {@link ymsghandler_post_exec}
 */
YYEXPORT int
ymsghandler_exec_on(struct ymsghandler *mh,
		    void *arg, void (*argfree)(void *),
		    void (*run)(void *));


#endif /* __YMSGHANDLEr_h__ */
