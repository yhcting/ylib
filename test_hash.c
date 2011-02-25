#include <stdio.h>
#include <string.h>

#include "yhash.h"
#include "common.h"

#ifdef _YDBG

#include <assert.h>

extern int dmem_count();

static void
_free(void* v) {
	yfree(v);
}

void
test_hash() {
	int           sv = dmem_count();
	int           i;
	char          buf[4096];
	char*         v;
	struct yhash* h = yhash_create(&_free);

	for (i=0; i<1024; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = ymalloc(strlen(buf)+1);
		strcpy(v, buf);
		/* key and value is same */
		yhash_add(h, buf, strlen(buf)+1, v);
		yassert(i+1 == yhash_sz(h));
	}

	for (i=256; i<512; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = yhash_find(h, buf, strlen(buf)+1);
		yassert(v && 0 == strcmp(v, buf));
	}

	for (i=1023; i>=0; i--) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		yhash_del(h, buf, strlen(buf)+1);
		yassert(i == yhash_sz(h));
	}

	yhash_destroy(h);

	yassert(sv == dmem_count());
}

#endif /* _YDBG */
