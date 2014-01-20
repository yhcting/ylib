/******************************************************************************
 *    Copyright (C) 2011, 2012, 2013, 2014
 *    Younghyung Cho. <yhcting77@gmail.com>
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


#ifndef __COMMOn_h__
#define __COMMOn_h__

#include "ydef.h"

/*****************************************************************************
 *
 * General Macros
 *
 *****************************************************************************/
#ifndef abs
#	define abs(x) (((x)>0)?(x):-(x))
#endif

#ifndef swap
#	define swap(x,y,tmp) do { (tmp)=(x);(x)=(y);(y)=(tmp); } while (0)
#endif

#ifndef min
#	define min(x,y) (((x)<(y))?x:y)
#endif

#ifndef max
#	define max(x,y) (((x)<(y))?y:x)
#endif

/*****************************************************************************
 *
 * Bit operations
 *     - Not fully tested yet.
 *
 *****************************************************************************/
#define bits(x, offset, bitsz) \
	(((x) >> (offset)) & ((1LL << (bitsz)) - 1))

#define bit(x, offset) bits(x, offset, 1)

#define clear_bits(x, offset, bitsz) \
	((x) & ~(((1LL << (bitsz)) - 1) << (offset)))
/*
 * replace bits from 'offset' to 'offset' + 'bitsz' of 'x' with bits
 *   from 0 to 'bitsz' of 'v'.
 */
#define set_bits(x, offset, bitsz, v) \
	(clear_bits(x, offset, bitsz)			\
	 | ((((1LL << (bitsz)) - 1) & (v)) << (offset)))

#define test_bit(x, bit) \
	(((x) >> (bit)) & 1LL)

#define set_bit(x, bit) \
	((x) | (1LL << (bit)))

#define clear_bit(x, bit) \
	((x) & ~(1LL << (bit)))


/*****************************************************************************
 *
 * Others
 *
 *****************************************************************************/

#define unroll16( expr, count, cond)	\
	switch( (count) & 0xf ) {	\
        case 0: while (cond){		\
			expr;		\
	case 15: expr;			\
	case 14: expr;			\
	case 13: expr;			\
	case 12: expr;			\
	case 11: expr;			\
	case 10: expr;			\
	case 9: expr;			\
	case 8: expr;			\
	case 7: expr;			\
	case 6: expr;			\
	case 5: expr;			\
	case 4: expr;			\
	case 3: expr;			\
	case 2: expr;			\
	case 1: expr;			\
	}				\
}

#define arrsz(a) ((int)(sizeof(a)/sizeof((a)[0])))

#define sec2ns(x) = ((x) * 1000 * 1000 * 1000)

#endif /* __COMMOn_h__ */
