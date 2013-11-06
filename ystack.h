/*****************************************************************************
 *    Copyright (C) 2011 Younghyung Cho. <yhcting77@gmail.com>
 *
 *    This file is part of ylib
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License
 *    (<http://www.gnu.org/licenses/lgpl.html>) for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.	If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/


/**
 * Operation to the empty list is not defined. It's consumer's responsibility!
 */

#ifndef __YSTACk_h__
#define __YSTACk_h__

#include "ylist.h"

struct ystack {
	struct ylist l;
};

static inline struct ystack *
ystack_create(void(*freecb)(void *)) {
	return (struct ystack *)ylist_create(freecb);
}

static inline struct ystack *
ystack_reset(struct ystack *s) {
	ylist_reset(&s->l);
	return s;
}

static inline void
ystack_destroy(struct ystack *s) {
	ylist_destroy(&s->l);
}

static inline struct ylist *
ystack_list(const struct ystack *s) {
	return &((struct ystack *)s)->l;
}

static inline struct ystack *
ystack_push(struct ystack *s, void *item) {
	ylist_add_last(&s->l, ylist_create_node(item));
	return s;
}

static inline void *
ystack_pop(struct ystack *s) {
	return ylist_free_node(ylist_del_last(&s->l));
}

static inline void *
ystack_peek(const struct ystack *s) {
	return ylist_peek_last(&s->l);
}

static inline int
ystack_is_empty(const struct ystack *s) {
	return ylist_is_empty(&s->l);
}

static inline unsigned int
ystack_size(const struct ystack *s) {
	return ylist_size(&s->l);
}

#endif /* __YSTACk_h__ */
