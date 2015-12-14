/******************************************************************************
 * Copyright (C) 2015
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
 * @file yutils.h
 * @brief Header defines small utilities.
 */

#ifndef __YUTILs_h__
#define __YUTILs_h__

/*****************************************************************************
 *
 * Bit operations
 *
 *****************************************************************************/
/**
 * Get \a bitsz bits at \a offset from LSB.
 * Ex. 0xc == %yut_bits(0xfcda, 8, 4)
 *
 * @param x Value
 * @param offset Offset from LSB
 * @param bitsz Size of bits to get.
 */
#define yut_bits(x, offset, bitsz)			\
	(((x) >> (offset)) & ((1LL << (bitsz)) - 1))

/**
 * Get one bit at \a offset from LSB.
 * See {@link yut_bits}.
 *
 * @param x Value
 * @param offset Offset from LSB.
 */
#define yut_bit(x, offset)			\
	yut_bits(x, offset, 1)

/**
 * Clear \a bitsz bits at \a offset from LSB.
 * See {@link yut_bits}.
 */
#define yut_clear_bits(x, offset, bitsz)		\
	((x) & ~(((1LL << (bitsz)) - 1) << (offset)))

/**
 * Replace \a bitsz bits at \a offset with \a bitsz bits at 0 of x.
 *
 * @param x Value
 * @param offset Offset from LSB
 * @param bitsz Size of bits to get
 * @param v New value to be replaced with
 */
#define yut_set_bits(x, offset, bitsz, v)			\
	(yut_clear_bits(x, offset, bitsz)			\
	 | ((((1LL << (bitsz)) - 1) & (v)) << (offset)))

/**
 * Test whether bit at \a bit is set.
 *
 * @param x Value
 * @param bit offset from LSB.
 */
#define yut_test_bit(x, bit)			\
	(((x) >> (bit)) & 1LL)

/**
 * Set bit at offset \a bit
 *
 * @param x Value
 * @param bit offset from LSB.
 */
#define yut_set_bit(x, bit)			\
	((x) | (1LL << (bit)))

/**
 * Clear bit at offset \a bit
 *
 * @param x Value
 * @param bit offset from LSB.
 */
#define yut_clear_bit(x, bit)			\
	((x) & ~(1LL << (bit)))



/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
/**
 * Compare two data.
 *
 * @param small (Out) Smaller value between \a d0 and \a d1
 * @param big (Out) Bigger value between \a d0 and \a d1
 * @param d0 (In)
 * @param d1 (In)
 * @param suffix Suffix of data.
 */
#define yut_cmpst(small, big, d0, d1, suffix)		\
	do {						\
		if (((d0)suffix) < ((d1)suffix)) {	\
			(small) = (d0);			\
			(big) = (d1);			\
		} else {				\
			(small) = (d1);			\
			(big) = (d0);			\
		}					\
	} while (0)

/**
 * Compare two data.
 *
 * @param d0small (Out) (bool) small == d0
 * @param small (Out) Smaller value between \a d0 and \a d1
 * @param big (Out) Bigger value between \a d0 and \a d1
 * @param d0 (In)
 * @param d1 (In)
 * @param suffix Suffix of data.
 */
#define yut_cmpst2(d0small, small, big, d0, d1, suffix)		\
	do {							\
		if (((d0)suffix) < ((d1)suffix)) {		\
			d0small = TRUE;				\
			(small) = (d0);				\
			(big) = (d1);				\
		} else {					\
			d0small = FALSE;			\
			(small) = (d1);				\
			(big) = (d0);				\
		}						\
	} while (0)


/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
/**
 * Unroll statment to improve performance.
 *
 * @param count Number of count repeated.
 * @param stmt Statment run in loop.
 */
#define yut_unroll16(count, stmt)				\
	/* Just in case, minimize referencing 'count' argument. \
	 * Reason: 'count' argument may update some values.	\
	 */							\
	do {							\
		int ___n___;					\
		int ___c___ = (count);				\
		if (unlikely(___c___ < 0)) {			\
			___c___ = 0;				\
		}						\
		___n___ = ___c___ / 16;				\
		switch (___c___ & 0xf) {			\
		case 0: while ( ___n___--) {			\
				stmt;				\
			case 15: stmt;				\
			case 14: stmt;				\
			case 13: stmt;				\
			case 12: stmt;				\
			case 11: stmt;				\
			case 10: stmt;				\
			case 9:	 stmt;				\
			case 8:	 stmt;				\
			case 7:	 stmt;				\
			case 6:	 stmt;				\
			case 5:	 stmt;				\
			case 4:	 stmt;				\
			case 3:	 stmt;				\
			case 2:	 stmt;				\
			case 1:	 stmt;				\
			}					\
		}						\
	} while (0)

/**
 * Get absolute value
 *
 * @param x Value
 */
#define yut_abs(x)				\
	(((x)>0)?(x):-(x))

/**
 * Swap two values
 *
 * @param x Value
 * @param y Value
 * @param tmp Temporal value.
 */
#define yut_swap(x,y,tmp)				\
	do { (tmp)=(x);(x)=(y);(y)=(tmp); } while (0)

/**
 * Get min value.
 *
 * @param x Value
 * @param y Value
 */
#define yut_min(x,y)				\
	(((x)<(y))?x:y)

/**
 * Get max value.
 *
 * @param x Value
 * @param y Value
 */
#define yut_max(x,y)				\
	(((x)<(y))?y:x)

/**
 * Get array size(# of elements in the array).
 *
 * @param a Array
 */
#define yut_arrsz(a)				\
	((int)(sizeof(a)/sizeof((a)[0])))

#endif /* __YUTILs_h__ */
