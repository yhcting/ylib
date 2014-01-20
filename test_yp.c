#include <string.h>
#include <assert.h>

#include "common.h"
#include "yp.h"
#include "test.h"

static void
test_yp(void) {
	int i;
	int *p, *p2;
	int *o = ypget(ypmalloc(sizeof(int) * 3));
	p2 = ypget(o);
	p = p2;
	for (i = 0; i < 3; i++)
		*p++ = 1;
	/* sp is assigned to another pointer */
	ypput(p2); /* put */
	p = o;
	for (i = 0; i < 3; i++)
		++*p++;
	/*
	 * should be freed at this point
	 */
	ypput(o);
}

TESTFN(test_yp, yp)
