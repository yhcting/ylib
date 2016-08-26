/******************************************************************************
 * Copyright (C) 2015
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

/*
 * Region is, two dimensional array([Y][X]) of band with following conditions
 * - Length of array is NOT fixed
 * - Bands in same axis in array, are sorted based on 'small' value.
 * - Bands in same axis in array, aren't overlapped with others.
 * - 'Y' axis([Y]) represents bands with Y-coord (parallel on x-axis).
 * - 'X' axis([X]) represents bands with X-coord (parallel on y-axis).
 * - Bands in [Y][...] are x-bands which have same Y-band.
 *   (That is, they are rectanges.)
 *
 * |<---------- sharing same y-band ------------->|<-- ...
 * +----------------------------------------------+------+-------------+
 * | +--------+---------+---------+---+---------+ |      | +---------+ |
 * | | y-band | x-band0 | x-band1 |...| x-empty | | ...  | | y-empty | |
 * | +--------+---------+---------+---+---------+ |      | +---------+ |
 * +----------------------------------|---------|-+------+-|---------|-+
 *                                    |         |          |         |
 *                                    |<------->|          |<------->|
 *                                x-band end-sentinel    y-band end-sentinel
 *                                                  (Sentinel for end-of-region)
 * Example
 *
 * |//|  |//|  <- Two x-bands
 * |//|  |//|
 * v  v  v  v
 * +--+  +--+          <------------ One y-band
 * |//|  |//|           ////////////
 * +--+  +--+------+   <------------
 * +---------------+
 * |\\\\\\\\\\\\\\\|
 * |\\\\\\\\\\\\\\\|
 * +---+---+---+---+------+
 *     |***|   |++++++++++|
 *     +---+   +----------+
 *
 *
 * So, region is must array of struct yband.
 * [Pros]
 * - Simple data structuture.
 * - Fast.
 *
 * [Cons]
 * - Hard to understand.
 *
 * We may design data structure something like below.
 * [# ybands] [yband0][# xbands][x-band0][x-band1]...
 *            [yband1][# xbands][...
 *
 *
 *
 * [ Abbrev ]
 * lho : Left-Hand Operand
 * rho : Right-Hand Operand
 *
 */

#include "geo.h"
#include "yband.h"
#include "yrect.h"
#include "yrgn.h"
#include "../ydynb.h"
#include "../yut.h"

struct yrgn {
	struct yband bs[0];
};

struct rectband {
	struct yband yb;
	struct yband xbs[0];
};

/*****************************************************************************
 *
 * Constants
 *
 *****************************************************************************/
/* define invalid non-empty band used as dummy-non-empty initial value */
#define BAND_MIN_INVALID {GEO_INVALID_MIN_VALUE, GEO_INVALID_MIN_VALUE + 1}
#define verify_band_min_invalid(band) \
	yassert(GEO_INVALID_MIN_VALUE == (band)->s	\
		&& GEO_INVALID_MIN_VALUE + 1 == (band)->b);
static const struct yband band_min_invalid = BAND_MIN_INVALID;
static const struct yband rband_min_invalid[] = {
	BAND_MIN_INVALID, /* yb */
	BAND_MIN_INVALID, /* xbs[0] */
	{0, 0} /* x-band end-sentinel */
};


#define BAND_MAX_INVALID {GEO_INVALID_MAX_VALUE - 1, GEO_INVALID_MAX_VALUE}
#define verify_band_max_invalid(band) \
	yassert(GEO_INVALID_MAX_VALUE -1 == (band)->s	\
		&& GEO_INVALID_MAX_VALUE == (band)->b);
static const struct yband band_max_invalid = BAND_MAX_INVALID;
static const struct yband rband_max_invalid[] = {
	BAND_MAX_INVALID, /* yb */
	BAND_MAX_INVALID, /* xbs[0] */
	{0, 0} /* x-band end-sentinel */
};

#ifdef CONFIG_DEBUG

static inline void
assert_band_min_invalid(void) {
	verify_band_min_invalid(&band_min_invalid);
}

