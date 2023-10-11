/******************************************************************************
 * Copyright (C) 2015, 2023
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
 * @file yheap.h
 * @brief Header to use simple binary heap.
 *
 * Max-heap. For min-heap, change sign of coimpare function.
 */

#pragma once

/**
 * Define below config to use max-heap using static value instead of using
 * function pointer for comparison.
 * This is good to reduce cost spent at function call.
 */
/* #define CONFIG_YHEAP_STATIC_CMP_MAX_HEAP 1 */

#include "ydef.h"

/** heap object */
struct yheap;


struct yheap_node {
	/** Value used internally in heap. DO NOT access it at outside heap */
	uint32_t i;
#ifdef CONFIG_YHEAP_STATIC_CMP_MAX_HEAP
	int v; /** Value used for comparison */
#endif
#ifdef CONFIG_DEBUG
	struct yheap *h; /** heap where this node is living. */
#endif
};

/**
 * Create heap object.
 *
 * @param capacity Initial heap capacity. Capacity means space available without
 * additional memory allocation.
 * @param vfree Function to free item.
 * @param cmp Function to compare two heap items.
 * Return value follows rule of strcmp function.
 * @return NULL if fails (ex. ENOMEM)
 */
YYEXPORT struct yheap *
yheap_create(
	uint32_t capacity,
	void (*vfree)(struct yheap_node *)
#ifndef CONFIG_YHEAP_STATIC_CMP_MAX_HEAP
	, int (*cmp)(const struct yheap_node *, const struct yheap_node *)
#endif
);

/**
 * Create heap with given pointers of nodes.
 */
YYEXPORT struct yheap *
yheap_create2(
	struct yheap_node **arr,
	uint32_t arrsz,
	void (*vfree)(struct yheap_node *)
#ifndef CONFIG_YHEAP_STATIC_CMP_MAX_HEAP
	, int (*cmp)(const struct yheap_node *, const struct yheap_node *)
#endif
);

/**
 * Clean heap.
 * All items in the heap will be freed.
 */
YYEXPORT void
yheap_reset(struct yheap *);

/**
 * Destroy heap.
 */
YYEXPORT void
yheap_destroy(struct yheap *);

/**
 * Push item to heap.
 *
 * @return 0 if success. Otherwise @c -errno.
 */
YYEXPORT int
yheap_push(struct yheap *, struct yheap_node *);

/**
 * Get item at top of heap tree. Item is NOT removed from heap.
 *
 * @return Biggest item. NULL if heap is empty.
 */
YYEXPORT struct yheap_node *
yheap_peek(const struct yheap *);

/**
 * Get item at top of heap tree, and it is removed from heap.
 *
 * @return Biggest item. NULL if heap is empty.
 */
YYEXPORT struct yheap_node *
yheap_pop(struct yheap *);

/**
 * Push then pop.
 *
 * @param in node to push
 * @param out popped node
 * @return YYEXPORT
 */
YYEXPORT struct yheap_node *
yheap_pushpop(struct yheap *, struct yheap_node *);


/**
 * Pop then push.
 *
 * @param in node to push
 * @param out popped node
 * @return YYEXPORT
 */
YYEXPORT struct yheap_node *
yheap_poppush(struct yheap *, struct yheap_node *);

/**
 * Heapify node. This must be called if data of node is updated.
 * Node must already live in heap.
 */
void
yheap_heapify(struct yheap *, struct yheap_node *);


/**
 * Get number of elements in the heap.
 *
 * @return Heap size.
 */
YYEXPORT uint32_t
yheap_sz(const struct yheap *);

/**
 * Iterate heap elements
 *
 * @param ctx Context of this iteration.
 * @param cb callback called whenever visiting element.
 * If callback returns TRUE, iteration keeps going.
 * But if callback returns FALSE, iteration stops and function is returned.
 * @return -1 if fails (ex. internal error.).
 * 0 if iteration to all heap nodes are done.
 * 1 if iteration is stopped in the middle because @p cb returns FALSE.
 */
YYEXPORT int
yheap_iterates(
	struct yheap *,
	void *ctx,
	int (*cb)(struct yheap_node *e, void *ctx));
