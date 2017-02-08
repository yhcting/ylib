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

/**
 * @file yproc.h
 * @brief Module for easy-use linux /proc file system.
 */

#ifndef __YPROc_h__
#define __YPROc_h__

#ifndef __linux__
#   error yproc is linux specific module!
#endif

#include "ydef.h"

/**< At inux kernel it is set as 16 - See TASK_COMM_LEN. */
#define YPROC_TCOMM_LEN 16

/**
 * Data struct for "/proc/<pid>/stat" in linux
 */
struct yproc_pid_stat {
	int pid; /**< Process id */
	// +1 for terminating 0
	char tcomm[YPROC_TCOMM_LEN + 1]; /**< TASK COMM string */
	char state; /**< Process state (ex. R, S ...) */
	int ppid; /**< Parent process id */
	int pgid; /**< Process group id */
	int sid; /**< Session id */
	/* TODO:  fill other fields too. */
};


/**
 * Get file fpath from fd.
 *
 * @param fd File descriptor
 * @param buf Buffer in where file path is stored.
 * @param bufsz Bytes of {@code buf} buffer.
 * @return 0 for success. Otherwise -errno.
 */
YYEXPORT int
yproc_self_fd_path(int fd, char *buf, unsigned bufsz);


/**
 * Get process command line from pid.
 *
 * @param pid Process id
 * @param buf Buffer in where command-line is stored.
 * @param bufsz Bytes of {@code buf} buffer.
 * @return 0 for success. Otherwise -errno.
 */
YYEXPORT int
yproc_pid_cmdline(int pid, char *buf, unsigned bufsz);

/**
 * Get parent process id of given pid
 *
 * @param stat output data struct.
 * @param pid Process id
 * @return -errno if error, otherwise 0.
 */
YYEXPORT int
yproc_pid_stat(struct yproc_pid_stat *stat, int pid);


#endif /* __YPROc_h__ */