static inline void
assert_rband_min_invalid(void) {
	const struct yband *b = rband_min_invalid;
	verify_band_min_invalid(&b[0]);
	verify_band_min_invalid(&b[1]);
	yassert(!b[2].s && !b[2].b);
}

static inline void
assert_band_max_invalid(void) {
	verify_band_max_invalid(&band_max_invalid);
}

static inline void
assert_rband_max_invalid(void) {
	const struct yband *b = rband_max_invalid;
	yassert(GEO_INVALID_MAX_VALUE - 1 == b[0].s
		&& GEO_INVALID_MAX_VALUE == b[0].b
		&& GEO_INVALID_MAX_VALUE - 1 == b[1].s
		&& GEO_INVALID_MAX_VALUE == b[1].b
		&& !b[2].s
		&& !b[2].b);
}

#else /* CONFIG_DEBUG */

static inline void assert_band_min_invalid(void) { }
static inline void assert_rband_min_invalid(void) { }
static inline void assert_band_max_invalid(void) { }
static inline void assert_rband_max_invalid(void) { }

#endif /* CONFIG_DEBUG */

/*****************************************************************************
 *
 * Helpers
 *
 *****************************************************************************/
static inline struct yrgn *
bands2rgn(const struct yband *b) {
	return (struct yrgn *)b;
}

static inline struct yband *
rgn2bands(const struct yrgn *g) {
	return (struct yband *)g;
}

static inline struct yrgn *
rband2rgn(const struct rectband *rb) {
	return (struct yrgn *)rb;
}

static inline struct rectband *
rgn2rband(const struct yrgn *g) {
	return (struct rectband *)g;
}

static inline struct yband *
rband2bands(const struct rectband *rb) {
	return (struct yband *)rb;
}

static inline struct rectband *
bands2rband(const struct yband *b) {
	return (struct rectband *)b;
}

/* assumes buffer(out) size is large enough */
static inline void
mutate_rect_to_rgnbands(struct yband *out, const struct yrect *r) {
	if (ygeor_is_empty(r))
		ygeob_set_empty(&out[0]);
	else {
		*((struct yrect *)out) = *r;
		ygeob_set_empty(&out[2]); /* x-band sentinel */
		ygeob_set_empty(&out[3]); /* rgn(y-band) sentinel */
	}
}

/*
 * 'bsz' means 'Band SiZe'
 * # of bands in rect band(including end-sentinels).
 * Band-array in region follows special rules.
 * (See comment at head of file.)
 */
static inline u32
rband_bsz(const struct rectband *rb) {
	const struct yband *p = rband2bands(rb);
#ifdef CONFIG_DEBUG
	if (!ygeob_is_empty(p)) {
		/* Verify x-band array */
		p++; /* move to x-bands */
		/* Verify rect bands */
		const struct yband *prev = &band_min_invalid;
		while (!ygeob_is_empty(p)) {
			/* x-bands doesn't overlap */
			yassert(p->s < p->b && prev->b < p->s);
			prev = p;
			p++;
		}
	}
	p = rband2bands(rb);
#endif /* CONFIG_DEBUG */
	while (!ygeob_is_empty(p++));
	return (u32)(p - rband2bands(rb));
}

/*
 *'bsz' means 'Band SiZe'
 * # of bands in rgn including end-sentinels.
 */
static inline u32
rgn_bsz(const struct yrgn *g) {
	const struct yband *b = rgn2bands(g);
#ifdef CONFIG_DEBUG
	const struct yband *prevyb = &band_min_invalid;
	while (!ygeob_is_empty(b)) {
		yassert(b->s < b->b && prevyb->b <= b->s);
		b += rband_bsz(bands2rband(b));
	}
	b = rgn2bands(g);
#endif /* CONFIG_DEBUG */
	while (!ygeob_is_empty(b)) {
		while (!ygeob_is_empty(b++));
	}
	/* b is at rgn-end-sentinel */
	return b - rgn2bands(g) + 1;
}


/*****************************************************************************
 *
 * Band
 *
 *****************************************************************************/
