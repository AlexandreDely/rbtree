
#include "rbtree.h"

static void rotate_left(struct rb_node *n);
static void rotate_right(struct rb_node *n);
static inline struct rb_node *rb_sibling(struct rb_node *n);

static inline struct rb_node *rb_sibling(struct rb_node *n)
{
	if(!n || !n->p)
		return NULL;
	return (n == n->p->l) ? n->p->r : n->p->l;
}

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

void rb_erase(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *c = NULL
	if(!node || !root)
		return;
	if(RB_COLOR_RED == node->clr) {
		if(node->l && node->r) {
		}
	}
}
