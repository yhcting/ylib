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


/**
 * ylistl => ylist Low level.
 * Low level list.
 */


#ifndef __YLISTl_h__
#define __YLISTl_h__

#include "ydef.h"

/*======================================
 * Double Linked List
 *======================================*/

#define YLISTL_DECL_HEAD(hd) struct ylistl_link hd = {&hd, &hd}

/**
 * If possible DO NOT access struct directly!.
 */
struct ylistl_link {
	struct ylistl_link *next, *prev;
};

/**
 * initialize list head.
 */
static inline void
ylistl_init_link(struct ylistl_link *link) {
	link->next = link->prev = link;
}

static inline int
ylistl_is_empty(const struct ylistl_link *head) {
	return head->next == head;
}

static inline void
ylistl_add(struct ylistl_link *prev,
	   struct ylistl_link *next,
	   struct ylistl_link *anew) {
	next->prev = prev->next = anew;
	anew->next = next; anew->prev = prev;
}

static inline void
ylistl_add_next(struct ylistl_link *link, struct ylistl_link *anew) {
	ylistl_add(link, link->next, anew);
}

static inline void
ylistl_add_prev(struct ylistl_link *link, struct ylistl_link *anew) {
	ylistl_add(link->prev, link, anew);
}

static inline void
ylistl_add_first(struct ylistl_link *head, struct ylistl_link *anew) {
	ylistl_add_next(head, anew);
}

static inline void
ylistl_add_last(struct ylistl_link *head, struct ylistl_link *anew) {
	ylistl_add_prev(head, anew);
}

static inline void
__ylistl_del(struct ylistl_link *prev, struct ylistl_link *next) {
	prev->next = next;
	next->prev = prev;
}

static inline void
ylistl_del(struct ylistl_link *link) {
	__ylistl_del(link->prev, link->next);
}

static inline void
ylistl_replace(struct ylistl_link *old, struct ylistl_link *anew) {
	anew->next = old->next;
	anew->next->prev = anew;
	anew->prev = old->prev;
	anew->prev->next = anew;
}

/**
 * @pos     : the &struct ylistl_link to use as a loop cursor
 * @head    : head of list (&struct ylistl_link)
 */
#define ylistl_foreach(pos, head)					\
        for ((pos) = (head)->next; (pos) != (head); (pos) = (pos)->next)

#define ylistl_foreach_backward(pos, head)				\
        for ((pos) = (head)->prev; (pos) != (head); (pos) = (pos)->prev)

/**
 * @pos     : the &struct ylistl_link to use as a loop cursor
 * @n       : another &struct ylistl_link to use as temporary storage
 * @head    : head of list (&struct ylistl_link)
 */
#define ylistl_foreach_removal_safe(pos, n, head)	\
        for ((pos) = (head), (n) = (pos)->next;		\
	     (pos) != (head);				\
	     (pos) = (n), (n) = (pos)->next)

#define ylistl_foreach_removal_safe_backward(pos, n, head)	\
        for ((pos) = (head), (n) = (pos)->prev;			\
	     (pos) != (head);					\
	     (pos) = (n), (n) = (pos)->prev)
/**
 * @pos     : the @type* to use as a loop cursor.
 * @head    : the head for list (&struct ylistl_link)
 * @type    : the type of the struct of *@pos
 * @member  : the name of the ylistl_link within the struct.
 */
#define ylistl_foreach_item(pos, head, type, member)			\
        for ((pos) = container_of((head)->next, type, member);		\
	     &(pos)->member != (head);					\
	     (pos) = container_of((pos)->member.next, type, member))

#define ylistl_foreach_item_backward(pos, head, type, member)		\
        for ((pos) = container_of((head)->prev, type, member);		\
	     &(pos)->member != (head);					\
	     (pos) = container_of((pos)->member.prev, type, member))

/**
 * @pos     : the @type* to use as a loop cursor.
 * @n       : another @type* to use as temporary storage.
 * @head    : the head for list (&struct ylistl_link)
 * @type    : the type of the struct of *@pos
 * @member  : the name of the ylistl_link within the struct.
 */
#define ylistl_foreach_item_removal_safe(pos, n, head, type, member)    \
        for ((pos) = container_of((head)->next, type, member),		\
		     (n) = container_of((pos)->member.next, type, member); \
	     &(pos)->member != (head);					\
	     (pos) = (n),						\
		     (n) = container_of((pos)->member.next, type, member))

#define ylistl_foreach_item_removal_safe_backward(pos, n, head, type, member) \
        for ((pos) = container_of((head)->prev, type, member),		\
		     (n) = container_of((pos)->member.prev, type, member); \
	     &(pos)->member != (head);					\
	     (pos) = (n),						\
		     (n) = container_of((pos)->member.prev, type, member))

static inline unsigned int
ylistl_size(const struct ylistl_link *head) {
	struct ylistl_link *pos;
	unsigned int size = 0;
	ylistl_foreach(pos, head)
		size++;
	return size;
}

#endif /* __YLISTl_h__ */
