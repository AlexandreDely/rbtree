
#include "rbtree.h"
#include <string.h>

static void rotate_left(struct rb_node *n);
static void rotate_right(struct rb_node *n);
static void rb_swap_nodes(struct rb_node *n1, struct rb_node *n2);
static inline struct rb_node *rb_sibling(struct rb_node *n);

static inline struct rb_node *rb_sibling(struct rb_node *n)
{
	if(!n || !n->p)
		return NULL;
	return (n == n->p->l) ? n->p->r : n->p->l;
}

/**
 * rb_swap_nodes - Swap two nodes brutally
 * @n1: Target node #1
 * @n2: Target node #2
 *
 * The two nodes are swapped, breaking the order in the tree.
 * This functions is ONLY used for the deletion procedure,
 * where the order will be set back up.
 */
static void rb_swap_nodes(struct rb_node *n1, struct rb_node *n2)
{
	struct rb_node tmp;
	if(!n1 || !n2)
		return;

	memcpy(&tmp, sizeof (struct rb_node), n1);
	memcpy(n1, sizeof (struct rb_node), n2);
	memcpy(n2, sizeof (struct rb_node), &tmp);
}

/**
 * rb_predecessor - Find the predecessor of a node
 * @n: The node whose predecessor we're looking for
 *
 * This functions returns NULL if the node @n
 * has not any predecessor (which means @n is the left-most
 * node in the tree). Otherwise, a pointer to the appropriate
 * node object is returned.
 */
struct rb_node *rb_predecessor(struct rb_node *n)
{
	struct rb_node *prev = NULL, *orig = NULL, *p = NULL;
	if(!n)
		return NULL;
	if(!n->l) {
		orig = n;
		while(NULL != (p = orig->p)) {
			if(p->l == orig) {
				orig = p;
			} else if(p->r == orig) {
				return p;
			}
		}
		/*
		 * Here we reached the root of the tree
		 * without finding a predecessor for
		 * the target node ...
		 */
		return NULL;
	} else {
		p = n->l;
		while(NULL != (orig = p->r)) {
			p = orig;
		}
		return p;
	}
}

/**
 * rb_successor - Find the successor of a node
 * @n: The node whose successor we're looking for
 *
 * This returns NULL if such a successor does not exist
 * (i.e., @n is the right-most node in the tree).
 *
 * The successor s of @n is the element such that :
 * For all element e in the tree, with e > @n,
 * s <= e
 */
struct rb_node *rb_successor(struct rb_node *n)
{
	struct rb_node *prev = NULL, *orig = NULL, *p = NULL;
	if(!n)
		return NULL;
	if(!n->r) {
		orig = n;
		while(NULL != (p = orig->p)) {
			if(p->r == orig) {
				orig = p;
			} else if(p->l == orig) {
				return p;
			}
		}
		/*
		 * Here we reached the root of the tree
		 * without finding a predecessor for
		 * the target node ...
		 */
		return NULL;
	} else {
		p = n->r;
		while(NULL != (orig = p->l)) {
			p = orig;
		}
		return p;
	}
}

/**
 * rb_insert_raw - Insert one node in a RB-tree
 * @root: The tree in which the node will be
 * @n: The node about to be inserted
 *
 * This function inserts an already allocated node @n
 * in the tree @root. Beware that depending on the top-layer
 * of the tree, using this function without proper preprocessing
 * can violate rules maintained by the said top layer ...
 */
int rb_insert_raw(struct rb_root *root, struct rb_node *n)
{
	int order = 0;
	struct rb_node **p = &(root->root), *parent = NULL;
	while(*p) {
		parent = *p;
		order = root->tmpl->order(n, (*p));
		if(0 < order) {
			*p = (*p)->l;
		} else if(0 > order) {
			*p = (*p)->r;
		} else {
			return -ENODATA;
		}
	}
	rb_link_node(n, parent, p);
	rb_balance(n, root);
	return 0;
}

/**
 * rb_insert_v - The va_list version of @rb_insert
 * @tree: The tree where the insertion occurs
 * @args: The list of arguments
 *
 * See @rb_insert for a description.
 */
int rb_insert_v(struct rb_root *tree, va_list args)
{
	int order = 0, ret = 0;
	struct rb_node *node = NULL, *parent = NULL;
	struct rb_node **p = &tree->root;
	node = tree->tmpl->alloc_node(args);
	if(!tree || !tree->tmpl || (ret = tree->tmpl->ins_preprocess(tree, node))) {
		return (ret) ? ret : -EINVAL;
	}
	ret = rb_insert_raw(tree, node);
	if(-ENODATA == ret) {
		tree->tmpl->free_node(node);
	}
	return 0;
}

/**
 * rb_insert - Perform an element insertion
 * @tree: The tree in which the insertion occurs
 * ...: Variable insertion arguments
 *
 * The variable arguments are here to let the developer
 * pass all the needed arguments for a **one-element** insertion.
 * The arguments should not contain an allocated node, the tree's template functions
 * should take care of the proper wiring ...
 */
int rb_insert(struct rb_root *tree, ...)
{
	va_list args;
	int ret = 0;
	va_start(args, tree);
	ret = rb_insert_v(tree, args);
	va_end(args);
	return ret;
}

static void rotate_left(struct rb_node *n)
{
	struct rb_node *r = NULL, *p = NULL, *tmp = NULL;
	if(!n || !n->r)
		return;

	p = n->p;
	r = n->r;
	tmp = r->l;

	/* Perform the "rotation" */
	r->l = n;
	n->r = tmp;

	/* Updating the parent nodes */
	r->p = n;
	n->p = r;
	if(tmp)
		tmp->p = n;
}

