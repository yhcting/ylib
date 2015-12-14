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
 * @file ypoint.h
 * @brief This defines point on XY plain.
 *
 * Interfaces have prefix 'ygeop_'
 * Point is represented as (x, y).
 */

#ifndef __YPOINt_h__
#define __YPOINt_h__

#include "../ydef.h"

/**
 * Point in 2D plain.
 */
struct ypoint {
	int x; /**< X value */
	int y; /**< Y value */
};

/**
 * Set point values
 *
 * @param p Point structure
 * @param x X value
 * @param y Y value
 */
static YYINLINE void
ygeop_set(struct ypoint *p, int x, int y) {
	p->x = x;
	p->y = y;
}

/**
 * Compare two points
 *
 * @param p0 Point
 * @param p1 Point
 * @return TRUE if two points are equal. Otherwise FALSE.
 */
static YYINLINE bool
ygeop_eq(const struct ypoint *p0, const struct ypoint *p1) {
	return p0->x == p1->x
		&& p0->y == p1->y;
}


#endif /* __YPOINt_h__ */
