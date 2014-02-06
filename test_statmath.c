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

#include <string.h>
#include <assert.h>

#include "ycommon.h"
#include "ystatmath.h"
#include "test.h"

static void
test_imean(void) {
	int i;
	double vs0[] = {1};
	double vs1[] = {1, 2, 3, 4, 5, 6, 7};
	double vs2[] = {1, 2, 3, 4, 5, 6, 7, 8};
	struct ysm_imean *im, *im0, *im1, *im2;
	im = ysm_imean_create();
	for (i = 0; i < arrsz(vs0); i++)
		ysm_imean_add(im, vs0[i]);
	yassert(1 == ysm_imean(im));
	ysm_imean_destroy(im);

	im = ysm_imean_create();
	for (i = 0; i < arrsz(vs1); i++)
		ysm_imean_add(im, vs1[i]);
	yassert(4 == ysm_imean(im));
	ysm_imean_destroy(im);

	im0 = ysm_imean_create();
	im1 = ysm_imean_create();
	im2 = ysm_imean_create();
	im = ysm_imean_create();
	for (i = 0; i < arrsz(vs2); i++)
		ysm_imean_add(im, vs2[i]);
	yassert(4.5f == ysm_imean(im));
	for (i = 0; i < arrsz(vs2) / 2; i++)
		ysm_imean_add(im0, vs2[i]);
	for (; i < arrsz(vs2); i++)
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
	for (i = 0; i < arrsz(vs0); i++)
		ysm_ivar_add(iv, vs0[i]);
	yassert(0 == ysm_ivar(iv));
	ysm_ivar_destroy(iv);

	iv = ysm_ivar_create();
	for (i = 0; i < arrsz(vs1); i++)
		ysm_ivar_add(iv, vs1[i]);
	yassert(4 == ysm_ivar(iv));
	ysm_ivar_destroy(iv);

	iv = ysm_ivar_create();
	iv0 = ysm_ivar_create();
	iv1 = ysm_ivar_create();
	iv2 = ysm_ivar_create();
	for (i = 0; i < arrsz(vs2); i++)
		ysm_ivar_add(iv, vs2[i]);
	yassert(5.25f == ysm_ivar(iv));
	for (i = 0; i < arrsz(vs2) / 2; i++)
		ysm_ivar_add(iv0, vs2[i]);
	for (; i < arrsz(vs2); i++)
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
