/******************************************************************************
 *    Copyright (C) 2011, 2012, 2013, 2014
 *    Younghyung Cho. <yhcting77@gmail.com>
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


#include "ytree.h"
#include "ylist.h"
#include "common.h"

/**
 * base struct of walker.
 * This SHOULD BE located on top of each walker struct.
 */
struct treei_base {
	/**< whitebox part */
	struct ytreei   itr;
	/**< type of iterator - use "YTREEI_XXX" values */
	int             type;
};

struct treei_preot {
	struct treei_base base;
	struct ylist     *s; /**< stack to use */
};

struct treei_levelot {
	struct treei_base base;
	struct ylist     *q; /**< queue */
};

struct treei_postot {
	struct treei_base    base;
	const struct ytreel_link  *top;
};

/********************************
 * Walker's operations.
 ********************************/
static struct ytree_node *
treei_preot_next(struct treei_preot *p) {
	struct ylistl_link *pos;
	yassert(p);
	ylistl_foreach_backward(pos, &p->base.itr.lnext->child) {
		ylist_push(p->s,
			   container_of(pos, struct ytreel_link, sibling));
	}
	p->base.itr.lcurr = p->base.itr.lnext;
	p->base.itr.lnext =
		ylist_is_empty(p->s)?
		NULL:
		(struct ytreel_link *)ylist_pop(p->s);

	return (p->base.itr.lcurr)?
		container_of(p->base.itr.lcurr, struct ytree_node, link):
		NULL;;
}

static struct ytree_node *
treei_r2lpreot_next(struct treei_preot *p) {
	struct ylistl_link *pos;
	yassert(p);
	ylistl_foreach(pos, &p->base.itr.lnext->child)
		ylist_push(p->s,
			   container_of(pos, struct ytreel_link, sibling));

	p->base.itr.lcurr = p->base.itr.lnext;
	p->base.itr.lnext = ylist_is_empty(p->s)?
		           NULL:
			   (struct ytreel_link *)ylist_pop(p->s);

	return (p->base.itr.lcurr)?
		container_of(p->base.itr.lcurr, struct ytree_node, link):
		NULL;;
}

static struct ytree_node *
treei_levelot_next(struct treei_levelot *p) {
	struct ylistl_link *pos;
	yassert(p);
	ylistl_foreach(pos, &p->base.itr.lnext->child)
		ylist_enq(p->q,
			  container_of(pos, struct ytreel_link, sibling));

	p->base.itr.lcurr = p->base.itr.lnext;
	p->base.itr.lnext = ylist_is_empty(p->q)?
			   NULL:
			   (struct ytreel_link*)ylist_deq(p->q);

	return (p->base.itr.lcurr)?
		container_of(p->base.itr.lcurr, struct ytree_node, link):
		NULL;
}

static struct ytree_node *
treei_postot_next(struct treei_postot *p) {
	const struct ytreel_link *l = p->base.itr.lnext;
	yassert(p);
	if (l && l != p->top) {
		if (ytreel_has_next(l)) {
			l = ytreel_next(l);
			while (!ytreel_is_leaf(l))
				l = ytreel_first_child(l);
		} else
			l = l->parent;
	} else
		l = NULL;

	p->base.itr.lcurr = p->base.itr.lnext;
	p->base.itr.lnext = l;

	return (p->base.itr.lcurr)?
		container_of(p->base.itr.lcurr, struct ytree_node, link):
		NULL;
}

static struct ytree_node *
treei_r2lpostot_next(struct treei_postot *p) {
	const struct ytreel_link *l = p->base.itr.lnext;
	yassert(p);
	if (l && l != p->top) {
		if (ytreel_has_prev(l)) {
			l = ytreel_prev(l);
			while (!ytreel_is_leaf(l))
				l = ytreel_last_child(l);
		} else
			l = l->parent;
	} else
		l = NULL;
	p->base.itr.lcurr = p->base.itr.lnext;
	p->base.itr.lnext = l;

	return (p->base.itr.lcurr)?
		container_of(p->base.itr.lcurr, struct ytree_node, link):
		NULL;
}


struct ytreei *
ytreei_create(const struct ytree_node *top_node, int type) {
	struct treei_base *itr = NULL;

	if (unlikely(!top_node)) {
		yassert(0);
		return NULL;
	}

	switch(type) {
        case YTREEI_R2L_PRE_OT:
        case YTREEI_PRE_OT: {
		struct treei_preot *p;
		p = (struct treei_preot *)ymalloc(sizeof(*p));
		/*
		 * This stack SHOULD NOT be used to destroy item.
		 *Just set freecb as NULL
		 */
		p->s = ylist_create(0, NULL);
		if (YTREEI_PRE_OT == type)
			p->base.itr.next
				= (struct ytree_node *(*)(struct ytreei *))
				  &treei_preot_next;
		else
			p->base.itr.next
				= (struct ytree_node *(*)(struct ytreei *))
				  &treei_r2lpreot_next;

		p->base.itr.lnext = &top_node->link;
		itr = (struct treei_base *)p;
        } break;

        case YTREEI_LEVEL_OT: {
		struct treei_levelot *p;
		p = (struct treei_levelot *)ymalloc(sizeof(*p));
		/* This stack SHOULD NOT be used to destroy item */
		p->q = ylist_create(0, NULL);
		p->base.itr.next
			= (struct ytree_node *(*)(struct ytreei *))
			  &treei_levelot_next;
		p->base.itr.lnext = &top_node->link;
		itr = (struct treei_base *)p;
        } break;

        case YTREEI_R2L_POST_OT:
        case YTREEI_POST_OT: {
		struct treei_postot *p;
		const struct ytreel_link *l;
		p = (struct treei_postot *)ymalloc(sizeof(*p));
		/*
		 * This stack SHOULD NOT be used to destroy item.
		   Just set freecb as NULL
		*/
		p->top = &top_node->link;
		l = p->top;

		if (YTREEI_POST_OT == type) {
			p->base.itr.next
				= (struct ytree_node *(*)(struct ytreei *))
				  &treei_postot_next;
			/*
			 * find initial(starting) value
			 * of post-order traverse
			 */
			while (!ytreel_is_leaf(l))
				l = ytreel_first_child(l);
		} else {
			p->base.itr.next
				= (struct ytree_node *(*)(struct ytreei *))
				  &treei_r2lpostot_next;
			/*
			 * find initial(starting) value
			 * of post-order traverse
			 */
			while (!ytreel_is_leaf(l))
				l = ytreel_last_child(l);
		}
		/* now l is leaf and this is starting value.*/
		p->base.itr.lnext = l;

		itr = (struct treei_base *)p;
        } break;

        default:
		yassert(0);
		return NULL;
	}


	itr->type = type;
	itr->itr.lcurr = NULL;

	return (struct ytreei *)itr;
}

int
ytreei_destroy(struct ytreei *itr) {
	if (unlikely(!itr)) {
		yassert(0);
		return -1;
	}

	switch(((struct treei_base *)itr)->type) {
        case YTREEI_R2L_PRE_OT:
        case YTREEI_PRE_OT: {
		struct treei_preot *p = (struct treei_preot *)itr;
		ylist_destroy(p->s);
        } break;

        case YTREEI_LEVEL_OT: {
		struct treei_levelot *p = (struct treei_levelot *)itr;
		ylist_destroy(p->q);
        } break;

        case YTREEI_POST_OT:
        case YTREEI_R2L_POST_OT: {
		; /* Nothing to do specially */
        } break;
        default:
		yassert(0);
		return -1;
	}
	yfree(itr);
	return 0;
}


/*======================================================
 * Tree Implementation
 *======================================================*/
