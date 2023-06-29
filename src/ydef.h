/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015, 2023
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
 * @file ydef.h
 * @brief File defines basic types and macros.
 *
 * Most files are including this file.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

/*****************************************************************************
 *
 * YLIB specific(internal) type and macros (It's NOT exported interface!)
 * These are widely used. But there are no defacto-standard-definition.
 * In ylib, 'YY' prefix is used.
 * And some of them will be redefined at internal-headers.
 *
 * The reason why these symbols are needed, is to support static(usually
 * YYINLINE) functions in API header file.
 *
 *****************************************************************************/

/* >>> @cond */

#define YYcontainerof(ptr, type, member)			\
	((type *)(((char *)(ptr)) - offsetof(type, member)))

#define YYassert(x) do { } while (0)

/* GNU C Specific */
#ifdef __GNUC__

/*
 * Visibility(especially default visibility) is different among compilers.
 */
#define YYEXPORT __attribute__ ((visibility ("default")))
/*
 * 'YYINLINE' directive is introduced at C99.
 * It's NOT original ANSI C standard(C89).
 */
#define YYINLINE inline
#define YYlikely(x) __builtin_expect(!!(x), 1)
#define YYunlikely(x) __builtin_expect(!!(x), 0)

#else /* __GNUC__ */

#define YYEXPORT /* default visibility */
#define YYINLINE
#define YYlikely(x) x
#define YYunlikely(x) x

#endif /* __GNUC__ */

/* <<< @endcond */


/*****************************************************************************
 *
 * Basic macros and types
 * These symbols have defacto-standard-definition.
 * So, definition of these symbols are inside 'ifndef/endif' section.
 * That means, even if these symbols are defined outside ylib, we believe
 * definition are same sementically.
 *
 *****************************************************************************/
#ifndef NULL
/** NULL pointer */
#define NULL ((void *)0)
#endif

#ifndef TRUE
/** Boolean TRUE */
#define TRUE (0 == 0)
#endif

#ifndef FALSE
/** Boolean FALSE */
#define FALSE (0 != 0)
#endif

#ifndef bool
/** bool type */
#define bool char
#endif

/* offsetof is included at C standard since C99 */
#ifndef offsetof
/** offset of macro */
#define offsetof(type, member) ((size_t) &((type *)0)->member)
#endif
