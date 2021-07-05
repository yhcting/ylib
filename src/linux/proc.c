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

//#define YDPRINT

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

#include "def.h"
#include "common.h"
#include "yproc.h"



int
yproc_self_fd_path(int fd, char *buf, unsigned bufsz) {
	int r;
	char pathbuf[PATH_MAX];
	if (unlikely(fd <= 0 || !buf || !bufsz))
		return -EINVAL;
	r = snprintf(pathbuf, sizeof(pathbuf), "/proc/self/fd/%d", fd);
	if (unlikely(r >= sizeof(pathbuf)))
		return -EINVAL; // truncated!
	if (0 > (r = readlink(pathbuf, buf, bufsz - 1)))
		return -errno;
	buf[r] = 0; /* add trailing 0 */
	return 0;
}

int
yproc_pid_cmdline(int pid, char *buf, unsigned bufsz) {
	int r, fd;
	char *aux;

	if (unlikely(!buf || !bufsz))
		return -EINVAL;
	/* Try to get program name by PID */
	r = snprintf(buf, bufsz, "/proc/%d/cmdline", pid);
	if (unlikely(r >= bufsz))
		return -EINVAL; /* truncated! */
	if (unlikely(0 > (fd = open(buf, O_RDONLY))))
		return -errno;

	/* Read file contents into buffer */
	if (unlikely(0 >= (r = read(fd, buf, bufsz - 1)))) {
		r = errno;
		close(fd);
		return -r;
	}
	close(fd);

	buf[r] = 0;
	aux = strstr(buf, "^@");
	if (aux)
		*aux = 0;
	return 0;
}

int
yproc_pid_stat(struct yproc_pid_stat *stat, int pid) {
	int r, fd;
	char buf[4096]; /* this is large enough */
	if (unlikely(!stat))
		return -EINVAL;
	r = snprintf(buf, sizeof(buf), "/proc/%d/stat", pid);
	if (unlikely(r >= sizeof(buf)))
		return -EINVAL; /* This is UNEXPECTED! */
	if (unlikely(0 > (fd = open(buf, O_RDONLY))))
		return -errno;

	/* Read file contents into buffer */
	if (unlikely(0 > (r = read(fd, buf, sizeof(buf) - 1)))) {
		r = -errno;
		goto close_fd;
	}
	buf[r] = 0; /* trailing 0 */

	dpr("raw string:%s", buf);
	r = sscanf(buf,
		"%d (%[^)]) %c %d %d %d",
		&stat->pid,
		stat->tcomm,
		&stat->state,
		&stat->ppid,
		&stat->pgid,
		&stat->sid);
	dpr("sscanf result: %d\n", r);
	if (6 != r) {
		/* This is UNEXPECTED! Kernel may be updated? */
		r = -EINVAL;
		goto close_fd;
	}
	dpr("%d (%s) %c %d %d %d\n",
		stat->pid,
		stat->tcomm,
		stat->state,
		stat->ppid,
		stat->pgid,
		stat->sid);

	r = 0; // mark as success

 close_fd:
	close(fd);
	return r;
}
