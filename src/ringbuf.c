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

#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "common.h"
#include "yringbuf.h"


/*
 * Write process with example
 * --------------------------
 * Only one context is allowed to write.
 *
 * Pre:
 *   Get slot to write: 'yringbuf.wsi'-th slot.
 * Before writing:
 *   ver: 0x10
 * Writing:
 *   Set-ver: 0x11 (means 'slot 0x10 is under updating')
 *   Write
 *   Set-ver: 0x20 (means 'slot 0x20 is valid available.')
 * Post:
 *   Update writing buffer index: yringbuf.wsi++
 *
 *
 * Read process
 * ------------
 * Reading may be retried on failure.
 *
 * Pre:
 *   Get slot of previous index of 'yringbuf.wsi'.
 *   This is recently update data.
 * Reading:
 *   Get-ver:
 *     - if (ver & 0x1 == 1) then failure.
 *     - Save-ver to 'verBefore'
 *   Read
 *   Get-ver:
 *     - if (ver == verBefore) then success.
 *     - otherwise failure.
 */


struct slot {
	/**
	 * monotonic increasing VERsion of data.
	 * [31:1]: version number (sequence number)
	 * [1: 0]: status floag (valid / invalid)
	 *
	 * 0 == (0x1 & version) => valid
	 *   - Data in slot is valid.
	 * 1 == (0x1 & version) => invalid
	 *   - slot is under writing.
	 *
	 * This concept is tolerant of overflow, too.
	 * 0xfffffffe(valid) -> 0xffffffff(invalid) -> 0(valid)
	 */
	uint32_t ver;
	/**
	 * User slot data.
	 */
	char d[0];
};

struct yringbuf {
	/* ---------- These are read-only of this struct. ----------- */
	uint32_t user_slotsz; /**< User slot size*/
	uint32_t slotnr; /**< Number of user slot */
	/**
	 * Owner thread of this ringbuffer.
	 * Only owner thread can write slot.
	 */
	pthread_t owner;

	/* ----------- These are under concurrent environment. ------- */
	uint32_t sid; /**< monotonic increasing slot id */
	/** write slot index(slot index to write or be writing) */
	uint32_t wsi;
	/**
	 * Databuffer.
	 * Note that @p d is  at 4 bytes-aligned position.
	 */
	char d[0];
};


/**
 * Internal slot size(4 bytes aligned) including id and user slot.
 */
static inline uint32_t
slotsz_(uint32_t user_slotsz) {
	/* To access slot.id without any potential problem, slot MUST be 4-btyes
	 * aligned.
	 * Get size of 4-bytes aligned.
	 */
	return ((sizeof(struct slot) + user_slotsz + 3) / 4) * 4;
}

static inline uint32_t
slotsz(struct yringbuf *rb) {
	return slotsz_(rb->user_slotsz);
}

static inline struct slot *
slotof(struct yringbuf *rb, uint32_t index) {
	return (struct slot *)(rb->d + slotsz(rb) * index);
}

struct yringbuf *
yringbuf_create(uint32_t slotnr, uint32_t user_slotsz) {
	if (unlikely(slotnr < 2))
		return NULL;

	struct yringbuf * rb = ymalloc(sizeof(struct yringbuf)
		+  slotsz_(user_slotsz) * slotnr);
	rb->user_slotsz = user_slotsz;
	rb->slotnr = slotnr;
	rb->sid = 0;
	rb->wsi = 0;
	rb->owner = pthread_self();

	/* Slots are initialized as 0 */
	memset(&rb->d, 0, slotsz(rb) * slotnr);
	return rb;
}

void
yringbuf_destroy(struct yringbuf *rb) {
	yassert(pthread_equal(rb->owner, pthread_self()));
	yfree(rb);
}

void
yringbuf_write(struct yringbuf *rb, void *data) {
	yassert(pthread_equal(rb->owner, pthread_self()));

	/* This is the only place updating wsi.
	 * So, in case of read, atomic operation is not required
	 */
	uint32_t wsi = rb->wsi;
	struct slot *slot = slotof(rb, wsi);
	/* Mark as 'slot is under updating by set LSB as 1' because inital value
	 * of 'ver' is 0, and write is done only in one thread context.
	 * And ATOMIC_RELAXED is enough because memcpy function works as
	 * barrier. However, to make more explicit, 'ATOMIC_RELEASE' is used.
	 */
	__atomic_add_fetch(&slot->ver, 1, __ATOMIC_RELEASE);
	memcpy(&slot->d, data, rb->user_slotsz);
	/* Mark as 'slot' is updated and available by adding 1.
	 * This increase version number by 1 and set LSB 0.
	 * ATOMIC_ACQ_REL is used.
	 */
	__atomic_add_fetch(&slot->ver, 1, __ATOMIC_ACQ_REL);
	/* Update slot index for write. */
	wsi = (wsi + 1) % rb->slotnr;
	/* This should be done at the end. ATOMIC_RELEASE is used. */
	__atomic_store(&rb->wsi, &wsi, __ATOMIC_RELEASE);
}

int
yringbuf_read(struct yringbuf *rb, void *buf) {
	uint32_t wsi = __atomic_load_n(&rb->wsi, __ATOMIC_ACQUIRE);
	/* Most recently written slot index. */
	wsi = ((0 == wsi ? rb->slotnr : wsi) - 1) % rb->slotnr;
	struct slot *slot = slotof(rb, wsi);
	/* ATOMIC_RELAXED is enough because memcpy function works as barrier
	 * However, to make more explicit, 'ATOMIC_ACQUIRE' is used.
	 */
	uint32_t ver_pre = __atomic_load_n(&slot->ver, __ATOMIC_ACQUIRE);
	if (ver_pre & 1)
		return -EAGAIN;

	memcpy(buf, &slot->d, rb->user_slotsz);
	uint32_t ver_post = __atomic_load_n(&slot->ver, __ATOMIC_ACQUIRE);
	return ver_pre == ver_post ? 0 : -EAGAIN;
}
