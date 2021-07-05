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
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "lib.h"
#include "ylog.h"

#define LOG_BUF_SZ 4096

/**
 * global variable is used for performance.
 */
enum yloglv ___yloglv;

static struct {
	int stdfd;
	int errfd;
} _cfg;

static pthread_key_t _tkey;

/**
 * This MUST match enum yloglv in ylog.h
 */
static const char _lvchar[] = { 'V', 'D', 'I', 'W', 'E', 'F' };

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static char *
get_buffer(void) {
	void *b = pthread_getspecific(_tkey);
	if (unlikely(!b)) {
		b = ymalloc(LOG_BUF_SZ);
		if (unlikely(!b))
			return NULL;
		pthread_setspecific(_tkey, b);
	}
	return b;
}

static int
write_fd(int fd, const char *buf, int sz) {
	while (sz > 0) {
		int eno;
		int n = write(fd, buf, sz);
		if (unlikely(n < 0)) {
			eno = errno;
			if (EAGAIN != eno)
				return -eno;
			n = 0;
		}
		sz -= n;
	}
	return 0;
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
void
___ylog_write(enum yloglv lv,
	      const char *file, int lineno,
	      const char *fmt, ...) {
	int r, fd;
	struct timespec tm;
	char *buf = get_buffer();
	if (unlikely(!buf))
		return; /* ignore this log */
	if (unlikely(YLOG_VERBOSE > lv
		     || YLOG_FATAL < lv))
		return; /* invalid log level. ignore this log */
	if (unlikely(clock_gettime(CLOCK_MONOTONIC, &tm)))
		memset(&tm, 0, sizeof(tm));
	/* add log prefix */
	r = snprintf(buf, LOG_BUF_SZ, "[%c] %9lu.%9lu %s:%d ",
		     _lvchar[lv], tm.tv_sec, tm.tv_nsec, file, lineno);
	if (likely(r < LOG_BUF_SZ)) {
		int n;
		va_list alist;
		va_start(alist, fmt);
		n = vsnprintf(buf + r, LOG_BUF_SZ - r, fmt, alist);
		va_end(alist);
		r += n;
	}
	if (unlikely(r > LOG_BUF_SZ))
		r = LOG_BUF_SZ;
	fd = (likely(lv < YLOG_ERR))? _cfg.stdfd: _cfg.errfd;
	write_fd(fd, buf, r); /* return value is ignored intentionally */
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
void
log_clear(void) {
	void *b = pthread_getspecific(_tkey);
	if (likely(b))
		yfree(b);
	pthread_setspecific(_tkey, NULL);
}
#endif /* CONFIG_DEBUG */

static int
minit(const struct ylib_config *cfg) {
	int r;

	r = pthread_key_create(&_tkey, &yfree);
	if (unlikely(r))
		return -r;

	/* Set to default value */
	___yloglv = YLOG_WARN;
	_cfg.stdfd = STDOUT_FILENO;
	_cfg.errfd = STDERR_FILENO;

	if (!cfg)
		return 0; /* use default */

	if (unlikely(0 <= cfg->ylog_stdfd))
		_cfg.stdfd = cfg->ylog_stdfd;
	if (unlikely(0 <= cfg->ylog_errfd))
		_cfg.errfd = cfg->ylog_errfd;
	if (unlikely(YLOG_VERBOSE <= cfg->ylog_level
		|| YLOG_FATAL >= cfg->ylog_level)
	) { ___yloglv = cfg->ylog_level; }
	return 0;
}

static void
mexit(void) {
	fatali0(pthread_key_delete(_tkey));
}

LIB_MODULE(log, minit, mexit) /* @suppress("Unused static function") */