static inline bool
band_can_merge(const struct yband *bs, const struct yband *bb) {
	YYassert(!ygeob_is_empty(bs)
		 && !ygeob_is_empty(bb)
		 && bs->s < bb->s
		 && bs->b <= bb->s);
	return bs->b == bb->s;
}

/*
 * @param top_lho Is top-band belongs to lho-band?
 * @return # of bands added to 'outb'(0 or 1).
 */
static inline u32
band_add(struct yband *prevb,
	 struct yband *outb,
	 const struct yband *b) {
	dpr("band_add: prev(%d, %d) b(%d, %d)\n",
	    prevb->s, prevb->b, b->s, b->b);
	if (unlikely(ygeob_is_empty(b)))
		return 0;
	yassert(!ygeob_is_empty(prevb)
		&& prevb->b <= b->s);
	if (band_can_merge(prevb, b)) {
		prevb->b = b->b; /* merge */
		return 0;
	}
	*outb = *b;
	return 1;
}

/*
 * @param b_lho Is band belongs to lho-band?
 * @return # of bands added to 'outb' (0 or 1).
 *
 */
static inline u32
band_op_no_overlap(struct yband *prevb,
		   struct yband *outb,
		   const struct yband *b,
		   enum optype opty,
		   bool b_lho) {
	/* top band has meaning only in following cases */
	if (OP_UNION == opty
	    || (OP_DIFF == opty
		&& b_lho))
		return band_add(prevb, outb, b);
	return 0;
}

static inline u32
band_op_overlap(struct yband *prevb,
		struct yband *outb,
		const struct yband *ol,
		enum optype opty) {
	if (OP_DIFF == opty)
		return 0;
	return band_add(prevb, outb, ol);
}

/*
 * @param pprevb Value is updated inside this function.
 * @return # of bands written at 'outb'
 *         (Never fails!)
 */
static inline u32
band_op(struct yband *outb, /* (Out) array having at least 3 free spaces */
	bool *botb0, /* (Out) bottom from b0 */
	struct yband *pobb, /* Out */
	struct yband **pprevb, /* (InOut) */
	const struct yband *b0, /* (In) */
	const struct yband *b1, /* (In */
	enum optype opty) {
	enum ybandpos bpr; /* Band Pos Returned */
	const struct yband *bs, *bb;
	struct yband obs, obo;
	bool bs_b0; /* is bs == *p0botb */
	struct yband *cob = outb; /* Current Out Band */
	yassert(!ygeob_is_empty(*pprevb)
		&& !ygeob_is_empty(b0)
		&& !ygeob_is_empty(b1));
	yut_cmpst2(bs_b0, bs, bb, b0, b1, ->s);
	bpr = ygeob_analyze_layout(&obs, &obo, pobb, bs, bb);
	dpr("band_op: bs(%d, %d), bb(%d, %d): bs_b0:%d\n"
	    "    bpr:%d\n",
	    bs->s, bs->b, bb->s, bb->b, bs_b0, bpr);
	/* top band - no overlap */
	if (likely(band_op_no_overlap(*pprevb,
				      cob,
				      &obs,
				      opty,
				      bs_b0))) {
		dpr("band_op(nool): prevb(%d, %d) cob(%d, %d)\n",
		    (*pprevb)->s, (*pprevb)->b,
		    cob->s, cob->b);
		*pprevb = cob;
		cob++;
	}
	/* overlapping band in the middle */
	if (likely(band_op_overlap(*pprevb,
				   cob,
				   &obo,
				   opty))) {
		dpr("band_op(ol): prevb(%d, %d) cob(%d, %d)\n",
		    (*pprevb)->s, (*pprevb)->b,
		    cob->s, cob->b);
		*pprevb = cob;
		cob++;
	}
	/* return bottom rect band for next iteration. */
	*botb0 = (bs_b0 && (YBAND_SUBSET == bpr))
		|| (!bs_b0 && (YBAND_SUBSET != bpr));
	return (u32)(cob - outb);
}


/*****************************************************************************
 *
 * Rect Band
 *
 *****************************************************************************/
