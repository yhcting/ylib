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

#ifndef __TESt_h__
#define __TESt_h__

extern int dmem_count(void);
extern void dregister_tstfn(void (*fn)(void), const char *mod);

#define TESTFN(fn, mod)							\
	static void __tst_##fn(void) __attribute__ ((constructor));	\
	static void __tst_##fn(void) {					\
		dregister_tstfn(&fn, #mod);				\
	}

#endif /* __TESt_h__ */
