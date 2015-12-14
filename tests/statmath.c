/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015
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
#include "test.h"
#ifdef CONFIG_DEBUG

#include <string.h>
#include <assert.h>

#include "common.h"
#include "yutils.h"
#include "ystatmath.h"

static void
test_imean(void) {
	int i;
	double vs0[] = {1};
	double vs1[] = {1, 2, 3, 4, 5, 6, 7};
	double vs2[] = {1, 2, 3, 4, 5, 6, 7, 8};
	struct ysm_imean *im, *im0, *im1, *im2;
	im = ysm_imean_create();
	for (i = 0; i < yut_arrsz(vs0); i++)
		ysm_imean_add(im, vs0[i]);
	yassert(1 == ysm_imean(im));
	ysm_imean_destroy(im);

	im = ysm_imean_create();
	for (i = 0; i < yut_arrsz(vs1); i++)
		ysm_imean_add(im, vs1[i]);
	yassert(4 == ysm_imean(im));
	ysm_imean_destroy(im);

	im0 = ysm_imean_create();
	im1 = ysm_imean_create();
	im2 = ysm_imean_create();
	im = ysm_imean_create();
	for (i = 0; i < yut_arrsz(vs2); i++)
		ysm_imean_add(im, vs2[i]);
	yassert(4.5f == ysm_imean(im));
	for (i = 0; i < yut_arrsz(vs2) / 2; i++)
		ysm_imean_add(im0, vs2[i]);
	for (; i < yut_arrsz(vs2); i++)
		ysm_imean_add(im1, vs2[i]);
	ysm_imean_combine(im2, im0, im1);
	yassert(ysm_imean(im) == ysm_imean(im2));
	ysm_imean_destroy(im);
	ysm_imean_destroy(im0);
	ysm_imean_destroy(im1);
	ysm_imean_destroy(im2);
}

static void
test_ivar(void) {
	int i;
	double vs0[] = {1};
	double vs1[] = {1, 2, 3, 4, 5, 6, 7};
	double vs2[] = {1, 2, 3, 4, 5, 6, 7, 8};
	struct ysm_ivar *iv, *iv0, *iv1, *iv2;
	iv = ysm_ivar_create();
	for (i = 0; i < yut_arrsz(vs0); i++)
		ysm_ivar_add(iv, vs0[i]);
	yassert(0 == ysm_ivar(iv));
	ysm_ivar_destroy(iv);

	iv = ysm_ivar_create();
	for (i = 0; i < yut_arrsz(vs1); i++)
		ysm_ivar_add(iv, vs1[i]);
	yassert(4 == ysm_ivar(iv));
	ysm_ivar_destroy(iv);

	iv = ysm_ivar_create();
	iv0 = ysm_ivar_create();
	iv1 = ysm_ivar_create();
	iv2 = ysm_ivar_create();
	for (i = 0; i < yut_arrsz(vs2); i++)
		ysm_ivar_add(iv, vs2[i]);
	yassert(5.25f == ysm_ivar(iv));
	for (i = 0; i < yut_arrsz(vs2) / 2; i++)
		ysm_ivar_add(iv0, vs2[i]);
	for (; i < yut_arrsz(vs2); i++)
		ysm_ivar_add(iv1, vs2[i]);
	ysm_ivar_combine(iv2, iv0, iv1);
	yassert(ysm_ivar(iv) == ysm_ivar(iv2));

	ysm_ivar_destroy(iv);
	ysm_ivar_destroy(iv0);
	ysm_ivar_destroy(iv1);
	ysm_ivar_destroy(iv2);

	iv = ysm_ivar_create();
	yassert(isnan(ysm_ivar(iv)));
	ysm_ivar_destroy(iv);
}


static void
test_statmath(void) {
	test_imean();
	test_ivar();
}

TESTFN(test_statmath, statmath)

#endif /* CONFIG_DEBUG */
