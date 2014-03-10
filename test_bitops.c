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


#include "ycommon.h"
#include "test.h"

#include <assert.h>

/**
 * Linked list test.
 */
static void
test_bitops(void) {
	unsigned long x __attribute__((unused));
	x = 0xabcffcda;
	yassert(0xc == bits(x, 8, 4));
	yassert(0xa == bits(x, 0, 4));
	yassert(0xa == bits(x, 28, 4));
	yassert(0xabcf0cda == clear_bits(x, 12, 4));
	yassert(0xabcffcd0 == clear_bits(x, 0, 4));
	yassert(0xabcffcd1 == set_bits(x, 0, 4, 1));
	yassert(0xabcf1cda == set_bits(x, 12, 4, 1));
	yassert(test_bit(x, 1));
	yassert(test_bit(x, 3));
	yassert(0xabcffcdb == set_bit(x, 0));
	yassert(0xabcffcda == set_bit(x, 13));
	yassert(0xabcfecda == clear_bit(x, 12));
	yassert(0xabcffcda == clear_bit(x, 26));
}

TESTFN(test_bitops, bitopts)

