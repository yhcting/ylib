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

/**
 * @file yringbuf.h
 * @brief Use ring-buffer supporting best-effort lockless single-writer and
 * multiple readers.
 *
 */

#pragma once

#include "ydef.h"

struct yringbuf;

/**
 * Create ringbuffer. In case that @p slotsz is very large or write happens very
 * frequently, to avoid retrying read due to polluted data, increase @p slotnr.
 * But, in most case, 3 is enough.
 *
 * @param slotnr Number of slots in ring.
 * @param slotsz Size of each slot.
 * @return struct yringbuf *
 */
YYEXPORT struct yringbuf *
yringbuf_create(uint32_t slotnr, uint32_t slotsz);

/**
 * It should be executed only in thread where `ytringbuf_create` is executed.
 */
YYEXPORT void
yringbuf_destroy(struct yringbuf *);

/**
 * Write @p slotsz bytes of data (MT-undafe).
 * This should be called in only one context at a time.
 * 'write' should be done only in thread where `ytringbuf_create` is executed.
 * Always success.
 */
YYEXPORT void
yringbuf_write(struct yringbuf *, void *data);

/**
 * Read data (MT-safe with best-effort). If function fails, data in @p buf is
 * not defined.
 *
 * @param buf Destination buffer
 * @return 0 if success. Otherwise @c -errno.
 */
YYEXPORT int
yringbuf_read(struct yringbuf *, void *buf);
