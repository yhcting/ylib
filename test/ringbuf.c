/******************************************************************************
 * Copyright (C) 2023
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

#include <stdint.h>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "yringbuf.h"
#include "yut.h"

#define NR_READERS 5
/* 37 is used to test not aligned case */
#define DATA_BYTES 37

static const int NR_SLOT = 5;
static const int WRITE_COUNT = 10000000;
static const int READ_COUNT = 1000000;

struct data {
	unsigned char a[DATA_BYTES];
};

struct st {
	unsigned int rdfail;
};

struct threadarg {
	struct yringbuf *rb;
	struct st st;
};

/**
 * Doing something that takes a little time that is not optimized by compiler
 */
static void
dummy_cost_op() {
#if 0
	int fdr, fdw, bytes;
	char buf[4096];
	fdr = open("/proc/version", O_RDONLY);
	yassert(fdr >= 0);
	bytes = read(fdr, buf, sizeof(buf));
	yassert(bytes > 0);
	close(fdr);
	fdw = open("/dev/null", O_WRONLY);
	yassert(fdw >= 0);
	bytes = write(fdw, buf, bytes);
	yassert(bytes > 0);
	close(fdw);
#else
	return;
#endif
}

static void
set_data(struct data *d, int index) {
	int base = index % (UCHAR_MAX - yut_arrsz(d->a));
	for (int i = 0; i < yut_arrsz(d->a); i++)
		d->a[i] = base + i;
}

static void
verify_data(struct data *d) {
	for (int i = 1; i < yut_arrsz(d->a); i++)
 		yassert(d->a[i - 1] + 1 == d->a[i]);
}

#define busy_delay(n) do {		\
	int r____ = (n);		\
	while (r____-- > 0) {		\
	}				\
} while (0)

static void *
rb_reader(void *arg) {
	struct data d;
	int r, i, retry;
	struct threadarg *ta = arg;
	struct yringbuf *rb = ta->rb;
	for (i = 0; i < READ_COUNT; i++) {
		dummy_cost_op();
		retry = 3;
		while (retry-- > 0) {
			if (0 == (r = yringbuf_read(rb, &d)))
				break;
		};
		if (-EAGAIN == r) {
			ta->st.rdfail++;
			continue;
		}
		verify_data(&d);
	}
	return NULL;
}

static void
test_ringbuf(void) {
	int i, r;
	pthread_t thds[NR_READERS];
	struct threadarg thdargs[NR_READERS];
	struct yringbuf *rb;
	struct data d;

	memset(&thdargs, 0, sizeof(thdargs));

	rb = yringbuf_create(NR_SLOT, sizeof(struct data));
	yassert(rb);
	set_data(&d, 0);
	yringbuf_write(rb, &d);

	for (i = 0; i < yut_arrsz(thds); i++) {
		thdargs[i].rb = rb;
		r = pthread_create(
			&thds[i],
			NULL,
			&rb_reader,
			&thdargs[i]);
		yassert(!r);
	}
	/* context switching */
	usleep(1);

	for (i = 0; i < WRITE_COUNT; i++) {
		dummy_cost_op();
		set_data(&d, i);
		yringbuf_write(rb, &d);
	}

	int fcnt = 0;
	for (i = 0; i < yut_arrsz(thds); i++) {
		pthread_join(thds[i], NULL);
		fcnt += thdargs[i].st.rdfail;
	}
	/* 37 bytes data with 5 slot is usually safe enough! */
	yassert(fcnt < READ_COUNT / 100);

	yringbuf_destroy(rb);
}

TESTFN(ringbuf)

#endif /* CONFIG_TEST */
