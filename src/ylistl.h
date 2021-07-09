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
 * @file ylistl.h
 * @brief Header file for ylist low level interface.
 *
 * Low level list implementation.
 */

#ifndef __YLISTl_h__
#define __YLISTl_h__

#include "ydef.h"


/****************************************************************************
 *
 * Double Linked List
 *
 ****************************************************************************/
/**
 * Declare initialized struct @ref ylistl_link variable.
 *
 * @param hd Variable name to be declared and initialized.
 */
#define YLISTL_DEFINE_HEAD(hd) struct ylistl_link hd = {&hd, &hd}

/**
 * link structure.
 * DO NOT access struct directly, except that you have to!.
 */
struct ylistl_link {
	/* @cond */
	struct ylistl_link *next, *prev;
	/* @endcond */
};

/**
 * Initialize list head node.
 *
 * @param link Link
 */
static YYINLINE void
ylistl_init_link(struct ylistl_link *link) {
	link->next = link->prev = link;
}

/**
 * Is list is empty
 *
 * @param head Head node
 * @return boolean
 */
static YYINLINE bool
ylistl_is_empty(const struct ylistl_link *head) {
	return head->next == head;
}

/**
 * Does @c link has next link in the list whose head is @c head.
 *
 * @param head Head link of the list
 * @param link Link
 * @return TRUE or FALSE
 */
static YYINLINE bool
ylistl_has_next(
	const struct ylistl_link *head,
	const struct ylistl_link *link
) {
	return link->next != head;
}

/**
 * Does @c link has next link in the list whose head is @c head.
 *
 * @param head Head link of the list
 * @param link Link
 * @return TRUE or FALSE
 */
static YYINLINE bool
ylistl_has_prev(
	const struct ylistl_link *head,
	const struct ylistl_link *link
) {
	return link->prev != head;
}

/**
 * Add new link between @p prev and @p next.
 * Existing link of @p prev and @p next are replaced with @p anew.
 *
 * @param prev Previous node
 * @param next Next node
 * @param anew Node to add
 */
static YYINLINE void
ylistl_add(
	struct ylistl_link *prev,
	struct ylistl_link *next,
	struct ylistl_link *anew
) {
	next->prev = prev->next = anew;
	anew->next = next; anew->prev = prev;
}

/**
 * Add new link at next to @p link}.
 *
 * @param link Base link
 * @param anew Node to add
 */
static YYINLINE void
ylistl_add_next(struct ylistl_link *link, struct ylistl_link *anew) {
	ylistl_add(link, link->next, anew);
}

/** @see ylistl_add_next */
static YYINLINE void
ylistl_add_prev(struct ylistl_link *link, struct ylistl_link *anew) {
	ylistl_add(link->prev, link, anew);
}

/**
 * Add new node at the first position of list.
 *
 * @param head Head node of list
 * @param anew Node to add
 */
static YYINLINE void
ylistl_add_first(struct ylistl_link *head, struct ylistl_link *anew) {
	ylistl_add_next(head, anew);
}

/** @see ylistl_add_first */
static YYINLINE void
ylistl_add_last(struct ylistl_link *head, struct ylistl_link *anew) {
	ylistl_add_prev(head, anew);
}

/**
 * Delete link
 *
 * @param link Link
 */
static YYINLINE void
ylistl_remove(struct ylistl_link *link) {
	link->prev->next = link->next;
	link->next->prev = link->prev;
}

/**
 * Delete link of the first element of the list and return it.
 *
 * @param head Head of list
 * @return The first element
 */
static YYINLINE struct ylistl_link *
ylistl_remove_first(struct ylistl_link *head) {
	struct ylistl_link *lk = head->next;
	ylistl_remove(lk);
	return lk;
}

/**
 * Delete link of the last element of the list and return it.
 *
 * @param head Head of list
 * @return The last element
 */
static YYINLINE struct ylistl_link *
ylistl_remove_last(struct ylistl_link *head) {
	struct ylistl_link *lk = head->prev;
	ylistl_remove(lk);
	return lk;
}


/**
 * Replace @p old node with @p anew node
 *
 * @param old Node to be replaced with.
 * @param anew Node replacing @p old
 */
static YYINLINE void
ylistl_replace(struct ylistl_link *old, struct ylistl_link *anew) {
	anew->next = old->next;
	anew->next->prev = anew;
	anew->prev = old->prev;
	anew->prev->next = anew;
}

/****************************************************************************
 *
 * Macros
 *
 ****************************************************************************/
/**
 * Iterates link-node of the list.
 *
 * @param cur (struct @ref ylistl_link *) Iteration cursor
 * @param head (struct @ref ylistl_link *) Head of list
 */
#define ylistl_foreach(cur, head) \
        for ((cur) = (head)->next; (cur) != (head); (cur) = (cur)->next)

/**
 * Same with @ref ylistl_foreach. But direction is opposite.
 *
 * @param cur See @ref ylistl_foreach
 * @param head See @ref ylistl_foreach
 */
#define ylistl_foreach_reverse(cur, head) \
        for ((cur) = (head)->prev; (cur) != (head); (cur) = (cur)->prev)

/**
 * Same with @ref ylistl_foreach. And it is safe from removal.
 *
 * @param cur See @ref ylistl_foreach
 * @param tmp (struct @ref ylistl_link *) Temporary storage
 * @param head See @ref ylistl_foreach
 */