/*
 * TODO: This is NOT optimal!
 * Maximum # of band-buffer required as output, is
 *         (Never fails!)
 */
static inline u32
rband_max_requred_outbands(u32 rb0sz, u32 rb1sz, enum optype opty) {
	u32 maxsz = yut_max(rb0sz, rb1sz);
	/*
	 * Worstcase (required maximum number of bands) at overlapping band is,
	 * all x-bands are separated.
	 * If there are bands overlapping, they can be merged.
	 * So, 'sz0 + sz1' can be maximum # for overlapping band.
	 */
	/* +1 for region ending-sentinel */
	return maxsz * 2 /* Two no-overlap bands */
		+ rb0sz + rb1sz + 1; /* overlap bands */
}


/*
 * 'rbs' and 'rbb' should follows below rules
 * - rbs.s <= rbb.s
 * - They are separated (No-overlap)
 */
static bool
rband_can_merge(struct rectband *rbs,
		const struct yband *yb,
		const struct yband *xbs) {
	const struct yband *b0, *b1;
	yassert(!ygeob_is_empty(&rbs->yb)
		&& !ygeob_is_empty(yb)
		&& !ygeob_is_empty(xbs)
		&& rbs->yb.s < yb->s
		&& rbs->yb.b <= yb->s);
	if (rbs->yb.b < yb->s)
		return FALSE;
	b0 = rbs->xbs;
	b1 = xbs;
	while (!ygeob_is_empty(b0)
	       && !ygeob_is_empty(b1)) {
		if (!ygeob_eq(b0++, b1++))
			return FALSE;
	}
	if (!ygeob_is_empty(b0)
	    || !ygeob_is_empty(b1))
		return FALSE;
	return TRUE;
}

/*
 * @return # of bands added to 'outb' (0 or at least 2;"yb + xb")
 *         > 0 means one 'rectband' is added to 'outb'
 *         (Never fails!)
 */
static u32
rband_add(struct rectband *prevrb,
	  struct yband *outb,
	  const struct yband *yb,
	  const struct yband *xbs) {
	struct yband *p;
	if (unlikely(ygeob_is_empty(yb)))
		return 0;
	yassert(!ygeob_is_empty(xbs)
		&& !ygeob_is_empty(&prevrb->yb));
	if (rband_can_merge(prevrb, yb, xbs)) {
		prevrb->yb.b = yb->b; /* merge */
		return 0;
	}
	*outb = *yb;
	p = outb + 1; /* xbs */
	while (!ygeob_is_empty(xbs))
		*p++ = *xbs++;
	ygeob_set_empty(p++);
	yassert(p > outb + 1);
	return p - outb;
}


/*
 * @param yb y-band
 * @param xbs array of x-bands ends with empty band.
 * @return # of bands added to 'outb' (0 or at least 2;"yb + xb")
 *         > 0 means one 'rectband' is added to 'outb'
 *         (Never fails!)
 */
static inline u32
rband_op_no_overlap(struct rectband *prevrb,
		    struct yband *outb,
		    const struct yband *yb,
		    const struct yband *xbs,
		    enum optype opty,
		    bool b_lho) {
	/* top band has meaning only in following cases */
	if (OP_UNION == opty
	    || (OP_DIFF == opty
		&& b_lho))
		return rband_add(prevrb, outb, yb, xbs);
	return 0;
}

