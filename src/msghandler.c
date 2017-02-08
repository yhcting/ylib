/******************************************************************************
 * Copyright (C) 2016
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
#include <errno.h>

#include "common.h"
#include "msg.h"
#include "ylog.h"
#include "ymsgq.h"
#include "ymsglooper.h"
#include "msghandler.h"

static void
default_handle(struct ymsghandler *handler __unused,
	       const struct ymsg *m) {
	if (unlikely(YMSG_TYP_EXEC != m->type
		     || !m->run)) {
		ylogw("Invalid message(default handler): type:%d, %p => ignored\n,",
		      m->type, m->run);
		return;
	}
	(*m->run)(m->data);
}

struct ymsghandler *
ymsghandler_create(struct ymsglooper *ml,
		   void *tag,
		   void (*tagfree)(void *),
		   void (*handle)(struct ymsghandler *, const struct ymsg *)) {
	struct ymsghandler *mh;
	if (unlikely(!ml))
		return NULL;
	mh = ymalloc(sizeof(*mh));
	if (unlikely(!mh))
		return NULL;
	mh->ml = ml;
	if (!handle)
		handle = &default_handle;
	mh->tag = tag;
	mh->tagfree = tagfree;
	mh->handle = handle;
	return mh;
}

void
ymsghandler_destroy(struct ymsghandler *mh) {
	if (mh->tag && mh->tagfree)
		(*mh->tagfree)(mh->tag);
	if (likely(mh))
		yfree(mh);
}

void *
ymsghandler_get_tag(struct ymsghandler *mh) {
	return mh->tag;
}

struct ymsglooper *
ymsghandler_get_looper(struct ymsghandler *mh) {
	return mh->ml;
}

int
ymsghandler_post_data(struct ymsghandler *mh,
		      int code, void *data,
		      void (*dfree)(void *)) {
	return ymsghandler_post_data2(mh,
				      code,
				      data,
				      dfree,
				      YMSG_PRI_NORMAL,
				      0);
}

int
ymsghandler_post_data2(struct ymsghandler *mh,
		       int code, void *data,
		       void (*dfree)(void *),
		       uint8_t pri, uint32_t opt) {

	struct ymsg *m = ymsg_create();
	if (unlikely(!m))
		return -ENOMEM;
	ymsg_set_data(m, pri, opt, code, data, dfree);
	msg_set_handler(msg_mutate(m), mh);
	return ymsgq_en(ymsglooper_get_msgq(mh->ml), m);
}

int
ymsghandler_post_exec(struct ymsghandler *mh,
		      void *arg, void (*argfree)(void *),
		      void (*run)(void *)) {
	return ymsghandler_post_exec2(mh,
				      arg,
				      argfree,
				      run,
				      YMSG_PRI_NORMAL,
				      0);
}

int
ymsghandler_post_exec2(struct ymsghandler *mh,
		       void *arg, void (*argfree)(void *),
		       void (*run)(void *),
		       uint8_t pri, uint32_t opt) {
	struct ymsg *m = ymsg_create();
	if (unlikely(!m))
		return -ENOMEM;
	ymsg_set_exec(m, pri, opt, arg, argfree, run);
	msg_set_handler(msg_mutate(m), mh);
	return ymsgq_en(ymsglooper_get_msgq(mh->ml), m);
}

int
ymsghandler_exec_on(struct ymsghandler *mh,
		    void *arg, void (*argfree)(void *),
		    void (*run)(void *)) {
	if (ymsglooper_get_thread(ymsghandler_get_looper(mh))
	    == pthread_self()) {
		(*run)(arg);
		if (arg && argfree)
			(*argfree)(arg);
		return 0;
	}
	return ymsghandler_post_exec(mh, arg, argfree, run);
}


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
#ifdef CONFIG_DEBUG
/*
 * This function is used for testing and debugging.
 */
extern void msg_clear(void);
void
msghandler_clear(void) {
	msg_clear();
}
#endif /* CONFIG_DEBUG */
