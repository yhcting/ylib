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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "ylib.h"
#include "ylog.h"
#include "common.h"
#include "ylistl.h"

struct tstfn {
	void (*fn)(void);
	void (*clear)(void);
	const char *modname;
	struct ylistl_link lk;
};

struct memblk {
	int line;
	const char *file;
	int sz;
	struct ylistl_link lk;
	uintptr_t blk[0];
};

static YLISTL_DEFINE_HEAD(_tstfnl);
static YLISTL_DEFINE_HEAD(_memhd);
static int _mem_count = 0;
static pthread_mutex_t _mem_lock = PTHREAD_MUTEX_INITIALIZER;

static void
init_mem_lock(void) {
	pthread_mutex_init(&_mem_lock, NULL);
}

static void
destroy_mem_lock(void) {
	pthread_mutex_destroy(&_mem_lock);
}

static void
lock_mem(void) {
	pthread_mutex_lock(&_mem_lock);
}

static void
unlock_mem(void) {
	pthread_mutex_unlock(&_mem_lock);
}

static void
add_mem_locked(struct memblk *m) {
	_mem_count++;
	ylistl_add_first(&_memhd, &m->lk);
}

static void
rm_mem_locked(struct memblk *m) {
	_mem_count--;
	ylistl_remove(&m->lk);
}

static void
clear_mem(void) {
	struct memblk *pos, *n;
	ylistl_foreach_item_safe(pos, n, &_memhd, struct memblk, lk) {
		ylistl_remove(&pos->lk);
		free(pos);
	}
}

