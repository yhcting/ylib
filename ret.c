/*****************************************************************************
 *    Copyright (C) 2011 Younghyung Cho. <yhcting77@gmail.com>
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


/******************************************************************************
 *
 * Multi-threading is not considered yet!
 *
 *****************************************************************************/


#include "yret.h"



static enum yret _last_ret = YROk; /* last error of this ylibrary */


#define DEFRETVAL(name, desc) desc,
static const char* _errdesc[] = {
#include "yret_err.in"
};

static const char* _warndesc[] = {
#include "yret_warn.in"
};
#undef DEFRETVAL


void
yretset(enum yret err) { _last_ret = err; }

enum yret
yretget(void) { return _last_ret; }

const char*
yretstring(enum yret r) {
	if (YROk == r) {
		return "Ok";
	} else if (YREVALStart < r && YREVALEnd > r) {
		yassert(r < 0);
		return _errdesc[r - YREVALStart - 1];
	} else if (YRWVALStart < r && YRWVALEnd > r) {
		yassert(r > 0);
		return _warndesc[r- YRWVALStart - 1];
	} else {
		/* Invalid argument */
		yassert(0);
		return NULL;
	}
}