static u32
rband_op_overlap(struct rectband *prevrb,
		 struct yband *outb,
		 const struct yband *yb,
		 const struct yband *xbs0,
		 const struct yband *xbs1,
		 enum optype opty) {
	u32 nrb; /* # of bands */
	struct yband *prevxb; /* PREVious X Band */
	struct yband oxbb0, oxbb1;
	/* Current/Next Out X Bottom Band */
	struct yband *pcoxbb, *pnoxbb, *pbtmp;
	const struct yband *pb0, *pb1, *pcxb0, *pcxb1;
	struct yband *cob;

	if (ygeob_is_empty(yb))
		return 0;

	pb0 = pcxb0 = xbs0; /* initial temporal assignment */
	pb1 = pcxb1 = xbs1;
	pcoxbb = &oxbb0;
	pnoxbb = &oxbb1;
	/* First x-band MUST NOT be empty */
	prevxb = (struct yband *)&band_min_invalid;
	cob = outb;
	/* Set y-band value and move to x-bands.
	 * Several x-bands may be added.
	 */
	*cob++ = *yb;
	while (!(ygeob_is_empty(pcxb0)
		 && ygeob_is_empty(pcxb1))) {
		bool botb0;
		dpr(">> rband_op overlap loop:\n"
		    "    pb0(%d, %d), pb1(%d, %d)\n"
		    "    pcxb0(%d, %d), pcxb1(%d, %d)\n",
		    pb0->s, pb0->b, pb1->s, pb1->b,
		    pcxb0->s, pcxb0->b, pcxb1->s, pcxb1->b);
		/* Max invalid band has possible-maximum-value.
		 * So, it always remains as bottom band.
		 * That means, pointer to band_max_invalid never move to next
		 * invalid-band(invalid memory address).
		 * So, it is safe in this loop.
		 */
		if (unlikely(ygeob_is_empty(pb0)))
			pb0 = (struct yband *)&band_max_invalid;
		if (unlikely(ygeob_is_empty(pb1)))
			pb1 = (struct yband *)&band_max_invalid;
		nrb = band_op(cob, &botb0, pnoxbb, &prevxb,
			      pb0, pb1, opty);
		dpr(">- bot:%d, nrb:%d\n", botb0, nrb);
		cob += nrb;
		if (unlikely(ygeob_is_empty(pnoxbb))) {
			pb0 = ++pcxb0;
			pb1 = ++pcxb1;
		} else {
			if (botb0) {
				/* bottom of pb0 should be reused */
				pb0 = pnoxbb;
				pb1 = ++pcxb1; /* pb1 SHOULD move to next */
			} else {
				/* bottom of pb1 should be reused */
				pb0 = ++pcxb0;
				pb1 = pnoxbb;
			}
		}
		/* swap oxbb buffer */
		pbtmp = pcoxbb;
		pcoxbb = pnoxbb;
		pnoxbb = pbtmp;
	}
	/* first invalid band MUST NOT be used at merge */
	assert_band_min_invalid();

	/* if there is no x-band, entire region should be removed */
	yassert(cob > outb);
	if (cob - outb == 1)
		/* There is no x-bands. So, we don't need to keep 'yb' */
		return 0;
	else {
		ygeob_set_empty(cob++);
		/* New rect band is created. We can merge them? */
		if (rband_can_merge(prevrb, outb, outb + 1)) {
			prevrb->yb.b = outb->b; /* merge! */
			return 0;
		}
	}
	return cob - outb;
}

/*
 * @return # of bands added to 'outb'
 *         > 0 means some 'rectband' is added to 'outb'
 *         (Never fails!)
 */