static void
print_mem(void) {
	struct memblk *pos;
	printf("%20s %10s, %10s\n", "File", "Line", "Size");
	ylistl_foreach_item(pos, &_memhd, struct memblk, lk) {
		printf("%20s %10d, %10d\n",
		       pos->file,
		       pos->line,
		       pos->sz);
	}
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
void
dregister_tstfn(void (*fn)(void), const char *mod) {
	/* malloc should be used instead of dmalloc */
	struct tstfn* n = malloc(sizeof(*n));
	n->fn = fn;
	n->clear = NULL;
	n->modname = mod;
	ylistl_add_last(&_tstfnl, &n->lk);
}

void
dunregister_tstfn(void (*fn)(void), const char *mod) {
	struct tstfn *p, *n;
	ylistl_foreach_item_safe(p, n, &_tstfnl, struct tstfn, lk) {
		if (p->fn == fn && !strcmp(p->modname, mod)) {
			ylistl_remove(&p->lk);
			free(p);
		}
	}
}

void
dregister_clearfn(void (*fn)(void), const char *mod) {
	struct tstfn *p;
	ylistl_foreach_item(p, &_tstfnl, struct tstfn, lk) {
		if (!strcmp(p->modname, mod)) {
			p->clear = fn;
			return;
		}
	}
	fprintf(stderr, "Unknown module name for clear function: %s\n", mod);
}

void *
dmalloc(size_t sz, const char *file, int line) {
	struct memblk *m;
	m = malloc(sizeof(struct memblk) + sz);
	m->sz = (int)sz;
	m->file = file;
	m->line = line;
	lock_mem();
	add_mem_locked(m);
	unlock_mem();
	return &m->blk[0];
}

void *
drealloc(void *p, size_t sz, const char *file, int line) {
	struct memblk *m = YYcontainerof(p, struct memblk, blk);
	void *newp;
	newp = dmalloc(sz, file, line);
	memcpy(newp, p, sz > m->sz? m->sz: sz);
	dfree(p);
	return newp;
}

void *
dcalloc(size_t n, size_t sz, const char *file, int line) {
	void *m = dmalloc(sz * n, file, line);
	memset(m, 0, sz * n);
	return m;
}

void
dfree(void *p) {
	struct memblk *m;
	m = YYcontainerof(p, struct memblk, blk);
	lock_mem();
	rm_mem_locked(m);
	unlock_mem();
	free(m);
}

char *
dstrdup(const char *s, const char *file, int lineno) {
	size_t len = strlen(s);
	char *d = dmalloc(len + 1, file, lineno);
	if (unlikely(!d)) {
		errno = ENOMEM;
		return NULL;
	}
	memcpy(d, s, len + 1);
	return d;
}

int
dmem_count(void) {
	return _mem_count;
}


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
#ifdef CONFIG_DEBUG
struct opt {
	const char *mods[1024]; /* 1024 is large enough value */
	int repeat_cnt;
	int loglv;
};

static void
print_usage(const char *cmd) {
	printf(
"Usage: %s [options] [module ...]\n"
"\n"
"OPTIONS\n"
"    -h\n"
"        show this text.\n"
"    -c repeat-count: Default 5\n"
"        how many times test is repeated per module.\n"
"        if < 1 or invalid-value then 1 is set.\n"
"    -l log-level: Default 4\n"
"        0(verbose) ~ 4(err) ~ 6(disable).\n"
"        if < 0 or invalid-value then 0, and > 6 then 6 is set.\n",
	cmd);
}

static int
parse_optarg(struct opt *opt, int argc, char **argv) {
	int c;
	int i;
	const char **pmod;
	opterr = 0;
	while (-1 != (c = getopt(argc, argv, "hc:l:"))) {
		switch (c) {
		case 'h':
			print_usage("y");
			exit(EXIT_SUCCESS);
		case 'c': {
			int v = atoi(optarg);
			if (v < 1)
				v = 1;
			opt->repeat_cnt = v;
		} break;
		case 'l': {
			int v = atoi(optarg);
			if (v < 0)
				v = 0;
			if (v > 6)
				v = 6;
			opt->loglv = v;
		} break;
		case '?':
			if (isprint (optopt))
				fprintf(stderr,
					"Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr,
					"Unknown option character `\\x%x'.\n",
					optopt);
			return -1;
		default:
			abort ();
		}
	}
	pmod = &opt->mods[0];
	for (i = optind; i < argc; i++)
		*pmod++ = argv[i];
	*pmod = NULL;
	return 0;
}

static int
test_mod(struct tstfn *tf, int count) {
	int sv;
	printf("<< Test [%s] >>\n", tf->modname);
	sv = dmem_count();
	/* Repeat same test several times to detect memory corruption
	 * in each test.
	 * This is useful to avoid following case.
	 *
	 * [ Example ]
	 * - Test order : modA -> modB
	 * - Memory is corrupted at test-modA, especially at the end of
	 * test.
	 * - Test may fail at modB because of memory corruption caused
	 * at test-modA
	 * - This result is NOT good to understand.
	 */
	while (count--) {
		(*tf->fn)();
		if (tf->clear)
			(*tf->clear)();
	}
	if (sv != dmem_count()) {
		print_mem();
		fprintf(stderr,
			"Unbalanced memory at [%s]!\n"
			"    balance : %d\n",
			tf->modname,
			dmem_count() - sv);
		return -1;
		/* yassert(0); */
	}
	printf(" => PASSED\n");
	return 0;
}

int
main(int argc, char *argv[]) {
	struct opt opt;
	const char **pmod;

	/* Set default values */
	memset(&opt, 0, sizeof(opt));
	opt.repeat_cnt = 5;
	opt.loglv = 4; /* YLOG_ERR */

	if (parse_optarg(&opt, argc, argv))
		return -1;

	init_mem_lock();

	struct ylib_config yc;
	memset(&yc, 0, sizeof(yc));
	yc.ylog_stdfd = yc.ylog_errfd = -1;
	yc.ylog_level = opt.loglv;
	ylib_init(&yc);
	/* This mechanism is ineffective but simple - O(n^2).
	 * If performance becomes matter, use hashmap!
	 */
	/* if module name is NOT specifiec, test all modules */
	if (!opt.mods[0]) {
		struct tstfn *p;
		pmod = &opt.mods[0];
		ylistl_foreach_item(p, &_tstfnl, struct tstfn, lk) {
			*pmod++ = p->modname;
		}
		*pmod = NULL;
	}
	pmod = &opt.mods[0];
	while (*pmod) {
		struct tstfn *tf, *p;
		tf = NULL;
		ylistl_foreach_item(p, &_tstfnl, struct tstfn, lk) {
			if (!strcmp(*pmod, p->modname)) {
				tf = p;
				break;
			}
		}
		if (!tf) {
			fprintf(stderr,
				"Invalid module name: %s\n", *pmod);
			return -1;
		}
		if (test_mod(tf, opt.repeat_cnt)) {
			fprintf(stderr,
				"%s: Test fails\n", *pmod);
			return -1;
		}
		pmod++;
	}
	ylib_exit();
	if (dmem_count()) {
		print_mem();
		fprintf(stderr,
			"Unbalanced memory lib init/exit\n"
		        "    balance : %d\n",
		        dmem_count());
		return -1;
		/* yassert(0); */
	}
	clear_mem();
	destroy_mem_lock();
	printf(">>>>>> TEST PASSED <<<<<<<\n");
	return 0;
}

#else /* CONFIG_DEBUG */
int
main(int argc, const char *argv[]) {
	printf("TEST SKIPPED. THIS IS NOT DEBUG BUILD!\n");
	return 0;
}
#endif /* CONFIG_DEBUG */
