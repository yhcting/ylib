/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015
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
 * @file ytrie.h
 * @brief Header to use 'trie' data structure.
 */

#ifndef __YTRIe_h__
#define __YTRIe_h__

#include <stdint.h>

/** Maximum key length in the trie */
#define YTRIE_MAX_KEY_LEN 1024

/** Trie object */
struct ytrie;

/**
 * Get pointer of value. So, user can edit trie value directly.
 * This is dangerous function. So DO NOT use it if possible.
 * This function can be minimize performance overhead.
 *
 * @param key Key of trie
 * @param keysz Key size
 * @return NULL if fails (ex. {@code key} isn't in the trie.)
 *         Otherwise pointer of trie value.
 */
YYEXPORT void **
ytrie_getref(struct ytrie *,
	     const uint8_t *key, uint32_t keysz);

/**
 * Get value.
 *
 * @param key Key of trie
 * @param keysz Key size
 * @return NULL if fail. Otherwise trie value.
 */
YYEXPORT void *
ytrie_get(struct ytrie *, const uint8_t *key, uint32_t keysz);


/**
 * Iterate trie nodes.
 *
 * @param tag Tag during iteration.
 * @param key Key of node where tree iteration starts at.
 * @param keysz Size of {@code fromkey}
 * @param cb
 *     Callback called whenever visiting trie value.
 *     If callback returns 1, iteration keeps going.
 *     But if callback returns 0, iteration stops and 'ytrie_iterate' is done.
 * @return
 *     - 0 : Iteration stops in the middle of iteration due to callback
 *           returns 0.
 *     - 1 : Interation is completely done.
 *     - <0 : Error. -errno.
 */
YYEXPORT int
ytrie_iterate(struct ytrie *,
              void *tag,
              const uint8_t *key,
              uint32_t keysz,
              int (*cb)(void *, const uint8_t *, uint32_t, void *));

/**
 * Insert value to the trie
 *
 * @param key Key of node where tree iteration starts at.
 * @param keysz Size of {@code fromkey}
 * @param v Value to add.
 * @return
 *     - 1: value is overwritten
 *     - 0: newly added
 *     - <0: -errno.
 */
YYEXPORT int
ytrie_insert(struct ytrie *,
	     const uint8_t *key,
	     uint32_t keysz,
	     void *v);

/**
 * Create trie object.
 *
 * @param vfree callback to free element in the trie.
 * @return NULL if fails.
 */
YYEXPORT struct ytrie *
ytrie_create(void(*vfree)(void *));

/**
 * Reset contents of trie. Trie becomes empty.
 */
YYEXPORT void
ytrie_reset(struct ytrie *);

/**
 * Destroy trie.
 */
YYEXPORT void
ytrie_destroy(struct ytrie *);

/**
 * Remove trie value.
 *
 * @param key Key of node where tree iteration starts at.
 * @param keysz Size of {@code fromkey}
 * @return Number of deleted values (0 means nothing deleted).
 *         -errno if fails.
 */
YYEXPORT int
ytrie_remove(struct ytrie *, const uint8_t *key, uint32_t keysz);

/**
 * Get function used to free trie value.
 *
 * @return function.
 */
YYEXPORT void(*
ytrie_vfree(const struct ytrie *))(void *);

/**
 * Is two tries are equal?
 *
 * @param cmp Callback to compare element.
 *	      Return value of callback function should be same with 'strcmp'.
 * @return TRUE or FALSE
 */
YYEXPORT bool
ytrie_equal(const struct ytrie *, const struct ytrie *,
	    int(*cmp)(const void *, const void *));

/**
 * Copy trie. Key is deep-copied. And value is copied by using {@code clonev}
 *
 * @param dst Destination trie object where value is copied to.
 *            All existing values will be removed and filled with new values.
 * @param src Source trie object where value is copied from.
 * @param tag Tag object passed to {@code clonev}
 * @param clonev Function used to copy trie value.
 *               It should returns NULL if fails.
 * @return 0 if success. Otherwise -errno.
 */
YYEXPORT int
ytrie_copy(struct ytrie *dst, const struct ytrie *src, void *tag,
	   void *(*clonev)(void *tag,const void *));

/**
 * Clone trie object.
 *
 * @param tag Tag object passed to {@code clonev}
 * @param clonev Callback function cloning element.
 *               It should returns NULL if fails.
 */
YYEXPORT struct ytrie *
ytrie_clone(const struct ytrie *,
	    void *tag, void *(*clonev)(void *tag, const void *v));

/**
 * Find unique key that starts with {@code start_with}.
 *
 * @param start_with Prefix value of key to find
 * @param sz Size of {@code start_with} key
 * @param buf (out)
 * @param bufsz
 * @return
 *     - 0: There are more than one candidates that starts with
 *            {@code start_with}.
 *     - 1: There is only one candidate.
 *     - 2: There is no matching candidate key.
 *     - <0: -errno. Ex. size of {@code buf} is not large enough.
 */
YYEXPORT int
ytrie_auto_complete(struct ytrie *,
		    const uint8_t *start_with, uint32_t sz,
		    uint8_t *buf, uint32_t bufsz);

#endif /* __YTRIe_h__ */
