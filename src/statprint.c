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

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "common.h"
#include "ystatprint.h"
#include "ystatmath.h"

/* 64 bit = 4 * 2^10^6  = 4 * 10^3^6 = 4 * 10^18 < 30 characters */
#define MAX_DOUBLE_STR_LEN 30
#define BAR_GRAPH_MIN_HEIGHT 2
#define BAR_GRAPH_MAX_VALUE_SZ 200
#define BAR_GRAPH_MAX_INTER_SPACE 5

static int
stats_double(double *omin, /* [out] */
	     double *omax, /* [out] */
	     double *mean, /* [out] */
	     double *var,  /* [out] variance */
	     const double *vs,
	     u32 vsz) {
	const double *d, *de;
	double r, min, max;
	struct ysm_ivar iv;
	register int calvar;
	de = vs + vsz;
	max = -DBL_MAX;
	min = DBL_MAX;
	calvar = mean || var;
	if (calvar)
		ysm_ivar_init(&iv);
	for (d = vs; d < de; ++d) {
		r = fpclassify(*d);
		if (unlikely(r != FP_NORMAL
			     && r != FP_ZERO))
			return EINVAL;
		if (unlikely(*d < min))
			min = *d;
		if (unlikely(*d > max))
			max = *d;
		if (calvar)
			ysm_ivar_add(&iv, *d);
	}
	if (likely(omin))
		*omin = min;
	if (likely(omax))
		*omax = max;
	if (mean)
		*mean = ysm_ivar_mean(&iv);
	if (var)
		*var = ysm_ivar(&iv);
	return 0;
}


/* assmes that buffer is large enough */
static void
bargraph_prline(int fd,
		char *buf,
		u32 bufsz,
		u32 sz,
		double v) {
	u32 i;
	int r __unused;
	char *p, *pe;

	/* print top line */
	p = buf;
	pe = buf + bufsz;
	for (i = 0; i < sz; i++)
		*p++ = '-';
	if (unlikely(0 > (r = snprintf(p, pe - p, " %f\n", v))))
		return;
	p += r;
	/* ignore IO error */
	r = write(fd, buf, p - buf);
}

int
ystpr_bargraph(int fd,
	       const double *vs,
	       u32 vsz,
	       u32 height,
	       const u32 *idxs,
	       const char * const *cmts,
	       u32 isz,
	       u32 isp,
	       char barc) {
	const double *d, *de;
	char *p;
	u32 i, j, n;
	int iv;
	int r;
	double min, max;
	char ln[4096]; /* large enough line buffer */

	assert(sizeof(ln) > BAR_GRAPH_MAX_VALUE_SZ);
	if (unlikely(BAR_GRAPH_MIN_HEIGHT > height
		     || isp > BAR_GRAPH_MAX_INTER_SPACE
		     || vsz > BAR_GRAPH_MAX_VALUE_SZ
		     || vsz * (isp + 1) + MAX_DOUBLE_STR_LEN > sizeof(ln)
		     || fd < 0
		     || !vs
		     || !vsz))
		return EINVAL;

	/* check buffer boundary */
	n = 0; /* previous value */
	for (i = 0; i < isz; i++) {
		if (unlikely(idxs[i] >= vsz
			     || n > idxs[i] /* array is well-sorted? */
			     /* why + 3?. It is for \r \n \0. */
			     || (idxs[i] * (isp + 1) + strlen(cmts[i]) + 3
				 > sizeof(ln))))
			return EINVAL;
		n = idxs[i];
	}

	/* get min and max */
	if (unlikely(r = stats_double(&min, &max, NULL, NULL, vs, vsz)))
		return r;

	/* print top line */
	bargraph_prline(fd, ln, sizeof(ln), vsz * (isp + 1), max);

	/* print bar graph */
	de = vs + vsz;
	for (i = height; i > 0; i--) {
		p = ln;
		for (d = vs; d < de; ++d) {
			iv = (int)((*d - min) * (double)height / (max - min));
			if (iv >= i - 1
			    || (i == height
			        && iv >= height)
			    || (i == 1
			        && iv < 0))
				*p++ = barc;
			else
				*p++ = ' ';
			memset(p, ' ', isp);
			p += isp;
		}
		*p++ = '\n';
		r = write(fd, ln, p - ln); /* ignore IO error */
	}

	/* print bottom line */
	bargraph_prline(fd, ln, sizeof(ln), vsz * (isp + 1), min);

	if (!idxs || !cmts || !isz)
		return 0; /* all done */

	/* print index arrow */
	r = -1; /* max position */
	memset(ln, ' ', sizeof(ln));
	for (i = 0; i < isz; i++) {
		int pos = idxs[i] * (isp + 1);
		if (r < pos)
			r = pos;
		ln[pos] = '^';
	}
	ln[++r] = '\n';
	r = write(fd, ln, r + 1); /* ignore IO error */

	/* print first and second line index string */
	for (j = 0; j < 2; j++) {
		n = 0; /* max position */
		memset(ln, ' ', sizeof(ln));
		for (i = j; i < isz; i += 2) {
			int len = strlen(cmts[i]);
			u32 pos = idxs[i] * (isp + 1) + strlen(cmts[i]);
			if (pos > n)
				n = pos;
			memcpy(&ln[idxs[i] * (isp + 1)], cmts[i], len);
		}
		ln[n++] = '\n';
		r = write(fd, ln, n); /* ignore IO error */
	}

	return 0;
}

