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
#include "test.h"
#ifdef CONFIG_DEBUG

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "ymempool.h"

#include <assert.h>

#define TESTSZ (1024)
#define TESTGRPSZ (7)

static void
test_mempool(void) {
	int i, j, k;
	struct ymempool *mp;
	int *b[TESTSZ];

	mp = ymempool_create(TESTGRPSZ, sizeof(int), 0);

	/*
	 * Normal Test...
	 */
	b[0] = ymempool_get(mp);
	ymempool_put(mp, b[0]);

	b[0] = ymempool_get(mp);
	b[1] = ymempool_get(mp);
	ymempool_put(mp, b[0]);
	ymempool_put(mp, b[1]);

	for (i = 0; i < TESTSZ; i++) {
		b[i] = ymempool_get(mp);
		*b[i] = i;
	}

	for (i = 0; i < TESTSZ / 2; i++)
		ymempool_put(mp, b[i]);

	for (i = 0; i < TESTSZ / 2; i++) {
		b[i] = ymempool_get(mp);
		*b[i] = i;
	}

	for (i = TESTSZ / 2; i < TESTSZ; i++)
		ymempool_put(mp, b[i]);

	for (i = 0; i < TESTSZ / 2; i++) {
		ymempool_put(mp, b[i]);
		b[i] = ymempool_get(mp);
		*b[i] = i;
	}

	for (i = TESTSZ / 2; i < TESTSZ; i++) {
		b[i] = ymempool_get(mp);
		*b[i] = i;
	}

	for (i = 0; i < TESTSZ; i++) {
		assert(*b[i] == i);
	}

	ymempool_destroy(mp);

	/*
	 * Defragmentation Test
	 */

#define __MAGICV 0x12345678
#define __POISONV 0xdeaddead /* poision value */

	mp = ymempool_create(TESTGRPSZ, sizeof(int), YMEMPOOL_mt_safe);

	/* initialize */
	for (i = 0; i < TESTSZ; i++) {
		b[i] = ymempool_get(mp);
		*b[i] = __MAGICV; /* magic value */
	}

	/*
	 * put randomly selected blocks and get some of them again repeatedly.
	 * (To make fragmentation)
	 */
	srand(time(NULL));

	for (i = 0; i < 100; i++) {
		/* nr is number of blocks that is put from pool */
		for (j = 0; j < TESTSZ / 2; j++) {
			k = rand() % TESTSZ;
			if (b[k]) {
				*b[k] = __POISONV;
				ymempool_put(mp, b[k]);
				b[k] = NULL;
			}
		}

		/* get again */
		for (j = 0; j < TESTSZ; j++) {
			if (!b[j]) {
				b[j] = ymempool_get(mp);
				*b[j] = __MAGICV;
			}
		}
	}

	/* free enough blocks to prepare defragmentation */
	for (i = 0; i < TESTSZ / 4 * 3; i++) {
		*b[i] = __POISONV;
		ymempool_put(mp, b[i]);
		b[i] = NULL;
	}

#if 0 /* ymempool_shrink is NOT supported at this moment */
	/* call shrink here! */
	ymempool_shrink(mp, 0);
	{ /* just scope */
		int nr = 0;
		/* verify values */
		for (i = 0; i < TESTSZ; i++) {
			if (b[i]) {
				nr++;
				assert(__MAGICV == *b[i]);
			}
		}
		assert(nr == ymempool_usedsz(mp));
	} /* just scope */
#endif
	ymempool_destroy(mp);

#undef __POISONV
#undef __MAGICV
}

TESTFN(test_mempool, mempool)

#endif /* CONFIG_DEBUG */
