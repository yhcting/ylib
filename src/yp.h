/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015, 2016, 2023
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
 * @file yp.h
 * @brief Header for simple smart-pointer-like-implementation.
 */

#pragma once

#include "ydef.h"

/*****************************************************************************
 *
 * Simple Smart/Debuggable Pointer.
 *
 *****************************************************************************/

/**
 * Allocate memory having meta-data.
 *
 * @param sz Size in bytes.
 * @return Newly allocated memory. NULL if fails.
 */
YYEXPORT void *
ypmalloc(uint32_t sz);

/**
 * Allocate zero-initialized memory having meta-data.
 *
 * @param sz Size in bytes.
 * @return Newly allocated memory. NULL if fails.
 */
YYEXPORT void *
ypcalloc(uint32_t sz);

/**
 * WARN: Free memory allocated by 'ypmalloc' in force.
 * (reference count is ignored!)
 */
YYEXPORT void
ypfree(void *);

/**
 * Put pointer. Reference counter is decreased.
 * And if reference count is '0', memory is freed automatically.
 */
YYEXPORT void
ypput(void *);

/**
 * Get pointer. Reference counter is increased.
 */
YYEXPORT void
ypget(const void *);