static void rotate_right(struct rb_node *n)
{
	struct rb_node *l = NULL, *tmp = NULL, *p = NULL;
	if(!n || !n->l)
		return;
	/* Storing the needed nodes */
	p = n->p;
	l = n->l;
	tmp = l->r;

	/* Perform the "rotation" */
	l->r = n;
	n->l = tmp;

	/* Updating the parent nodes */
	l->p = p;
	n->p = l;
	if(tmp)
		tmp->p = n;
}

/**
 * rb_balance - Balance a RB-tree after a one-node insertion
 * @node: The node that was inserted
 * @root: The tree where the node now is
 *
 * This function returns a negative error code depending on
 * what happened or 0 if it exited successfully.
 */
int rb_balance(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *g = NULL, *p = NULL, *u = NULL;
	int uprising = 1;
	if(!node || !root) {
		return -EINVAL;
	}
	node->clr = RB_COLOR_RED;
	while(uprising) {
		if(!node->p) {
			node->clr = RB_COLOR_BLACK;
			return 0;
		}
		if(RB_COLOR_BLACK == node->p->clr) {
			return 0;
		}
		p = node->p;
		g = p->p;
		u = (p != g->l) ? g->l : g->r;
		if(RB_COLOR_RED == u->clr) {
			p->clr = RB_COLOR_BLACK;
			u->clr = RB_COLOR_BLACK;
			g->clr = RB_COLOR_RED;
			node = g;
		} else {
			uprising = 0;
		}
	}
	if((p->r == node) && (g->l == p)) {
		rotate_left(p);
		p = node;
		g = node->p;
		node = node->l;
	} else if((p->l == node) && (g->r == p)) {
		rotate_right(p);
		p = node;
		g = node->p;
		node = node->r;
	}
	p->clr = RB_COLOR_BLACK;
	g->clr = RB_COLOR_RED;
	if(node == p->l) {
		rotate_right(g);
	} else {
		rotate_left(g);
	}
	return 0;
}

/**
 * rb_erase - Erase a node from a RB-tree
 * @node: The node to erase
 * @root: The tree to which the node belongs
 */
void rb_erase(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *c = NULL, *neighbor = NULL, *p = NULL, *s = NULL;
	int uprising = 1;
	if(!node || !root || !root->tmpl || !root->tmpl->free_node)
		return;
	if(node->l && node->r) {
		neighbor = rb_predecessor(node);
		rb_swap_nodes(node, neighbor);
	}
	c = (NULL == node->l) ? node->r : node->l;
	if(RB_COLOR_RED == node->clr) {
		if(c)
			c->p = node->p;
		root->tmpl->free_node(node);
		return;
	} else if((RB_COLOR_BLACK == node->clr) && c && (RB_COLOR_RED == c->clr)) {
		/*
		 * If node is black, and has one child, it must be red ...
		 */
		c->p = node->p;
		c->clr = RB_COLOR_BLACK;
		root->tmpl->free_node(node);
		return;
	}
	/*
	 * Tricky case :
	 * The node is black and is childless (leaf-children).
	 */
	p = node->p;
	s = rb_sibling(node);
	root->tmpl->free_node(node);
	while(uprising) {
		if(!p) {
			/*
			 * The tree only has a root ...
			 * We can delete it right away
			 */
			root->root = NULL;
			return;
		}
		if(RB_COLOR_RED == s->clr) {
			p->clr = RB_COLOR_RED;
			s->clr = RB_COLOR_BLACK;
			if(p->l == node) {
				p->l = NULL;
				rotate_left(p);
			} else {
				p->r = NULL;
				rotate_right(p);
			}
		}
		if(RB_COLOR_BLACK == s->clr && RB_COLOR_BLACK == p->clr &&
				(!s->l || RB_COLOR_BLACK == s->l->clr) &&
				(!s->r || RB_COLOR_BLACK == s->r->clr)) {
			s->clr = RB_COLOR_RED;
			node = p;
			p = node->p;
			s = rb_sibling(node);
		} else {
			uprising = 0;
		}
	}
	/*
	 * At this point, node must NOT be dereferenced
	 */
	if(RB_COLOR_RED == p->clr && s->clr == RB_COLOR_BLACK &&
			(!s->l || RB_COLOR_BLACK == s->l->clr) &&
			(!s->r || RB_COLOR_BLACK == s->r->clr)) {
		p->clr = RB_COLOR_BLACK;
		s->clr = RB_COLOR_RED;
		return;
	}
	if(RB_COLOR_BLACK == s->clr) {
		if((p->l == node) &&
			(!s->r || RB_COLOR_BLACK == s->r->clr) &&
			(s->l && RB_COLOR_RED == s->l->clr)) {
			s->clr = RB_COLOR_RED;
			s->l->clr = RB_COLOR_BLACK;
			rotate_right(s);
		} else if((p->r == node) &&
			(!s->l || RB_COLOR_BLACK == s->l->clr) &&
			(s->r && RB_COLOR_RED == s->r->clr)) {
			s->clr = RB_COLOR_RED;
			s->r->clr = RB_COLOR_BLACK;
			rotate_left(s);
		}
		s->clr = p->clr;
		p->clr = RB_COLOR_BLACK;
		if(node == p->l) {
			if(s->r)
				s->r->clr = RB_COLOR_BLACK;
			rotate_left(p);
		} else {
			if(s->l)
				s->l->clr = RB_COLOR_BLACK;
			rotate_right(p);
		}
	}
}
