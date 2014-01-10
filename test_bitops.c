/*****************************************************************************
 *    Copyright (C) 2012 Younghyung Cho. <yhcting77@gmail.com>
 *
 *    This file is part of ylib
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License
 *    (<http://www.gnu.org/licenses/lgpl.html>) for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.	If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include "common.h"
#include "ylist.h"
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

