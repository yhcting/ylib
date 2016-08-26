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

#include <malloc.h>
#include <assert.h>
#include <stdlib.h>

#include "ylib.h"


static struct module *_hd = NULL;

/* DO NOT USE 'ylist' for 'ylistl' here.
 * Both are also part of 'ylib'.
 * BEFORE INITialzation, DO NOT USE ANY MODULES!
 */
struct module {
	const char *name;
	int (*init)(const struct ylib_config *);
	void (*exit)(void);
	struct module *next;
};

static void
add_module(struct module *m) {
	m->next = _hd;
	_hd = m;
}

static void
free_modules(void) {
	struct module *m, *n;
	for (m = _hd; m; m = n) {
		n = m->next;
		free(m);
	}
}


void
ylib_register_module(const char *name,
		     int (*init_)(const struct ylib_config *),
		     void (*exit_)(void)) {
	assert(name);
	struct module *m = malloc(sizeof(*m));
	if (!m) {/* Out Of Memory! */
		assert(0);
		exit(EXIT_FAILURE);
	}
	m->name = name;
	m->init = init_;
	m->exit = exit_;
	add_module(m);
}


int
ylib_init(const struct ylib_config *c) {
	struct module *m;
	struct module *lastm = _hd;
	int r = 0;
	for (m = _hd; m; m = m->next) {
		lastm = m;
		if (r = (*m->init)(c))
			goto fail; /* error! stop! */
	}

	return 0;

 fail:
	/* 'init' fails at 'lastm' */
	for (m = _hd; m != lastm; m = m->next)
		(*m->exit)();
	return r;
}


void
ylib_exit(void) {
	struct module *m;
	for (m = _hd; m; m = m->next)
		(*m->exit)();
	free_modules();
}
