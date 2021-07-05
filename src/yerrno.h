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
 * @file yerrno.h
 * @brief ylib specific error number is defined.
 */

#ifndef __YERRNo_h__
#define __YERRNo_h__

/* yerrno SHOULD be SUPER-SET of standard errno */
#include <errno.h>

#include "ydef.h"

/**
 * ylib specific errno.
 *
 * Note that errno MUST NOT be overlapped with standard or glibc errno.
 * And, it SHOULD NOT be too large not to be considered as valid memory address
 * value, because sometimes, errno may be used as return value of function
 * having pointer as it's return type.
 */
enum {
	YERRNO_BASE = 0x7ff,
	YEILLST = YERRNO_BASE, /**< Illegal state */
	YERRNO_LIMIT
};

/** check errno is yerrno specific or standard one */
#define yerrno_is_yerr(e) (YERRNO_BASE <= (e) && YERRNO_LIMIT > (e))

/**
 * Return string describing errno.
 * In case of @c ec is standard errno, this works same as @c strerror
 */
YYEXPORT const char *
yerrno_str(int ec);

#endif /* __YERRNo_h__ */
