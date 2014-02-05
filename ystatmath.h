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

#include <math.h>
#include "ycommon.h"

/******************************************************************************
 *
 * Incremental mean
 *
 *****************************************************************************/
struct ysm_imean {
	unsigned int n; /* # of elements accumulated */
	double       u; /* current mean */
};

/*
 * u(n) = u(n-1) + (x(n) - u(n-1)) / n
 */
static inline double
__imean_add(unsigned int n, /* # of elements - including given new value 'v' */
	    double u,       /* mean of 'n-1' elements */
	    double v) {     /* new value */
	return u + (v - u) / (double)n;
}

/**
 * @return : NULL if fails (usually OOM)
 */
static inline struct ysm_imean *
ysm_imean_create(void) {
	return (struct ysm_imean *)ycalloc(1, sizeof(struct ysm_imean));
}

static inline void
ysm_imean_destroy(struct ysm_imean *im) {
	yfree(im);
}

static inline void
ysm_imean_add(struct ysm_imean *im, double v) {
	im->n++;
	im->u = __imean_add(im->n, im->u, v);
}

static inline double
ysm_imean(const struct ysm_imean *im) {
	return im->u;
}

/**
 * get number of elements participated.
 */
static inline unsigned int
ysm_imean_n(const struct ysm_imean *im) {
	return im->n;
}


/******************************************************************************
 *
 * Incremental variance
 * (Knuth's equation)
 *
 *****************************************************************************/
struct ysm_ivar {
	unsigned int n; /* # of elements accumulated */
	double       s; /* S(n) */
	double       u; /* mean */
};

static inline struct ysm_ivar *
ysm_ivar_create(void) {
	return (struct ysm_ivar *)ycalloc(1, sizeof(struct ysm_ivar));
}

static inline void
ysm_ivar_destroy(struct ysm_ivar *iv) {
	yfree(iv);
}

/*
 * Notation
 * V : standard variance (= sigma^2)
 * sigma : standard deviation
 * u : mean
 * x : element
 *
 * Let S(n) = n * V(n)
 * S(n) = S(n-1) + (x(n) - u(n-1)) * (x(n) - u(n))
 */
static inline void
ysm_ivar_add(struct ysm_ivar *iv, double v) {
	double ou = iv->u;
	double os = iv->s;
	iv->n++;
	iv->u = __imean_add(iv->n, ou, v);
	iv->s = os + (v - ou) * (v - iv->u);
}

static inline double
ysm_ivar_mean(const struct ysm_ivar *iv) {
	return iv->u;
}

/**
 * get variance of this sample.
 * @return NAN is returned for empty sample.
 */
static inline double
ysm_ivar(const struct ysm_ivar *iv) {
	/* This is floating point division operation.
	 * So, we don't need to check 'divide by zero' exception.
	 * (By IEEE) <non-zero>/0 => +-INF, 0/0 => NaN
	 */
	return iv->s / (double)iv->n;
}

/**
 * get number of elements participated.
 */
static inline unsigned int
ysm_ivar_n(const struct ysm_ivar *iv) {
	return iv->n;
}

/**
 * get 95% confidence interval.
 * 'min' and 'max' are set as NAN for empty sample.
 */
static inline void
ysm_ivar_ci95(const struct ysm_ivar *iv, double *min, double *max) {
	double s, r;
	if (unlikely(!iv->n)) {
		*min = *max = NAN;
		return;
	}
	s = sqrt(ysm_ivar(iv));
	r = 1.96f * s / sqrt(iv->n);
	*min = iv->u - r;
	*max = iv->u + r;
}

#endif /* __YSTATMATh_h__ */
