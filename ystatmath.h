/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014
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
	u32    n; /* # of elements accumulated */
	double u; /* current mean */
};

/*
 * u(n) = u(n-1) + (x(n) - u(n-1)) / n
 */
static inline double
__imean_add(u32 n, /* # of elements - including given new value 'v' */
	    double u,       /* mean of 'n-1' elements */
	    double v) {     /* new value */
	/* n is never 0 */
	return u + (v - u) / (double)n;
}

static inline void
ysm_imean_init(struct ysm_imean *im) {
	memset(im, 0, sizeof(*im));
}

/**
 * @return : NULL if fails (usually OOM)
 */
static inline struct ysm_imean *
ysm_imean_create(void) {
	return (struct ysm_imean *)ycalloc(1, sizeof(struct ysm_imean));
}

static inline void
ysm_imean_clean(struct ysm_imean *im) {
	ysm_imean_init(im);
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
static inline u32
ysm_imean_n(const struct ysm_imean *im) {
	return im->n;
}

/**
 * combine two samples.
 * destination 'out' SHOULD NOT be used as source.
 */
static inline void
ysm_imean_combine(struct ysm_imean *out,
		  const struct ysm_imean* a0,
		  const struct ysm_imean* a1) {
	out->n = a0->n + a1->n;
	/* We don't need to care a case that out->n == 0 - divide by zero,
	 *   because NaN operations are well-defined at IEEE 754.
	 * Then, why below condition exist?
	 * After combining two empty samples, output may be used as new
	 *   sample and being used again.
	 * Once 'out->u' is set as NaN, it cannot be used at above case.
	 */
	if (unlikely(!out->n)) {
		memset(out, 0, sizeof(*out));
		return;
	}
	out->n = a0->n + a1->n;
	/* u(n + m) = (u(n) * n + u(m) * m) / (n + m) */
	out->u = (a0->u * (double)a0->n + a1->u * (double)a1->n)
		 / (double)out->n;
}

/******************************************************************************
 *
 * Incremental variance
 * (Knuth's equation)
 *
 *****************************************************************************/
struct ysm_ivar {
	u32    n; /* # of elements accumulated */
	double s; /* S(n) */
	double u; /* mean */
};

static inline void
ysm_ivar_init(struct ysm_ivar *iv) {
	memset(iv, 0, sizeof(*iv));
}

static inline struct ysm_ivar *
ysm_ivar_create(void) {
	return (struct ysm_ivar *)ycalloc(1, sizeof(struct ysm_ivar));
}

static inline void
ysm_ivar_clean(struct ysm_ivar *iv) {
	ysm_ivar_init(iv);
}

static inline void
ysm_ivar_destroy(struct ysm_ivar *iv) {
	yfree(iv);
}

static inline void
ysm_ivar_add(struct ysm_ivar *iv, double v) {
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
	double ou = iv->u;
	iv->n++;
	iv->u = __imean_add(iv->n, ou, v);
	iv->s = iv->s + (v - ou) * (v - iv->u);
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
static inline u32
ysm_ivar_n(const struct ysm_ivar *iv) {
	return iv->n;
}

static inline void
ysm_ivar_combine(struct ysm_ivar *out,
		 const struct ysm_ivar* a0,
		 const struct ysm_ivar* a1) {
	out->n = a0->n + a1->n;
	/* We don't need to care a case that out->n == 0 - divide by zero,
	 *   because NaN operations are well-defined at IEEE 754.
	 * But, this is for same reason with above 'imean' case.
	 * (See above comments in 'ysm_imean_combine')
	 */
	if (unlikely(!out->n)) {
		memset(out, 0, sizeof(*out));
		return;
	}
	/* See imean for below equation */
	out->u = (a0->u * (double)a0->n + a1->u * (double)a1->n)
		/ (double)out->n;
	/*
	 * See 'ysm_ivar_add' for notation.
	 * V(n + m) = ((V(n) + u(n)^2) * n + (V(m) + u(m)^2) * m) / (n + m)
	 *            - u(n + m)^2
	 *
	 * Let N = n + m, Then
	 * S(N) = S(n) + n * u(n)^2 + S(m) + m * u(m)^2 - N * u(N)^2
	 */
	out->s = a0->s + a0->n * a0->u * a0->u
		+ a1->s + a1->n * a1->u * a1->u
		- out->n * out->u * out->u;
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
