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

/**
 * @file ystatmath.h
 * @brief Header to use statmath module which helps calculate statistics data.
 */

#ifndef __YSTATMATh_h__
#define __YSTATMATh_h__

#include <math.h>
#include <string.h>

#include "ydef.h"

/******************************************************************************
 *
 * Incremental mean
 *
 *****************************************************************************/
/**
 * Structure to calculate incremental mean value.
 */
struct ysm_imean {
	/* @cond */
	uint32_t n; /* # of elements accumulated */
	double u; /* current mean */
	/* @endcond */
};

/**
 * Initialise structure for incremental-mean.
 *
 * @param im Inremental-mean object
 */
static YYINLINE void
ysm_imean_init(struct ysm_imean *im) {
	memset(im, 0, sizeof(*im));
}

/**
 * Create incremental-mean-object.
 *
 * @return NULL if fails (usually OOM)
 */
YYEXPORT struct ysm_imean *
ysm_imean_create(void);

/**
 * Clean incremental-mean-object. Every values are set to 0.
 *
 * @param im Inremental-mean object
 */
static YYINLINE void
ysm_imean_reset(struct ysm_imean *im) {
	ysm_imean_init(im);
}

/**
 * Destroy incremental-mean-object.
 */
YYEXPORT void
ysm_imean_destroy(struct ysm_imean *);

/**
 * Add value and calculate mean incrementally.
 */
YYEXPORT void
ysm_imean_add(struct ysm_imean *, double);

/**
 * Get current mean.
 *
 * @param im Inremental-mean object
 * @return Mean
 */
static YYINLINE double
ysm_imean(const struct ysm_imean *im) {
	return im->u;
}

/**
 * Get number of elements participated.
 *
 * @param im Inremental-mean object
 * @return Number of elements
 */
static YYINLINE uint32_t
ysm_imean_n(const struct ysm_imean *im) {
	return im->n;
}

/**
 * Combine two samples(incremental mean).
 * Destination {@code out} SHOULD NOT be used as source.
 *
 * @param out (out) Object in where combined value is set.
 */
YYEXPORT void
ysm_imean_combine(struct ysm_imean *out,
		  const struct ysm_imean *,
		  const struct ysm_imean *);

/******************************************************************************
 *
 * Incremental variance
 * (Knuth's equation)
 *
 *****************************************************************************/
/**
 * Structure to calculate incremental variance.
 */
struct ysm_ivar {
	/* \cond */
	uint32_t n; /* # of elements accumulated */
	double s; /* S(n) */
	double u; /* mean */
	/* \endcond */
};

/**
 * Initialise structure for incremental-variance.
 *
 * @param iv Incremental-variance object
 */
static YYINLINE void
ysm_ivar_init(struct ysm_ivar *iv) {
	memset(iv, 0, sizeof(*iv));
}

/**
 * Create incremental-variance object.
 *
 * @return NULL if fails (usually ENOMEM)
 */
YYEXPORT struct ysm_ivar *
ysm_ivar_create(void);

/**
 * Reset incremental-variance object. Every values are set to 0.
 *
 * @param iv Incremental-variance object
 */
static YYINLINE void
ysm_ivar_reset(struct ysm_ivar *iv) {
	ysm_ivar_init(iv);
}

/**
 * Destroy incremental-variance-object
 */
YYEXPORT void
ysm_ivar_destroy(struct ysm_ivar *);

/**
 * Add value and calculate variance incrementally.
 */
YYEXPORT void
ysm_ivar_add(struct ysm_ivar *, double);

/**
 * Get current mean value.
 *
 * @param iv Incremental-variance object
 * @return Current mean value.
 */
static YYINLINE double
ysm_ivar_mean(const struct ysm_ivar *iv) {
	return iv->u;
}

/**
 * Get variance of this sample.
 *
 * @param iv Incremental-variance object
 * @return NAN for empty sample.
 */
static YYINLINE double
ysm_ivar(const struct ysm_ivar *iv) {
	/* This is floating point division operation.
	 * So, we don't need to check 'divide by zero' exception.
	 * (By IEEE) <non-zero>/0 => +-INF, 0/0 => NaN
	 */
	return iv->s / (double)iv->n;
}

/**
 * Get number of elements participated.
 *
 * @param iv Incremental-variance object
 * @return number of participated elements
 */
static YYINLINE uint32_t
ysm_ivar_n(const struct ysm_ivar *iv) {
	return iv->n;
}

/**
 * Combine two samples and save result to {@code out}.
 * Destination {@code out} SHOULD NOT be used as source.
 *
 * @param out (out) Object in where combined value is set.
 */
YYEXPORT void
ysm_ivar_combine(struct ysm_ivar *out,
		 const struct ysm_ivar *,
		 const struct ysm_ivar *);

/**
 * Get 95% confidence interval.
 * 'min' and 'max' are set as NAN for empty sample.
 *
 * @param iv Incremental-variance object
 * @param min (out) min. value of the interval
 * @param max (out) max. value of the interval
 */
YYEXPORT void
ysm_ivar_ci95(const struct ysm_ivar *iv, double *min, double *max);

#endif /* __YSTATMATh_h__ */
