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
 * Abbrev
 *    YRxxx : yretxxx
 *****************************************************************************/


#ifndef __YREt_h__
#define __YREt_h__

#include "ydef.h"

#define DEFRETVAL(name, desc) name,
enum yret {
	/* Error should be minus value */
	YREVALStart = -0x7fffffff,
#include "yret_err.in"
	YREVALEnd,

	/* Don't change value of YROk! */
	YROk        = 0,

	/* Warning should be plus value */
	YRWVALStart = 0,
#include "yret_warn.in"
	YRWVALEnd
};
#undef DEFRETVAL


void
yretset(enum yret ret);


/***********************
 * Exported interface
 ***********************/

/*
 * get last warning or error
 */
EXPORT enum yret
yretget();

EXPORT const char*
yretstring(enum yret);


#endif /* __YREt_h__ */
