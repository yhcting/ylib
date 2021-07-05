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
 * @file yband.h
 * @brief This defines band on XY plain.
 *
 * Interfaces have prefix 'ygeob_'
 * Band is area between two lines that are parallel to x-y axis.
 */

#ifndef __YBANd_h__
#define __YBANd_h__

#include "../ydef.h"

/*
 * Band position
 * (How can I write comment below preformatted text well for Doxygen?
 *  I can't find good way at this moment.)
 *
 * Case SEPARATE : Two bands doens't overlap.
 *
 * ------------------------
 * ////////////////////////
 * ------------------------
 * ~~~~~~~~~~~~~~~~~~~~~~~~
 * \\\\\\\\\\\\\\\\\\\\\\\\
 * ~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *
 * Case OVERLAP : Overlap. But not subset.
 *
 * ------------------------
 * ////////////////////////
 * ~~~~~~~~~~~~~~~~~~~~~~~~
 * XXXXXXXXXXXXXXXXXXXXXXXX
 * ------------------------
 * \\\\\\\\\\\\\\\\\\\\\\\\
 * ~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *
 * Case SUBSET : Subset.
 *
 * ------------------------
 * ////////////////////////
 * ~~~~~~~~~~~~~~~~~~~~~~~~
 * XXXXXXXXXXXXXXXXXXXXXXXX
 * ~~~~~~~~~~~~~~~~~~~~~~~~
 * ////////////////////////
 * ------------------------
 *
 */

/**
 * Layout between two bands
 */
enum ybandpos {
	YBAND_SEPARATE, /**< Bands are separated.(No overlapped) */
	YBAND_OVERLAP, /**< Partially overlapped. */
	YBAND_SUBSET, /**< One band is subset of the other band. */
};

/**
 * Band in 2D plain.
 * Larger line is 'open', that is, it is 'excluded'.
 */
struct yband {
	int s; /**< Small(closed) value */
	int b; /**< Big value(open) */
};

/**
 * Set band values.
 *
 * @param b Band structure
 * @param sv Smaller value
 * @param bv Bigger value
 */
static YYINLINE void
ygeob_set(struct yband *b, int sv, int bv) {
	YYassert(sv <= bv);
	b->s = sv;
	b->b = bv;
}

/**
 * Make band be empty.
 *
 * @param b Band structure
 */
static YYINLINE void
ygeob_set_empty(struct yband *b) {
	b->s = b->b = 0;
}

/**
 * Is band empty?
 *
 * @param b Band structure
 * @return Boolean
 */
static YYINLINE bool
ygeob_is_empty(const struct yband *b) {
	return b->s >= b->b;
}


/**
 * Compare two bands
 *
 * @param b0 Band structure
 * @param b1 Band structure
 * @return TRUE if those are equal. Otherwise FALSE.
 */
static YYINLINE bool
ygeob_eq(const struct yband *b0, const struct yband *b1) {
	return b0->s == b1->s
		&& b0->b == b1->b;
}

/**
 * It analyzes two bands and gives non-overlapping sub-bands.
 *
 * @param obs (Out) Non-overlapping sub-band at small-value-side.
 * @param obo (Out) Overlapping sub-band in the middle.
 * @param obb (Out) Non-overlapping sub-band at big-value-side.
 * @param bs Band whose {@code s} value is smaller than {@code bb}.
 * @param bb Band whose {@code s} value is bigger than {@code bs}.
 * @return Band position. See. @ref ybandpos
 */
static YYINLINE enum ybandpos
ygeob_analyze_layout(
	struct yband *obs, /* Out Band Small-side */
	struct yband *obo, /* Out Band Overlapping */
	struct yband *obb, /* Out Band Big-side */
	const struct yband *bs, /* Band 's' value is smaller */
	const struct yband *bb /* Band 's' value is bigger */
) {
	YYassert(bs->s <= bb->s);
	if (bs->b <= bb->s) {
		*obs = *bs;
		ygeob_set_empty(obo);
		*obb = *bb;
		return YBAND_SEPARATE;
	} else if (bs->b >= bb->b) {
		ygeob_set(obs, bs->s, bb->s);
		*obo = *bb;
		ygeob_set(obb, bb->b, bs->b);
		return YBAND_SUBSET;
	} else {
		ygeob_set(obs, bs->s, bb->s);
		ygeob_set(obo, bb->s, bs->b);
		ygeob_set(obb, bs->b, bb->b);
		return YBAND_OVERLAP;
	}
}

#endif /* __YBANd_h__ */
