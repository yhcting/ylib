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


#ifndef __COMMOn_h__
#define __COMMOn_h__

#include "ydef.h"
#include "yret.h"

/*=======================================
 * Macros
 *=======================================*/

#ifndef abs
#	define abs(x) (((x)>0)?(x):-(x))
#endif

#ifndef swap
#	define swap(x,y,tmp) do { tmp=x;x=y;y=tmp; } while (0)
#endif

#ifndef min
#	define min(x,y) (((x)<(y))?x:y)
#endif

#ifndef max
#	define max(x,y) (((x)<(y))?y:x)
#endif

#endif /* __COMMOn_h__ */
