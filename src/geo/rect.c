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

#include "geo.h"
#include "yrect.h"
#include "../yut.h"

void
ygeor_set3(
	struct yrect *or,
	int x0, int x1,
	int y0, int y1
) {
	int l, t, r, b;
	yut_cmpst(l, r, x0, x1,);
	yut_cmpst(t, b, y0, y1,);
	ygeor_set(or, l, t, r, b);
}

void
ygeor_intersect(
	struct yrect *or,
	const struct yrect *r0,
	const struct yrect *r1
) {
	const struct yrect *pr0, *pr1;
	int l, t, r, b;
	yut_cmpst(pr0, pr1, r0, r1, ->l);
	if (pr0->r <= pr1->l)
		goto no_intersect;
	l = pr1->l;

	yut_cmpst(pr0, pr1, r0, r1, ->r);
	if (pr0->r <= pr1->l)
		goto no_intersect;
	r = pr0->r;

	yut_cmpst(pr0, pr1, r0, r1, ->t);
	if (pr0->b <= pr1->t)
		goto no_intersect;
	t = pr1->t;

	yut_cmpst(pr0, pr1, r0, r1, ->b);
	if (pr0->b <= pr1->t)
		goto no_intersect;
	b = pr0->b;
	ygeor_set(or, l, t, r, b);

	return;

 no_intersect:
	ygeor_set_empty(or);
}
