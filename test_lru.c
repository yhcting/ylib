#include <string.h>
#include <assert.h>

#include "common.h"
#include "ylru.h"
#include "test.h"


static void
vfree(void *v) {
	yfree(v);
}

static void *
vcreate(unsigned int *data_size,
	const void *key, unsigned int keysz) {
	int *i;
	*data_size = sizeof(int);
	i = ymalloc(sizeof(*i));
	*i = 10;
	return i;
}

static void
test_lru(void) {
	int *pi __attribute__((unused));
	struct ylru_cb cbs = {
		.free = &vfree,
		.create = NULL
	};
	struct ylru *lru = ylru_create(sizeof(int) * 3, &cbs);

	pi = (int *)ylru_get(lru, NULL, "key0", 5);
	yassert(!pi);

	pi = ymalloc(sizeof(*pi));
	*pi = 100;
	ylru_put(lru, "k100", 5, pi, sizeof(*pi));
	yassert(sizeof(int) == ylru_sz(lru));

	pi = (int *)ylru_get(lru, NULL, "k100", 5);
	yassert(100 == *pi);
	yassert(0 == ylru_sz(lru));
	ylru_put(lru, "k100", 5, pi, sizeof(*pi));

	pi = ymalloc(sizeof(*pi));
	*pi = 200;
	ylru_put(lru, "k200", 5, pi, sizeof(*pi));
	yassert(sizeof(int) * 2 == ylru_sz(lru));

	pi = ymalloc(sizeof(*pi));
	*pi = 300;
	ylru_put(lru, "k300", 5, pi, sizeof(*pi));
	yassert(sizeof(int) * 3 == ylru_sz(lru));

	pi = ymalloc(sizeof(*pi));
	*pi = 400;
	ylru_put(lru, "k400", 5, pi, sizeof(*pi));
	yassert(sizeof(int) * 3 == ylru_sz(lru));

	/* Now lru is [ 200 - 300 - 400 (newest) ] */

	pi = (int *)ylru_get(lru, NULL, "k100", 5);
	yassert(!pi);
	pi = (int *)ylru_get(lru, NULL, "k200", 5);
	yassert(200 == *pi);
	yassert(sizeof(int) * 2 == ylru_sz(lru));
	ylru_put(lru, "k200", 5, pi, sizeof(*pi));

	/* Now, [ 300 - 400 - 200 (newest) ] */

	pi = ymalloc(sizeof(*pi));
	*pi = 500;
	ylru_put(lru, "k500", 5, pi, sizeof(*pi));
	yassert(sizeof(int) * 3 == ylru_sz(lru));

	/* Should be [ 400 - 200 - 500 ] */

	pi = (int *)ylru_get(lru, NULL, "k300", 5);
	yassert(!pi);

	ylru_clean(lru);
	ylru_destroy(lru);

	cbs.create = &vcreate;
	lru = ylru_create(sizeof(int) * 3, &cbs);
	pi = (int *)ylru_get(lru, NULL, "k000", 5);
	yassert(pi);
	yassert(10 == *pi);
	ylru_put(lru, "k000", 5, pi, sizeof(*pi));
	ylru_destroy(lru);
}


TESTFN(test_lru, lru)