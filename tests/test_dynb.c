/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014
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

#include <string.h>
#include <assert.h>

#include "common.h"
#include "ydynb.h"
#include "test.h"


static void
test_dynb(void) {
	struct ydynb *b = ydynb_create(2);
	yassert(2 == ydynb_freesz(b));
	ydynb_append(b, "ab ", 3);
	yassert(1 == ydynb_freesz(b));
	ydynb_append(b, "cde fgh ", 8);
	/* include trailing 0 */
	ydynb_append(b, "1234567890123456789012345678901234567890", 41);
	yassert(52 == b->sz);
	yassert(!strcmp("ab cde fgh 1234567890123456789012345678901234567890",
			(const char *)b->b));
	yassert(-EINVAL == ydynb_shrink(b, 0));
	yassert(-EINVAL == ydynb_shrink(b, 0xffff));
	yassert(-EINVAL == ydynb_shrink(b, b->sz - 1));
	ydynb_shrink(b, b->sz);
	yassert(0 == ydynb_freesz(b));
	ydynb_destroy(b);
}


TESTFN(test_dynb, dynb)