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


#include "ytree.h"
#include "yqueue.h"
#include "ystack.h"

#include "common.h"

/**
 * base struct of walker.
 * This SHOULD BE located on top of each walker struct.
 */
struct _tree_walker_base {
	/**< whitebox part */
	struct ytree_walker         wb;
	/**< type of walker - use "YTREE_WALKER_XXX" values */
	int                         type;
};

struct _tree_walker_preot {
	struct _tree_walker_base    base;
	struct ystack              *s;      /**< stack to use */
};

struct _tree_walker_levelot {
	struct _tree_walker_base    base;
	struct yqueue              *q;
};

struct _tree_walker_postot {
	struct _tree_walker_base    base;
	const struct ytreel_link   *top;
};

/********************************
 * Walker's operations.
 ********************************/
static struct ytree_node *
_tree_walker_preot_next(struct _tree_walker_preot *p) {
	struct ylistl_link *pos;
	yassert(p);
	ylistl_foreach_backward(pos, &p->base.wb.lnext->child) {
		ystack_push(p->s,
			    container_of(pos, struct ytreel_link, sibling));
	}
	p->base.wb.lcurr = p->base.wb.lnext;
	p->base.wb.lnext =
		ystack_is_empty(p->s)?
		NULL:
		(struct ytreel_link*)ystack_pop(p->s);

	return (p->base.wb.lcurr)?
		container_of(p->base.wb.lcurr, struct ytree_node, link):
		NULL;;
}

static struct ytree_node *
_tree_walker_r2lpreot_next(struct _tree_walker_preot *p) {
	struct ylistl_link *pos;
	yassert(p);
	ylistl_foreach(pos, &p->base.wb.lnext->child)
		ystack_push(p->s,
			    container_of(pos, struct ytreel_link, sibling));

	p->base.wb.lcurr = p->base.wb.lnext;
	p->base.wb.lnext = ystack_is_empty(p->s)?
			    NULL:
			    (struct ytreel_link*)ystack_pop(p->s);

	return (p->base.wb.lcurr)?
		container_of(p->base.wb.lcurr, struct ytree_node, link):
		NULL;;
}

static struct ytree_node *
_tree_walker_levelot_next(struct _tree_walker_levelot *p) {
	struct ylistl_link *pos;
	yassert(p);
	ylistl_foreach(pos, &p->base.wb.lnext->child)
		yqueue_en(p->q,
			  container_of(pos, struct ytreel_link, sibling));

	p->base.wb.lcurr = p->base.wb.lnext;
	p->base.wb.lnext = yqueue_is_empty(p->q)?
			    NULL:
			    (struct ytreel_link*)yqueue_de(p->q);

	return (p->base.wb.lcurr)?
		container_of(p->base.wb.lcurr, struct ytree_node, link):
		NULL;
}

static struct ytree_node *
_tree_walker_postot_next(struct _tree_walker_postot *p) {
	const struct ytreel_link *l = p->base.wb.lnext;
	yassert(p);
	if (l && l != p->top) {
		if (ytreel_has_next(l)) {
			l = ytreel_next(l);
			while(!ytreel_is_leaf(l))
				l = ytreel_first_child(l);
		} else
			l = l->parent;
	} else
		l = NULL;

	p->base.wb.lcurr = p->base.wb.lnext;
	p->base.wb.lnext = l;

	return (p->base.wb.lcurr)?
		container_of(p->base.wb.lcurr, struct ytree_node, link):
		NULL;
}

static struct ytree_node *
_tree_walker_r2lpostot_next(struct _tree_walker_postot *p) {
	const struct ytreel_link *l = p->base.wb.lnext;
	yassert(p);
	if (l && l != p->top) {
		if (ytreel_has_prev(l)) {
			l = ytreel_prev(l);
			while(!ytreel_is_leaf(l))
				l = ytreel_last_child(l);
		} else
			l = l->parent;
	} else
		l = NULL;
	p->base.wb.lcurr = p->base.wb.lnext;
	p->base.wb.lnext = l;

	return (p->base.wb.lcurr)?
		container_of(p->base.wb.lcurr, struct ytree_node, link):
		NULL;
}


