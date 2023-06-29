/******************************************************************************
 * Copyright (C) 2016, 2021
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

#include <assert.h>
#include <unistd.h>

#include "common.h"
#include "ymsglooper.h"


static void
test_msglooper(void) {
	/* create looper thread and stop it */
	struct ymsglooper *ml0 = ymsglooper_start_looper_thread();
	struct ymsglooper *ml1 = ymsglooper_start_looper_thread();

	ymsglooper_stop(ml0);
	ymsglooper_stop(ml1);
	while (!(ymsglooper_get_state(ml0) == YMSGLOOPER_TERMINATED
		&& ymsglooper_get_state(ml1) == YMSGLOOPER_TERMINATED)
	) { usleep(1000 * 50); }
	ymsglooper_destroy(ml0);
	ymsglooper_destroy(ml1);
	usleep(1000 * 500); /* wait until threads are done */

	ml0 = ymsglooper_start_looper_thread();
	ml1 = ymsglooper_start_looper_thread();

	ymsglooper_stop(ml0);
	ymsglooper_stop(ml1);
	yassert(!ymsglooper_get()); /* there is no message looper for this thread. */
	while (!(ymsglooper_get_state(ml0) == YMSGLOOPER_TERMINATED
		&& ymsglooper_get_state(ml1) == YMSGLOOPER_TERMINATED)
	) { usleep(1000 * 50); }
	ymsglooper_destroy(ml0);
	ymsglooper_destroy(ml1);
}

extern void msglooper_clear(void);
static void
clear_msglooper(void) {
	msglooper_clear();
}


TESTFN(msglooper) /* @suppress("Unused static function") */
CLEARFN(msglooper) /* @suppress("Unused static function") */

#endif /* CONFIG_DEBUG */
