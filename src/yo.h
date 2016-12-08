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
 * @file yo.h
 * @brief Header for simple object knowing how to free itself.
 */

#ifndef __Yo_h__
#define __Yo_h__

#include "ydef.h"

/**
 * To easy handle simple multiple value object.
 */
struct yo {
        void *o0; /**< user object0 */
        void *o1; /**< user object1 */
        void *o2; /**< user object2 */
        void *o3; /**< user object3 */
};

/**
 * Extended version of {@link yocreate2}
 */
YYEXPORT struct yo *
yocreate3(void *o0, void (*ofree0)(void *),
	  void *o1, void (*ofree1)(void *),
	  void *o2, void (*ofree2)(void *),
	  void *o3, void (*ofree3)(void *));

/**
 * Extended version of {@link yocreate1}
 */
static inline struct yo *
yocreate2(void *o0, void (*ofree0)(void *),
	  void *o1, void (*ofree1)(void *),
	  void *o2, void (*ofree2)(void *)) {
	return yocreate3(o0, ofree0,
			 o1, ofree1,
			 o2, ofree2,
			 NULL, NULL);
}

/**
 * Extended version of {@link yocreate0}
 */
static inline struct yo *
yocreate1(void *o0, void (*ofree0)(void *),
	  void *o1, void (*ofree1)(void *)) {
	return yocreate2(o0, ofree0,
			 o1, ofree1,
			 NULL, NULL);
}

/**
 * @param o0 user object that can be freed by \a ofree0
 * @param ofree0 Function being used to free \a o at {@link yodestroy}
 * @return yo instance. NULL if fails.
 */
static inline struct yo *
yocreate0(void *o0, void (*ofree0)(void *)) {
	return yocreate1(o0, ofree0,
			 NULL, NULL);
}

/**
 * Object is destroied by using \a ofree obtained at {@link yocreate0}
 */
YYEXPORT void
yodestroy(struct yo *);

#endif /*__Yo_h__ */
