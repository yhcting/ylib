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
 * @file yrect.h
 * @brief This defines rectangle on XY plain.
 *
 * Rectangle is repesented as 'intersection of two bands'
 * or 4 lines (left, top, right, bottom)
 */

#ifndef __YRECt_h__
#define __YRECt_h__

#include "yband.h"
#include "ypoint.h"

/**
 * Macro helps to make initialization data of yrect structure.
 *
 * @param t Top
 * @param b Bottom
 * @param l Left
 * @param r Right
 */
#define yrect_struct_init(t, b, l, r) { { { { t, b }, { l, r } } } }

/**
 * Rectangle in 2D plain.
 */
struct yrect {
	union {
		struct {
			/* DO NOT change order!
			 * Order is important! 'yb' first and 'xb' next
			 */
			struct yband yb; /**< y-coord band */
			struct yband xb; /**< x-coord band */
		};
		struct {
 			struct {
				int t, b; /**< alias to band yb */
			};
			struct {
				int l, r; /**< alias to band xb */
			};
		};
	};
};

/**
 * Set rectangle value.
 *
 * @param or (Out) Rectangle
 * @param l Left
 * @param t Top
 * @param r Right. 'open' value(excluded)
 * @param b Bottom. 'open. value(excluded)
 */
static YYINLINE void
ygeor_set(struct yrect *or, int l, int t, int r, int b) {
	YYassert(l <= r && t <= b);
	or->l = l;
	or->t = t;
	or->r = r;
	or->b = b;
}

/**
 * Set rectangle value.
 *
 * @param r (Out) Rectangle.
 * @param xb x-band.
 * @param yb y-band.
 */
static YYINLINE void
ygeor_set2(struct yrect *r,
	const struct yband *xb, const struct yband *yb
) {
	r->xb = *xb;
	r->yb = *yb;
}

/**
 * Set rectangle value.
 * There is no required condition between x0/y0 and x1/y1.
 *
 * @param r (Out) Rectangle
 * @param x0 X-value
 * @param x1 X-value
 * @param y0 Y-value
 * @param y1 Y-value
 */
YYEXPORT void
ygeor_set3(struct yrect *r, int x0, int x1, int y0, int y1);


/**
 * Make rect be empty.
 *
 * @param r Rectangle
 */
static YYINLINE void
ygeor_set_empty(struct yrect *r) {
	/* Setting only xband is enough */
	ygeob_set_empty(&r->xb);
}

/**
 * Is rectangle empty?
 *
 * @param r Rectangle.
 * @return Boolean
 */
static YYINLINE bool
ygeor_is_empty(const struct yrect *r) {
	return ygeob_is_empty(&r->xb)
		|| ygeob_is_empty(&r->yb);
}

/**
 * Compare two rectangles
 *
 * @param r0 Rectangle
 * @param r1 Rectangle
 * @return TRUE if those are equal. Otherwise FALSE.
 */
static YYINLINE bool
ygeor_eq(const struct yrect *r0, const struct yrect *r1) {
	return ygeob_eq(&r0->xb, &r1->xb)
		&& ygeob_eq(&r0->yb, &r1->yb);
}

/**
 * Is rectangle contains given point?
 *
 * @param r Rectangle
 * @param p Point
 * @return Boolean.
 */
static YYINLINE bool
ygeor_contain(const struct yrect *r, const struct ypoint *p) {
	return r->l <= p->x
		&& r->t <= p->y
		&& r->r > p->x
		&& r->b > p->y;
}


/**
 * Get intersection.
 *
 * @param r (Out) Intersection rect
 */
YYEXPORT void
ygeor_intersect(struct yrect *r, const struct yrect *, const struct yrect *);

#endif /* __YRECt_h__ */
