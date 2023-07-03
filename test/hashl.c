/******************************************************************************
 * Copyright (C) 2021
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
#ifdef CONFIG_TEST

#include <string.h>

#include "yhashl.h"

struct item {
	struct yhashl_node hn;
	int v;
};

static void
test_hashl(void) {
	int i;
	struct yhashl_node *rn, *tmp;
	struct item *itm;
	char sbuf[128];
	struct yhashl *h = ymalloc(sizeof(*h));
	yhashl_init(h, YHASHL_HFUNC_STR, YHASHL_KEYEQ_STR);
	for (i = 0; i < 1024; i++) {
		itm = ymalloc(sizeof(*itm));
		sprintf(sbuf, "item-%d", i);
		itm->v = i;
		rn = yhashl_set(h, ystrdup(sbuf), &itm->hn);
		yassert(!rn);
	}
	rn = yhashl_get(h, "item-10");
	yassert(rn && 10 == containerof(rn, struct item, hn)->v);

	itm = ymalloc(sizeof(*itm));
	itm->v = -1;
	rn = yhashl_set(h, "item-10", &itm->hn);
	yassert(rn);
	yfree(containerof(rn, struct item, hn));
	rn = yhashl_get(h, "item-10");
	yassert(rn && -1 == containerof(rn, struct item, hn)->v);

	yhashl_foreach_safe(h, rn, tmp) {
		yfree((void *)yhashl_node_key(rn));
		yfree(containerof(rn, struct item, hn));
	}
	yhashl_clean(h);
	yfree(h);
}

TESTFN(hashl)

#endif /* CONFIG_TEST */