struct ytree_walker*
ytree_walker_create(const struct ytree_node *top_node, int type) {
	struct _tree_walker_base *w = NULL;

	if (!top_node) {
		yassert(0);
		goto invalid_param;
	}

	switch(type) {
        case YTREE_WALKER_R2L_PRE_OT:
        case YTREE_WALKER_PRE_OT: {
		struct _tree_walker_preot *p;
		p = (struct _tree_walker_preot *)ymalloc(sizeof(*p));
		/*
		 * This stack SHOULD NOT be used to destroy item.
		 *Just set freecb as NULL
		 */
		p->s = ystack_create(NULL);
		if (YTREE_WALKER_PRE_OT == type)
			p->base.wb.next
				= (struct ytree_node *(*)(struct ytree_walker *))
				  &_tree_walker_preot_next;
		else
			p->base.wb.next
				= (struct ytree_node *(*)(struct ytree_walker *))
				  &_tree_walker_r2lpreot_next;

		p->base.wb.lnext = &top_node->link;
		w = (struct _tree_walker_base *)p;
        } break;

        case YTREE_WALKER_LEVEL_OT: {
		struct _tree_walker_levelot *p;
		p = (struct _tree_walker_levelot *)ymalloc(sizeof(*p));
		/* This stack SHOULD NOT be used to destroy item */
		p->q = yqueue_create(NULL);
		p->base.wb.next
			= (struct ytree_node *(*)(struct ytree_walker *))
			  &_tree_walker_levelot_next;
		p->base.wb.lnext = &top_node->link;
		w = (struct _tree_walker_base *)p;
        } break;

        case YTREE_WALKER_R2L_POST_OT:
        case YTREE_WALKER_POST_OT: {
		struct _tree_walker_postot *p;
		const struct ytreel_link *l;
		p = (struct _tree_walker_postot *)ymalloc(sizeof(*p));
		/*
		 * This stack SHOULD NOT be used to destroy item.
		   Just set freecb as NULL
		*/
		p->top = &top_node->link;
		l = p->top;

		if (YTREE_WALKER_POST_OT == type) {
			p->base.wb.next
				= (struct ytree_node *(*)(struct ytree_walker *))
				  &_tree_walker_postot_next;
			/*
			 * find initial(starting) value
			 * of post-order traverse
			 */
			while (!ytreel_is_leaf(l))
				l = ytreel_first_child(l);
		} else {
			p->base.wb.next
				= (struct ytree_node *(*)(struct ytree_walker *))
				  &_tree_walker_r2lpostot_next;
			/*
			 * find initial(starting) value
			 * of post-order traverse
			 */
			while (!ytreel_is_leaf(l))
				l = ytreel_last_child(l);
		}
		/* now l is leaf and this is starting value.*/
		p->base.wb.lnext = l;

		w = (struct _tree_walker_base *)p;
        } break;

        default:
		yassert(0);
		goto invalid_param;
	}


	w->type = type;
	w->wb.lcurr = NULL;

	return (struct ytree_walker *)w;

 invalid_param:
	yretset(YREInvalid_param);
	return NULL;
}

void
ytree_walker_destroy(struct ytree_walker *w) {

	if (!w) {
		yassert(0);
		goto invalid_param;
	}

	switch(((struct _tree_walker_base *)w)->type) {
        case YTREE_WALKER_R2L_PRE_OT:
        case YTREE_WALKER_PRE_OT: {
		struct _tree_walker_preot *p = (struct _tree_walker_preot *)w;
		ystack_destroy(p->s);
        } break;

        case YTREE_WALKER_LEVEL_OT: {
		struct _tree_walker_levelot *p
			= (struct _tree_walker_levelot *)w;
		yqueue_destroy(p->q);
        } break;

        case YTREE_WALKER_POST_OT:
        case YTREE_WALKER_R2L_POST_OT: {
		; /* Nothing to do specially */
        } break;
        default:
		yassert(0);
		goto invalid_param;
	}
	yfree(w);
	return;

 invalid_param:
	yretset(YREInvalid_param);
	return;
}


/*======================================================
 * Tree Implementation
 *======================================================*/
