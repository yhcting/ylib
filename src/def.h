/******************************************************************************
 * Copyright (C) 2015, 2023
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

#pragma once

#include "ydef.h"

#ifndef CONFIG_IGNORE_CONFIG
#include "config.h"
#endif


/*****************************************************************************
 *
 * Map internal symbol in 'ydef.h' to general name to use them easily in ylib.
 *
 *****************************************************************************/
#define containerof YYcontainerof
#define EXPORT YYEXPORT
#define INLINE YYINLINE
#define likely YYlikely
#define unlikely YYunlikely

#undef YYassert
#define YYassert yassert


/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
/* GNU C Specific */
#ifdef __GNUC__

/** Software memory barrier */
#define barrier() __asm__ __volatile__("": : :"memory")
/** Hardware memory barrier */
#define hwbarrier() __sync_synchronize()
#define unused __attribute__((unused))

#else /* __GNUC__ */

#error barrier, hwbarrier should be defined.
#define barrier()
#define hwbarrier()
#define unused

#endif /* __GNUC__ */


/*****************************************************************************
 *
 * Types
 *
 *****************************************************************************/
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
