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
 * @file yline.h
 * @brief This defines line on XY plain.
 *
 * Interfaces have prefix 'ygeol_'
 * Line is consist of two points.
 * In case of line segment, 2nd point is 'open' point.
 */

#ifndef __YLINe_h__
#define __YLINe_h__

#include "ypoint.h"

/**
 * Line in 2D plain.
 */
struct yline {
	union {
		struct {
			struct ypoint p0; /**< included(closed) point */
			struct ypoint p1; /**< excluded(open) point */
		};
		struct {
			struct {
				int x0, y0; /**< alias to point p0 */
			};
			struct {
				int x1, y1; /**< alias to point p1 */
			};
		};
	};
};

/**
 * Set line values
 *
 * @param l Line structure
 * @param x0 X value of point0
 * @param y0 Y value of point0
 * @param x1 X value of point1
 * @param y1 Y value of point1
 */
static YYINLINE void
ygeol_set(struct yline *l,
	  int x0, int y0,
	  int x1, int y1) {
	l->x0 = x0;
	l->y0 = y0;
	l->x1 = x1;
	l->y1 = y1;
}

/**
 * Set line values.
 *
 * @param l Line structure
 * @param p0 Point0
 * @param p1 Point1
 */
static YYINLINE void
ygeol_set2(struct yline *l,
	   const struct ypoint *p0,
	   const struct ypoint *p1) {
	ygeol_set(l, p0->x, p0->y, p1->x, p1->y);
}

/**
 * Compare two lines
 *
 * @param l0 Line
 * @param l1 Line
 * @return TRUE if two lines are equal. Otherwise FALSE.
 */
static YYINLINE bool
ygeol_eq(const struct yline *l0, const struct yline *l1) {
	return ygeop_eq(&l0->p0, &l1->p0)
		&& ygeop_eq(&l0->p1, &l1->p1);
}


#endif /* __YLINe_h__ */
