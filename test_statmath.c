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
	struct ysm_imean *im = ysm_imean_create();
	for (i = 0; i < arrsz(vs0); i++)
		ysm_imean_add(im, vs0[i]);
	yassert(1 == ysm_imean(im));
	ysm_imean_destroy(im);

	im = ysm_imean_create();
	for (i = 0; i < arrsz(vs1); i++)
		ysm_imean_add(im, vs1[i]);
	yassert(4 == ysm_imean(im));
	ysm_imean_destroy(im);

	im = ysm_imean_create();
	for (i = 0; i < arrsz(vs2); i++)
		ysm_imean_add(im, vs2[i]);
	yassert(4.5f == ysm_imean(im));
	ysm_imean_destroy(im);
}

static void
test_ivar(void) {
	int i;
	double vs0[] = {1};
	double vs1[] = {1, 2, 3, 4, 5, 6, 7};
	double vs2[] = {1, 2, 3, 4, 5, 6, 7, 8};
	struct ysm_ivar *iv = ysm_ivar_create();
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
	for (i = 0; i < arrsz(vs2); i++)
		ysm_ivar_add(iv, vs2[i]);
	yassert(5.25f == ysm_ivar(iv));
	ysm_ivar_destroy(iv);

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
