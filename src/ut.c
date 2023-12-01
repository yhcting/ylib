/******************************************************************************
 * Copyright (C) 2015, 2016, 2023
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
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdarg.h>
#include <ctype.h>

#include "common.h"
#include "yut.h"

static inline long s2ns(long x) {
	return x * 1000 * 1000 * 1000;
}

bool
yut_starts_with(const char *str, const char *substr) {
	int len, lensub, i;
	if (unlikely(!str || !substr))
		return FALSE;
	len = strlen(str);
	lensub = strlen(substr);
	if (unlikely(lensub > len))
		return FALSE;
	for (i = 0; i < lensub; i++) {
		if (unlikely(str[i] != substr[i]))
			return FALSE;
	}
	return TRUE;
}

char *
yut_trim_whitespaces(char *s) {
	char *e = s + strlen(s) - 1;
	while (isspace(*s)) s++;
	while(s < e && isspace(*e)) e--;
	if (e >= s && *e)
		e[1] = 0;
	return s;
}

uint64_t
yut_current_time_us(void) {
	struct timespec ts;
	fatali0(clock_gettime(CLOCK_MONOTONIC, &ts));
	return ts.tv_sec * 1000000LL + ts.tv_nsec / 1000LL;
}


int
yut_write_to_fd(int fd, const char *buf, int sz) {
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

int
yut_write_to_file(const char *path, bool append, const char *buf, int sz) {
	int fd, r;
	if (unlikely(0 > (fd = open(path,
		O_WRONLY | O_CLOEXEC
			| (append ? O_APPEND : (O_CREAT | O_TRUNC)),
		0600)))
	) { return -errno; }
	r = yut_write_to_fd(fd, buf, sz);
	close(fd);
	return r;
}

static int
write_to_fd_fmt(int fd, const char *fmt, va_list ap) {
	int r, len;
	char *buf = NULL;
	size_t sz = 2048;
	do {
		sz *= 2;
		if (unlikely(!(buf = realloc(buf, sz))))
			return -ENOMEM;
		len = vsnprintf(buf, sz, fmt, ap);
	} while (len < 0 || len >= sz);

	r = yut_write_to_fd(fd, buf, len);
	free(buf);
	return r < 0 ? r : len;
}


int
yut_write_to_fd_fmt(int fd, const char *fmt, ...) {
	int r;
	va_list ap;
	va_start(ap, fmt);
	r = write_to_fd_fmt(fd, fmt, ap);
	va_end(ap);
	return r;
}

YYEXPORT int
yut_write_to_file_fmt(const char *path, bool append, const char *fmt, ...) {
	int fd, r;
	va_list ap;
	if (unlikely(0 > (fd = open(path,
		O_WRONLY | O_CLOEXEC
			| (append ? O_APPEND : (O_CREAT | O_TRUNC)),
		0600)))
	) { return -errno; }
	va_start(ap, fmt);
	r = write_to_fd_fmt(fd, fmt, ap);
	va_end(ap);
	close(fd);
	return r;
}

int
yut_read_fd_str(int fd, char **out) {
	ssize_t rd;
	char *bufp; /* next free position in buf */
	char *buf;
	int bufsz;

	/* + 1 for trailing 0 */
	bufsz = 4096; /* Initial buffer size */
	if (unlikely(!(buf = ymalloc(bufsz + 1))))
		return -ENOMEM;
	bufp = buf;

	/* read all messages available */
	do {
		int buf_remains = bufsz - (bufp - buf);
		rd = read(fd, bufp, buf_remains);
		if (unlikely(rd < 0)) {
			if (EINTR == errno)
				continue;
			yfree(buf);
			return -errno;
		}
		if (unlikely(0 == rd))
			break;
		bufp += rd;
		if (unlikely(rd == buf_remains)) {
			/* extend buffer: + 1 for trailing 0 */
			void *tmp = ymalloc(bufsz * 2 + 1);
			if (unlikely(!tmp)) {
				yfree(buf);
				return -ENOMEM;
			}
			memcpy(tmp, buf, bufp - buf);
			bufsz *= 2;
			bufp = tmp + (bufp - buf);
			yfree(buf);
			buf = tmp;
		} else {
			break;
		}
	} while (TRUE);

	*bufp = 0; /* trailing 0 */
	*out = buf;
	return bufp - buf;
}

int
yut_read_fd_long(int fd, long *out) {
	int r;
	char *s = NULL;
	r = yut_read_fd_str(fd, &s);
	if (likely(r >= 0 && s))
		*out = atol(s);
	if (likely(s))
		yfree(s);
	return r;
}

int
yut_read_fd_double(int fd, double *out) {
	int r;
	char *s = NULL;
	r = yut_read_fd_str(fd, &s);
	if (likely(r >= 0 && s))
		*out = atof(s);
	if (likely(s))
		yfree(s);
	return r;
}

int
yut_read_file_str(const char *path, char **out) {
	int fd, r;
	if (unlikely(0 > (fd = open(path, O_RDONLY | O_CLOEXEC))))
		return -errno;
	r = yut_read_fd_str(fd, out);
	close(fd);
	return r;
}

int
yut_read_file_long(const char *path, long *out) {
	int r;
	char *s = NULL;
	r = yut_read_file_str(path, &s);
	if (likely(r >= 0 && s))
		*out = atol(s);
	if (likely(s))
		yfree(s);
	return r;
}

int
yut_read_file_double(const char *path, double *out) {
	int r;
	char *s = NULL;
	r = yut_read_file_str(path, &s);
	if (likely(r >= 0 && s))
		*out = atof(s);
	if (likely(s))
		yfree(s);
	return r;
}

/* Get first file */
int
yut_find_file_one(const char *dpath, char *buf, int bufsz) {
	size_t len;
	struct dirent *dent;
	DIR *dirp;
	dirp = opendir(dpath);
	if (unlikely(!dirp))
		return -errno;
	while ((dent = readdir(dirp))) {
		if (DT_REG != dent->d_type)
			continue;
		/* found */
		len = strlen(dent->d_name);
		if (len >= bufsz)
			return -EINVAL;
		strncpy(buf, dent->d_name, bufsz);
		return 0;
	}
	return -ENOENT;
}

struct timespec
yut_ts_add_ts(const struct timespec *a, const struct timespec *b) {
	const long ns_sec = s2ns(1);
	unsigned long ns = (a->tv_nsec % ns_sec) + (b->tv_nsec % ns_sec);
	return (struct timespec) {
		a->tv_sec + b->tv_sec
			+ (a->tv_nsec / ns_sec)
			+ (b->tv_nsec / ns_sec)
			+ (ns / ns_sec),
		ns % ns_sec
	};
}


struct timespec
yut_ts_add_ms(const struct timespec *ts, int ms) {
	struct timespec tmp = {
		.tv_sec = ms / 1000,
		.tv_nsec = (ms % 1000) * 1000 * 1000
	};
	return yut_ts_add_ts(ts, &tmp);
}
