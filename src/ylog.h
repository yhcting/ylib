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
 * @file ylog.h
 * @brief Header to use ylog
 */

#ifndef __YLOg_h__
#define __YLOg_h__

#include "ydef.h"

/**
 * Log level.
 */
enum yloglv {
	YLOG_VERBOSE = 0,
	YLOG_DEBUG,
	YLOG_INFO,
	YLOG_WARN,
	YLOG_ERR,
	YLOG_FATAL,
	YLOG_DISABLE_LOG,
	YLOG_NR_LOGLV = YLOG_DISABLE_LOG
};

/**
 * Internal variable. DO NOT edit this value directly.
 * Due to performance reason, varaible is exposed to the public(global).
 */
extern enum yloglv ___yloglv;

/**
 * Get current log level.
 */
static YYINLINE enum yloglv
ylog_loglv(void) {
	return ___yloglv;
}

/**
 * This is only for internal use. Client SHOULD NOT use this function
 *   DIRECTLY.
 */
YYEXPORT void
___ylog_write(enum yloglv, const char *file, int lineno, const char *fmt, ...);

/**
 * This is only for internal use. Client SHOULD NOT use this function
 *   DIRECTLY.
 */
#define _ylog(lv, fmt, args...)						\
	if (YYunlikely((lv) >= ylog_loglv())) {				\
		___ylog_write(lv, __FILE__, __LINE__, fmt, ##args);	\
	}

/**
 * log YLOG_VERBOSE
 */
#define ylogv(fmt, args...) _ylog(YLOG_VERBOSE, fmt, ##args)

/**
 * log YLOG_DEBUG
 */
#define ylogd(fmt, args...) _ylog(YLOG_DEBUG, fmt, ##args)

/**
 * log YLOG_INFO
 */
#define ylogi(fmt, args...) _ylog(YLOG_INFO, fmt, ##args)

/**
 * log YLOG_WARN
 */
#define ylogw(fmt, args...) _ylog(YLOG_WARN, fmt, ##args)

/**
 * log YLOG_ERR
 */
#define yloge(fmt, args...) _ylog(YLOG_ERR, fmt, ##args)

/**
 * log YLOG_FATAL
 */
#define ylogf(fmt, args...) _ylog(YLOG_FATAL, fmt, ##args)


#endif /* __YLOg_h__ */