int
ystpr_distgraph(int fd,
		const double *vs,
		u32 vsz,
		u32 w,
		u32 h,
		u32 isp,
		char barc) {
	/*
	 * Comment text will be something like
	 *   "-2s", "-s", "-3s", "u", "s", "2s" ...
	 * And min/max value - print double value.
	 * So, 64 should be enough
	 */
#define _MAX_CMT_LEN 64
#define _PRSTR_ALL_SAME_VALUES "All values are same.\n"
#define _dist_index(v) (((v) - min) * (double)w / interval)
#define _get_dist_index(out, v)				\
	do {							\
		tmp = _dist_index(v);				\
		if (unlikely(tmp > w - 1))			\
			tmp = w - 1;				\
		(out) = tmp;					\
	} while (0);

	const double *d;
	double min, max, var, sigma, mean, interval, *dist;
	u32 tmp, r, i, j, sigminus, sigplus, nrcmt, *idxs;
	char **cmts;
	const double * const de = vs + vsz;

	/* verify input and calculate required values. */
	if (unlikely(r = stats_double(&min, &max, &mean, &var, vs, vsz)))
		return r;

	sigma = sqrt(var);
	/* all values are same. */
	if (min >= max) {
		/* ignore IO error. '- 1' to remove trailing 0 */
		r = write(fd,
			  _PRSTR_ALL_SAME_VALUES,
			  sizeof(_PRSTR_ALL_SAME_VALUES) - 1);
		return 0;
	}

	/* allocates memories required */
	if (unlikely(!(dist = (double *)ycalloc(w, sizeof(*dist)))))
		return ENOMEM;

	/* calculate distribution */
	interval = max - min;
	for (d = vs; d < de; d++) {
		_get_dist_index(i, *d);
		dist[i]++;
	}

	/* calculate 'u - Ns' that can be shown at the graph */
	i = 0;
	while (0 <= _dist_index(mean - ++i * sigma));
	sigminus = i - 1;
	i = 0;
	while (w > _dist_index(mean + ++i * sigma));
	sigplus = i - 1;

	/* + 1 for 'mean', + 2 for min/max */
	nrcmt = sigminus + sigplus + 1 + 2;
	if (unlikely(!(cmts = ymalloc(sizeof(*cmts) * nrcmt)))) {
		yfree(dist);
		return ENOMEM;
	}
	if (unlikely(!(idxs = ymalloc(sizeof(*idxs) * nrcmt)))) {
		yfree(dist);
		yfree(cmts);
		return ENOMEM;
	}

	for (i = 0; i < nrcmt; i++) {
		cmts[i] = (char *)ymalloc(sizeof(*cmts[i])
					  * _MAX_CMT_LEN);
		if (unlikely(!cmts[i])) {
			for (j = 0; j < i; j++)
				yfree(cmts[j]);
			yfree(idxs);
			yfree(dist);
			return ENOMEM;
		}
		cmts[i][_MAX_CMT_LEN - 1] = 0; /* trailing 0 */
	}

	j = 0;

	/* add comment for min */
	idxs[j] = 0;
	snprintf(cmts[j++], _MAX_CMT_LEN - 1, "%f", min);
	for (i = sigminus; i > 0; i--) {
		_get_dist_index(idxs[j], mean - i * sigma);
		/* ignore return value intentionally */
		snprintf(cmts[j++], _MAX_CMT_LEN - 1, "-%ds", i);
	}

	_get_dist_index(idxs[j], mean);
	snprintf(cmts[j++], _MAX_CMT_LEN - 1, "u");
	for (i = 1; i <= sigplus; i++) {
		_get_dist_index(idxs[j], mean + i * sigma);
		/* ignore return value intentionally */
		snprintf(cmts[j++], _MAX_CMT_LEN - 1, "%ds", i);
	}
	/* add comment for max */
	idxs[j] = w - 1;
	snprintf(cmts[j++], _MAX_CMT_LEN - 1, "%f", max);

	r = ystpr_bargraph(fd,
			   dist,
			   w,
			   h,
			   idxs,
			   (const char * const *)cmts,
			   nrcmt,
			   isp,
			   barc);

	yfree(idxs);
	yfree(dist);
	for (i = 0; i < nrcmt; i++)
		yfree(cmts[i]);
	yfree(cmts);

	return r;

#undef _get_dist_index
#undef _dist_index
#undef _PRSTR_ALL_SAME_VALUES
#undef _MAX_CMT_LEN
}