static u32
rband_op(struct yband *outb, /* assumes that it has enough memory */
	 bool *botrb0, /* bottom band from rb0 */
	 struct yband *pyobb, /* (Out) */
	 struct rectband **pprevrb, /* for merging */
	 const struct yband *yb0,
	 const struct yband *xbs0,
	 const struct yband *yb1,
	 const struct yband *xbs1,
	 enum optype opty) {
	/* Function variables */
	enum ybandpos ybpr;
	struct yband obs, obo, obb;
	const struct yband *ybs, *ybb; /* Y Band Small/Big */
	const struct yband *xbss; /* X Bands matching ybs */
	/* const struct yband *xbsb; * X Bands matching ybb */
	u32 nrb; /* # of bands */
	struct yband *cob;
	bool rbys_rb0;

	dpr("rband: yb0(%d, %d), xbs0(%d, %d)\n"
	    "       yb1(%d, %d), xbs1(%d, %d)\n",
	    yb0->s, yb0->b, xbs0->s, xbs0->b,
	    yb1->s, yb1->b, xbs1->s, xbs1->b);

	cob = outb; /* Current Out Band */
	if (yb0->s < yb1->s) {
		ybs = yb0;
		xbss = xbs0;
		ybb = yb1;
		/* xbsb = xbs1; */
		rbys_rb0 = TRUE;
	} else {
		ybs = yb1;
		xbss = xbs1;
		ybb = yb0;
		/* xbsb = xbs0; */
		rbys_rb0 = FALSE;
	}

	ybpr = ygeob_analyze_layout(&obs, &obo, &obb, ybs, ybb);

	/* ==========================
	 * top rect band - no overlap
	 * ==========================
	 */
	if ((nrb = rband_op_no_overlap(*pprevrb,
				       cob,
				       &obs,
				       xbss,
				       opty,
				       rbys_rb0))) {
		*pprevrb = bands2rband(cob);
		cob += nrb;
	}

	/* ===================================
	 * overlapping rect band in the middle
	 * ===================================
	 */
	if ((nrb = rband_op_overlap(*pprevrb,
				    cob,
				    &obo,
				    xbs0,
				    xbs1,
				    opty))) {
		*pprevrb = bands2rband(cob);
		cob += nrb;
	}

	/* return bottom rect band for next iteration. */
	*pyobb = obb;
	*botrb0 = (rbys_rb0 && (YBAND_SUBSET == ybpr))
		|| (!rbys_rb0 && (YBAND_SUBSET != ybpr));
	return (u32)(cob - outb);
}

/*****************************************************************************
 *
 * Region
 *
 *****************************************************************************/
static inline bool
rgn_is_empty(const struct yrgn *g) {
	/* check whether first band is empty or not */
	return ygeob_is_empty(g->bs);
}

/*
 * @return NULL if fails(usually ENOMEM)
 */
