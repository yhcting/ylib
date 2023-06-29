/******************************************************************************
 * Copyright (C) 2015, 2023
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
 * @file yrgn.h
 * @brief This defines arbitrary set of rectangles on XY plain.
 *
 * Region is set of non-overlapping retangles.
 * Most important point here is 'non-overlapping'.
 */

#pragma once

/* Including heads to use macros */
#include "yband.h"
#include "yrect.h"

struct yrgn;

/**
 * Create empty region.
 *
 * @return NULL if fails(ex ENOMEM). Otherwise result region.
 */
YYEXPORT struct yrgn *
ygeog_create_empty(void);

/**
 * Clone region.
 *
 * @return NULL if fails(ex ENOMEM). Otherwise result region.
 */
YYEXPORT struct yrgn *
ygeog_clone(const struct yrgn *);

/**
 * Destroy region.
 */
YYEXPORT void
ygeog_destroy(struct yrgn *);

/**
 * Is region empty?
 *
 * @return Boolean
 */
YYEXPORT bool
ygeog_is_empty(const struct yrgn *);

/**
 * Get intersection region of two regions.
 *
 * @return NULL if fails(ex ENOMEM). Otherwise result region.
 */
YYEXPORT struct yrgn *
ygeog_intersect(const struct yrgn *, const struct yrgn *);

/**
 * See @ref ygeog_intersect
 *
 * @return See @ref ygeog_intersect
 */
YYEXPORT struct yrgn *
ygeog_intersect2(const struct yrgn *, const struct yrect *);

/**
 * Get diff region of two regions.
 *
 * @return NULL if fails(ex ENOMEM). Otherwise result region.
 */
YYEXPORT struct yrgn *
ygeog_diff(const struct yrgn *, const struct yrgn *);

/**
 * See @ref ygeog_diff
 *
 * @return See @ref ygeog_diff
 */
YYEXPORT struct yrgn *
ygeog_diff2(const struct yrgn *, const struct yrect *);

/**
 * See @ref ygeog_diff
 *
 * @return See @ref ygeog_diff
 */
YYEXPORT struct yrgn *
ygeog_diff3(const struct yrect *, const struct yrect *);

/**
 * Get union region of two regions.
 *
 * @return NULL if fails(ex ENOMEM). Otherwise result region.
 */
YYEXPORT struct yrgn *
ygeog_union(const struct yrgn *, const struct yrgn *);

/**
 * See @ref ygeog_union
 *
 * @return See @ref ygeog_union
 */
YYEXPORT struct yrgn *
ygeog_union2(const struct yrgn *, const struct yrect *);

/**
 * See @ref ygeog_union
 *
 * @return See @ref ygeog_union
 */
YYEXPORT struct yrgn *
ygeog_union3(const struct yrect *, const struct yrect *);

/**
 * This macros access internal data structure of region.
 * (It means, it breaks capsulation!)
 * But it is for high performance and usability.
 * Therefore user DO NOT assume anything about internal data struture.
 *
 * @param g (struct yrgn *)
 * @param r (struct yrect)
 */
#define ygeog_foreach_rect_begin(g, r)					\
	/* use very unique name to minimize conflict against user-code */ \
	do {								\
		const struct yband *b_____Z = (struct yband *)(g);	\
		/* y-band loop */					\
		while (!ygeob_is_empty(b_____Z)) {			\
			(r).yb = *b_____Z++; /* move to x-bands */	\
			/* x-band loop */				\
			while (!ygeob_is_empty(b_____Z)) {		\
				/* re-using yband and updating xband */	\
				(r).xb = *b_____Z++;

/**
 * Pair with @ref ygeog_foreach_rect_begin.
 */
#define ygeog_foreach_rect_end()		   		\
			} /* end of x-band loop */ 		\
			b_____Z++; /* move to next rect-band */	\
		} /* end of y-band loop */ 			\
	} while (0)
