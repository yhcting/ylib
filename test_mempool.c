#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "ymempool.h"
#include "common.h"

#include <assert.h>

#define _TESTSZ    (1*1024)

void
test_mempool() {
	int         i, j, k;
	struct ymp* mp;
	int*        b[_TESTSZ];
	mp = ymp_create(1, sizeof(int));

	/*
	 * Normal Test...
	 */
	b[0] = ymp_get(mp);
	ymp_put(mp, b[0]);

	b[0] = ymp_get(mp);
	b[1] = ymp_get(mp);
	ymp_put(mp, b[0]);
	ymp_put(mp, b[1]);

	for (i = 0; i < _TESTSZ; i++) {
		b[i] = ymp_get(mp);
		*b[i] = i;
	}

	for (i = 0; i < _TESTSZ / 2; i++)
		ymp_put(mp, b[i]);

	for (i = 0; i < _TESTSZ / 2; i++) {
		b[i] = ymp_get(mp);
		*b[i] = i;
	}

	for (i = _TESTSZ / 2; i < _TESTSZ; i++)
		ymp_put(mp, b[i]);

	for (i = 0; i < _TESTSZ / 2; i++) {
		ymp_put(mp, b[i]);
		b[i] = ymp_get(mp);
		*b[i] = i;
	}

	for (i = _TESTSZ / 2; i < _TESTSZ; i++) {
		b[i] = ymp_get(mp);
		*b[i] = i;
	}

	for (i = 0; i < _TESTSZ; i++) {
		assert(*b[i] == i);
	}

	ymp_destroy(mp);

	/*
	 * Defragmentation Test
	 */

#define __MAGICV     0x12345678
#define __POISONV    0xdeaddead /* poision value */

	mp = ymp_create(1, sizeof(int));

	/* initialize */
	for (i = 0; i < _TESTSZ; i++) {
		b[i] = ymp_get(mp);
		*b[i] = __MAGICV; /* magic value */
	}

	/*
	 * put randomly selected blocks and get some of them again repeatedly.
	 * (To make fragmentation)
	 */
	srand(time(NULL));

	for (i = 0; i < 100; i++) {
		/* nr is number of blocks that is put from pool */
		for (j = 0; j < _TESTSZ / 2; j++) {
			k = rand() % _TESTSZ;
			if (b[k]) {
				*b[k] = __POISONV;
				ymp_put(mp, b[k]);
				b[k] = NULL;
			}
		}

		/* get again */
		for (j = 0; j < _TESTSZ; j++) {
			if (!b[j]) {
				b[j] = ymp_get(mp);
				*b[j] = __MAGICV;
			}
		}
	}

	/* free enough blocks to prepare defragmentation */
	for (i = 0; i < _TESTSZ / 4 * 3; i++) {
		*b[i] = __POISONV;
		ymp_put(mp, b[i]);
		b[i] = NULL;
	}

	/* call shrink here! */
	ymp_shrink(mp);
	{ /* just scope */
		int nr = 0;
		/* verify values */
		for (i = 0; i < _TESTSZ; i++) {
			if (b[i]) {
				nr++;
				assert(__MAGICV == *b[i]);
			}
		}
		assert(nr == ymp_sz(mp));
	} /* just scope */
	ymp_destroy(mp);

#undef __POISONV
#undef __MAGICV

}
