/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014
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


/*
 * Trie is tested at ylisp.
 * So, there is no specific test file....
 */

#ifndef __YTRIe_h__
#define __YTRIe_h__

#include <stdint.h>

#define YTRIE_MAX_KEY_LEN  1024

struct ytrie;

/**
 * get pointer of element.
 * (To replace element directly! - Only for performance reason.)
 */
extern void **
ytrie_getref(struct ytrie *t,
	     const u8 *key, u32 sz);

/**
 * get element
 */
extern void*
ytrie_get(struct ytrie *t, const u8 *key, u32 sz);


/**
 * walk trie nodes.
 * @cb : callback that is called whenever walker visits element.
 *	 If @cb returns 1, walker keeps going.
 *	 But if @cb returns 0, walker stops and 'ytrie_walk' is done.
 */
extern int
ytrie_walk(struct ytrie *t,
	   void         *user,
	   const u8     *from,
	   u32           fromsz,
	   /* return 1 for keep going, 0 for stop and don't do anymore */
	   int         (*cb)(void *, const u8 *, u32, void *));

/**
 * insert element
 */
extern int
ytrie_insert(struct ytrie *t,
	     const u8     *key,
	     u32           sz,
	     void         *v);

/**
 * @fcb : callback to free element.
 */
extern struct ytrie *
ytrie_create(void(*fcb)(void *));

/**
 * just clean contents of trie.
 */
extern void
ytrie_clean(struct ytrie *);

/**
 * trie itself is destroyed
 */
extern void
ytrie_destroy(struct ytrie *);

/**
 * delete node.
 */
extern int
ytrie_delete(struct ytrie *, const u8 *key, u32 sz);

/**
 * get free callback of trie
 */
extern void(*
ytrie_fcb(const struct ytrie *))(void *);

/**
 * @cmp : callback to compare element.
 *	  (return value should be follows the one of 'strcmp')
 */
extern int
ytrie_equal(const struct ytrie *, const struct ytrie *,
	    int(*cmp)(const void *, const void *));

/**
 * copy trie - deep copy
 */
extern int
ytrie_copy(struct ytrie *dst, const struct ytrie *src, void *user,
	   void *(*clonev)(void *,const void *));

/**
 * @clonev : callback function that clones element.
 */
extern struct ytrie *
ytrie_clone(const struct ytrie *,
	    void *user, void *(*clonev)(void *, const void *));

/**
 * @return
 *    0 : meets the branch. (there are more than one candidates
 *	   that starts with @start_with)
 *    1 : meets leaf. (there is only one candidate)
 *    2 : fails. (ex. there is no candidates)
 */
extern int
ytrie_auto_complete(struct ytrie *,
		    const u8 *start_with, u32 sz,
		    u8 *buf, u32 bufsz);

#endif /* __YTRIe_h__ */