#define ylistl_foreach_safe(cur, tmp, head)		\
        for ((cur) = (head), (tmp) = (cur)->next;	\
	     (cur) != (head);				\
	     (cur) = (tmp), (tmp) = (cur)->next)

/**
 * Same with @ref ylistl_foreach_safe. But direction is opposite.
 *
 * @param cur See @ref ylistl_foreach_safe
 * @param tmp See @ref ylistl_foreach_safe
 * @param head See See @ref ylistl_foreach_safe
 */
#define ylistl_foreach_safe_reverse(cur, tmp, head)	\
        for ((cur) = (head), (tmp) = (cur)->prev;	\
	     (cur) != (head);				\
	     (cur) = (tmp), (tmp) = (cur)->prev)

/**
 * Iterates item of the list.
 *
 * @param cur (@p type *) iteration cursor
 * @param head (struct @ref ylistl_link *) the head for list
 * @param type Type of list item iterated by @p cur.
 * @param member Name of the @ref ylistl_link within item-struct
 */
#define ylistl_foreach_item(cur, head, type, member)			\
        for ((cur) = YYcontainerof((head)->next, type, member);		\
		&(cur)->member != (head);				\
		(cur) = YYcontainerof((cur)->member.next, type, member))

/**
 * Same with @ref ylistl_foreach_item. But direction is opposite.
 *
 * @param cur See @ref ylistl_foreach_item
 * @param head See @ref ylistl_foreach_item
 * @param type See @ref ylistl_foreach_item
 * @param member See @ref ylistl_foreach_item
 */
#define ylistl_foreach_item_reverse(cur, head, type, member)		\
        for ((cur) = YYcontainerof((head)->prev, type, member);		\
		&(cur)->member != (head);				\
		(cur) = YYcontainerof((cur)->member.prev, type, member))

/**
 * Same with @ref ylistl_foreach_item. And it is safe from remove operation.
 *
 * @param cur See @ref ylistl_foreach_item
 * @param tmp (@p type *) Temporary storage
 * @param head See @ref ylistl_foreach_item
 * @param type See @ref ylistl_foreach_item
 * @param member See @ref ylistl_foreach_item
 */
#define ylistl_foreach_item_safe(cur, tmp, head, type, member)    	\
        for ((cur) = YYcontainerof((head)->next, type, member),		\
			(tmp) = YYcontainerof((cur)->member.next,	\
				type, member); 				\
		&(cur)->member != (head);				\
		(cur) = (tmp),						\
			(tmp) = YYcontainerof((cur)->member.next, type, member))

/**
 * See @ref ylistl_foreach_item_safe. But direction is opposite.
 *
 * @param cur See @ref ylistl_foreach_item_safe
 * @param tmp See @ref ylistl_foreach_item_safe
 * @param head See @ref ylistl_foreach_item_safe
 * @param type See @ref ylistl_foreach_item_safe
 * @param member See @ref ylistl_foreach_item_safe
 */
#define ylistl_foreach_item_safe_reverse(cur, tmp, head, type, member)	\
        for ((cur) = YYcontainerof((head)->prev, type, member),		\
			(tmp) = YYcontainerof((cur)->member.prev,	\
				type, member); 				\
		&(cur)->member != (head);				\
		(cur) = (tmp),						\
			(tmp) = YYcontainerof((cur)->member.prev, type, member))

/****************************************************************************
 *
 * Rich(but heavy) functions
 *
 ****************************************************************************/
/**
 * Get size(number of items) of list.
 *
 * @param head Head node of the list.
 * @return size
 */
static YYINLINE int
ylistl_size(const struct ylistl_link *head) {
	struct ylistl_link *cur;
	int size = 0;
	ylistl_foreach(cur, head)
		size++;
	return size;
}

/**
 * Check @p node is in the list(@p head).
 *
 * @param head Head node of the list.
 * @param node Node to check.
 * @return bool TRUE or FALSE
 */
static YYINLINE bool
ylistl_contains(
	const struct ylistl_link *head,
	const struct ylistl_link *node
) {
	const struct ylistl_link *cur;
	ylistl_foreach(cur, head) {
		if (cur == node)
			return TRUE;
	}
	return FALSE;
}

/**
 * Delete link.
 * If @p node is NOT in the list headed by @p head, FALSE is returned.
 *
 * @param head Head of list.
 * @param node Node link.
 * @return TRUE or FALSE
 *
 */
static YYINLINE bool
ylistl_remove2(struct ylistl_link *head, struct ylistl_link *node) {
	if (!ylistl_contains(head, node))
		return FALSE;
	ylistl_remove(node);
	return TRUE;
}

/**
 * Delete link of the last element of the list and return it.
 * If list is empty, NULL is returned.
 *
 * @param head Head of list
 * @return The last element or NULL
 */
static YYINLINE struct ylistl_link *
ylistl_remove_last2(struct ylistl_link *head) {
	if (YYunlikely(ylistl_is_empty(head)))
		return NULL;
	return ylistl_remove_last(head);
}

/**
 * Delete link of the first element of the list and return it.
 * If list is empty, NULL is returned.
 *
 * @param head Head of list
 * @return The first element or NULL.
 */
static YYINLINE struct ylistl_link *
ylistl_remove_first2(struct ylistl_link *head) {
	if (YYunlikely(ylistl_is_empty(head)))
		return NULL;
	return ylistl_remove_first(head);
}

#endif /* __YLISTl_h__ */
