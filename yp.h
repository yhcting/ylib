#ifndef __Yp_h__
#define __Yp_h__

#include "ydef.h"

/*****************************************************************************
 *
 * Simple Smart Pointer.
 *
 *****************************************************************************/

/*
 * should be align with pointer size.
 * And, sizeof(long) == sizeof(void *) in general.
 *
 * Memory layout is
 * +-----------------+--------------+--------------
 * |   sizeof(long)  | sizeof(long) | user memory
 * | reference count | magic number | area
 * +-----------------+--------------+--------------
 *
 * NOTE : 'Magic Number' slot exists only with CONFIG_DEBUG
 */

#ifdef CONFIG_DEBUG

#define YP_MAGIC 0xfaebcddcL

static inline void *
ypmalloc(unsigned int sz) {
	long *p;
	p = (long *)ymalloc(sz + sizeof(long) * 2);
	*p++ = 0;
	*p++ = YP_MAGIC;
	return (void *)p;
}

#else /* CONFIG_DEBUG */

static inline void *
ypmalloc(unsigned int sz) {
	long *p;
	p = (long *)ymalloc(sz + sizeof(long));
	*p++ = 0;
	return (void *)p;
}


#endif /* CONFIG_DEBUG */

/* SP Put */
static inline void
ypput(void *v) {
	long *p = (long *)v;
	yassert(YP_MAGIC == *--p);
	--*--p;
	yassert(0 <= *p);
	if (unlikely(*p <= 0))
		yfree(p);
}

/* SP Get */
static inline void *
ypget(void *v) {
	long *p = (long *)v;
	yassert(YP_MAGIC == *--p);
	++*--p;
	return v;
}


#endif /* __Yp_h__ */