static struct yrgn *
rgn_op(const struct yrgn *g0, const struct yrgn *g1, enum optype opty) {
	struct ydynb *dynb;
	u32 bsz, rb0sz, rb1sz;
	const struct rectband *rb0, *rb1;
	struct rectband *prevrb;
	const struct yband *yb0, *yb1, *xbs0, *xbs1;
	struct yband oybb0, oybb1;
	/* Current/Next Out X Bottom Band */
	struct yband *pcoybb, *pnoybb, *pbtmp;
	dynb = ydynb_create2(4096 / sizeof(struct yband),
			     sizeof(struct yband));
	if (unlikely(!dynb))
		return NULL;

#define var_init_define(N)		\
	rb##N = rgn2rband(g##N);	\
	yb##N = &rb##N->yb;		\
	xbs##N = rb##N->xbs;		\
	rb##N##sz = rband_bsz(rb##N);

	var_init_define(0);
	var_init_define(1);
#undef var_init_define

	pcoybb = &oybb0;
	pnoybb = &oybb1;
	prevrb = (struct rectband *)rband_min_invalid;
	while (!(ygeog_is_empty(rband2rgn(rb0))
		 && ygeog_is_empty(rband2rgn(rb1)))) {
		bool botrb0;
		bsz = rband_max_requred_outbands(rb0sz,
						 rb1sz,
						 opty);
		/* +1 for region ending sentinel */
		if (ydynb_expand2(dynb, bsz + 1))
			goto fail;
		/* Now we have enough memory */

		/* Max invalid band has possible-maximum-band-value.
		 * So, it always remains as bottom band.
		 * That means, pointer to band_max_invalid never move to next
		 * invalid-band(invalid memory address).
		 * So, it is safe in this loop.
		 */
#define rb_empty_check(N)						\
		if (unlikely(ygeog_is_empty(rband2rgn(rb##N)))) {	\
			yb##N = &band_max_invalid;			\
			xbs##N = &rband_max_invalid[1];			\
		}

		rb_empty_check(0);
		rb_empty_check(1);
#undef rb_empty_check

		bsz = rband_op(ydynb_getfree(dynb),
			       &botrb0,
			       pnoybb,
			       &prevrb,
			       yb0, xbs0,
			       yb1, xbs1,
			       opty);
		ydynb_incsz(dynb, bsz);

#define rb_move_to_next(N) 						\
	do {								\
		rb##N = bands2rband(rband2bands(rb##N) + rb##N##sz); 	\
		yb##N = &rb##N->yb;					\
		xbs##N = rb##N->xbs;					\
		rb##N##sz = rband_bsz(rb##N); 				\
	} while (0)

		if (unlikely(ygeob_is_empty(pnoybb))) {
			/* Both bands are consumed */
			rb_move_to_next(0);
			rb_move_to_next(1);
		} else {
			if (botrb0) {
				/* bottom of pb0 should be reused */
				yb0 = pnoybb;
				rb_move_to_next(1);
			} else {
				/* bottom of pb1 should be reused */
				rb_move_to_next(0);
				yb1 = pnoybb;
			}
		}
#undef rb_move_to_next

		/* swap oybb buffer */
		pbtmp = pcoybb;
		pcoybb = pnoybb;
		pnoybb = pbtmp;
	}
	assert_rband_min_invalid();
	/* add ending sentinel */
	yassert(ydynb_freesz(dynb) > 0);
	ygeob_set_empty(ydynb_getfree(dynb));
	ydynb_incsz(dynb, 1);
	/* Ignoring return value intentionally.
	 * We can use as it is even if shrinking fails.
	 */
	ydynb_shrink(dynb, ydynb_sz(dynb));
	return (struct yrgn *)ydynb_destroy(dynb, TRUE);

 fail:
	ydynb_destroy(dynb, FALSE);
	return NULL;
}

/*****************************************************************************
 *
 * Region - interfaces
 *
 *****************************************************************************/
struct yrgn *
ygeog_clone(const struct yrgn *g) {
	u32 sz = rgn_bsz(g) * sizeof(struct yband);
	struct yband *b = ymalloc(sz);
	if (unlikely(!b))
		return NULL;
	memcpy(b, g, sz);
	return bands2rgn(b);
}

struct yrgn *
ygeog_create_empty(void) {
	struct yband *b = ymalloc(sizeof(struct yband));
	if (unlikely(!b))
		return NULL;
	ygeob_set_empty(b);
	return bands2rgn(b);
}

bool
ygeog_is_empty(const struct yrgn *rgn) {
	return rgn_is_empty(rgn);
}

void
ygeog_destroy(struct yrgn *rgn) {
	yfree(rgn);
}

struct yrgn *
ygeog_intersect(const struct yrgn *g0, const struct yrgn *g1) {
	return rgn_op(g0, g1, OP_INTERSECT);
}

struct yrgn *
ygeog_intersect2(const struct yrgn *g, const struct yrect *r) {
	struct yband tmp[4];
	mutate_rect_to_rgnbands(tmp, r);
	return ygeog_intersect(g, bands2rgn(tmp));
}

struct yrgn *
ygeog_diff(const struct yrgn *g0, const struct yrgn *g1) {
	return rgn_op(g0, g1, OP_DIFF);
}

struct yrgn *
ygeog_diff2(const struct yrgn *g, const struct yrect *r) {
	struct yband tmp[4];
	mutate_rect_to_rgnbands(tmp, r);
	return ygeog_diff(g, bands2rgn(tmp));
}

struct yrgn *
ygeog_diff3(const struct yrect *r0, const struct yrect *r1) {
	struct yband tmp[4];
	mutate_rect_to_rgnbands(tmp, r0);
	return ygeog_diff2((struct yrgn *)tmp, r1);
}

struct yrgn *
ygeog_union(const struct yrgn *g0, const struct yrgn *g1) {
	return rgn_op(g0, g1, OP_UNION);
}

struct yrgn *
ygeog_union2(const struct yrgn *g, const struct yrect *r) {
	struct yband tmp[4];
	mutate_rect_to_rgnbands(tmp, r);
	return ygeog_union(g, bands2rgn(tmp));
}

struct yrgn *
ygeog_union3(const struct yrect *r0, const struct yrect *r1) {
	struct yband tmp[4];
	mutate_rect_to_rgnbands(tmp, r0);
	return ygeog_union2((struct yrgn *)tmp, r1);
}
