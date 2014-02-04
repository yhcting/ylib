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

#ifndef __YSTATMATh_h__
#define __YSTATMATh_h__

#include "ydef.h"

/******************************************************************************
 *
 * Incremental mean
 *
 *****************************************************************************/
struct ysm_imean;

/**
 * @return : NULL if fails (usually OOM)
 */
EXPORT struct ysm_imean *
ysm_imean_create(void);

EXPORT void
ysm_imean_destroy(struct ysm_imean *);

EXPORT void
ysm_imean_add(struct ysm_imean *, double v);

/**
 * get mean value.
 */
EXPORT double
ysm_imean(struct ysm_imean *);


/******************************************************************************
 *
 * Incremental variance
 *
 *****************************************************************************/
struct ysm_ivar;

/**
 * @return : NULL if fails (usually OOM)
 */
EXPORT struct ysm_ivar *
ysm_ivar_create(void);

EXPORT void
ysm_ivar_destroy(struct ysm_ivar *);

EXPORT void
ysm_ivar_add(struct ysm_ivar *, double v);

/**
 * get mean value.
 */
EXPORT double
ysm_ivar_mean(struct ysm_ivar *);

/**
 * get variance.
 */
EXPORT double
ysm_ivar(struct ysm_ivar *);

#endif /* __YSTATMATh_h__ */
