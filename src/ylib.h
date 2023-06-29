/******************************************************************************
 * Copyright (C) 2016, 2023
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
 * @file ylib.h
 * @brief Header to use ylib
 */

#pragma once

/*
 * Include headers for modules
 */
#include "ydef.h"
#include "ylog.h"

/** Configuration for ylib */
struct ylib_config {
	/**
	 * capacity of msg-object-pool. if <=0, then default capacity is used
	 */
	int ymsg_pool_capacity;
	/**
	 * capacity of yo-pool. if <=0, then default capacity is used
	 */
	int yo_pool_capacity;
	/**
	 * capacity of ygp-pool. if <=0, then default capacity is used
	 * It's recommended using smaller value than yo_pool_capacity.
	 */
	int ygp_pool_capacity;
	/**
	 * File descriptor where standard log - all except for ERR and FATAL -
	 * is written to. If it is invalid, default fd is used.
	 */
	int ylog_stdfd;
	/**
	 * File descriptor where err(ERR, FATAL) log is written to.
	 * If it is invalid, default fd is used.
	 */
	int ylog_errfd;
	/**
	 * Log level. If it is invalid default value is used.
	 */
	enum yloglv ylog_level; /**< level of ylog */
};

/**
 * Initialize ylib. Call this before using ylib.
 * Before calling this, ylib may not be available.
 *
 * @param c Config struct. Once it is used, ylib doesn't refer it anymore.
 * (This is, client can free memory.)
 * Set to NULL to use default configuration.
 * @return 0 if success. Otherwise @c -errno.
 */
YYEXPORT int
ylib_init(const struct ylib_config *c);

/**
 * Cleanup internal-reasources used by ylib.
 * After calling this, ylib may become unavailable.
 *
 * @return 0 if success. Otherwise @c -errno.
 */
YYEXPORT int
ylib_exit(void);
