
#include "rbtree.h"
#include <stdarg.h>
#include <string.h>

static void rotate_right(struct rb_node *n, struct rb_root *root);
static void rotate_left(struct rb_node *n, struct rb_root *root);
static void rb_swap_nodes(struct rb_root *root,
		struct rb_node *n1, struct rb_node *n2);
static inline struct rb_node *rb_sibling(struct rb_node *n);
static void rb_delete_one_child(struct rb_node *node, struct rb_root *root);
static int rb_black_count(struct rb_node *node);

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
static void rb_swap_nodes(struct rb_root *root,
		struct rb_node *n1, struct rb_node *n2)
{
	struct rb_node tmp;
	struct rb_node *p1 = NULL, *p2 = NULL;
	if(!n1 || !n2)
		return;
	p1 = n1->p;
	p2 = n2->p;
	if(p1) {
		if(n1 == p1->l)
			p1->l = n2;
		else
			p1->r = n2;
	} else {
		root->root = n2;
	}
	if(p2) {
		if(n2 == p2->l)
			p2->l = n1;
		else
			p2->r = n1;
	} else {
		root->root = n1;
	}
	memcpy(&tmp, n1, sizeof (struct rb_node));
	memcpy(n1, n2, sizeof (struct rb_node));
	memcpy(n2, &tmp, sizeof (struct rb_node));
	if(n1->r)
		n1->r->p = n1;
	if(n1->l)
		n1->l->p = n1;
	if(n2->r)
		n2->r->p = n2;
	if(n2->l)
		n2->l->p = n2;
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
	struct rb_node *orig = NULL, *p = NULL;
	if(!n)
		return NULL;
	if(!n->l) {
		orig = n;
		while((p = orig->p)) {
			if(p->l == orig) {
				orig = p;
			} else if(p->r == orig) {
				return p;
			} else {
				printf("Houston, we've got a problem\n");
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
	struct rb_node *orig = NULL, *p = NULL;
	if(!n)
		return NULL;
	if(!n->r) {
		orig = n;
		while(NULL != (p = orig->p)) {
			if(p->r == orig) {
				orig = p;
			} else if(p->l == orig) {
				return p;
			} else {
				printf("Problem here\n");
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

struct rb_node *rb_leftmost(struct rb_root *root)
{
	struct rb_node *n = NULL;
	if(!root)
		return NULL;
	n = root->root;
	while(n && n->l) {
		n = n->l;
	}
	return n;
}

struct rb_node *rb_rightmost(struct rb_root *root)
{
	struct rb_node *n = NULL;
	if(!root)
		return NULL;
	n = root->root;
	while(n && n->r) {
		n = n->r;
	}
	return n;
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
		if(0 > order) {
			p = &((*p)->l);
		} else if(0 < order) {
			p = &((*p)->r);
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
	int ret = 0;
	struct rb_node *node = NULL;
	node = tree->tmpl->alloc_node(args);
	if(!tree || !tree->tmpl ||
		(tree->tmpl->ins_preprocess &&
				(ret = tree->tmpl->ins_preprocess(tree, node)))) {
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

static void rotate_left(struct rb_node *n, struct rb_root *root)
{
	struct rb_node *r = NULL, *p = NULL, *tmp = NULL;
	int left = 0;
	if(!n || !n->r || !root)
		return;

	p = n->p;
	r = n->r;
	tmp = r->l;
	left = (p && (n == p->l));
	/* Perform the "rotation" */
	r->l = n;
	n->r = tmp;

	/* Updating the parent nodes */
	r->p = p;
	n->p = r;
	if(tmp)
		tmp->p = n;
	if(!p)
		root->root = r;
	else {
		if(left) {
			p->l = r;
		} else {
			p->r = r;
		}
	}
}

static void rotate_right(struct rb_node *n, struct rb_root *root)
{
	struct rb_node *l = NULL, *tmp = NULL, *p = NULL;
	int left;
	if(!n || !n->l || !root)
		return;
	/* Storing the needed nodes */
	p = n->p;
	l = n->l;
	tmp = l->r;
	left = (p && (n == p->l));
	/* Perform the "rotation" */
	l->r = n;
	n->l = tmp;

	/* Updating the parent nodes */
	l->p = p;
	n->p = l;
	if(tmp)
		tmp->p = n;
	if(!p)
		root->root = l;
	else {
		if(left) {
			p->l = l;
		} else {
			p->r = l;
		}
	}
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
		if(u && RB_COLOR_RED == u->clr) {
			p->clr = RB_COLOR_BLACK;
			u->clr = RB_COLOR_BLACK;
			g->clr = RB_COLOR_RED;
			node = g;
		} else {
			uprising = 0;
		}
	}
	/* Probleme ici ? */
	if((p->r == node) && (g->l == p)) {
		rotate_left(p, root);
		node = node->l;
		p = node->p;
		g = p->p;
	} else if((p->l == node) && (g->r == p)) {
		rotate_right(p, root);
		node = node->r;
		p = node->p;
		g = p->p;
	}
	p->clr = RB_COLOR_BLACK;
	g->clr = RB_COLOR_RED;
	if(node == p->l) {
		rotate_right(g, root);
	} else {
		rotate_left(g, root);
	}
	return 0;
}

/**
 * rb_erase_free - Erase a node from a RB-tree and free memory
 * @node: The node to erase
 * @root: The tree to which the node belongs
 *
 * This function will remove @node from @root using @rb_erase_raw.
 * It will then free the memory occupied by @node, via the template function
 * (see @rb_tree_tmpl).
 */
void rb_erase_free(struct rb_node *node, struct rb_root *root)
{
	if(!node || !root || !root->tmpl || !root->tmpl->free_node)
		return;
	rb_erase_raw(node, root);
	root->tmpl->free_node(node);
}


static void rb_delete_one_child(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *p = NULL, *c = NULL;
	if(!node || !root)
		return;
	p = node->p;
	c = (NULL == node->l) ? node->r : node->l;
	if(p) {
		if(node == p->l)
			p->l = c;
		if(node == p->r)
			p->r = c;
		if(c)
			c->p = p;
	} else {
		root->root = c;
		if(c)
			c->p = NULL;
	}
}

/**
 * rb_erase - Erase a node from a RB-tree
 * @node: The node to erase
 * @root: The tree to which the node belongs
 */
void rb_erase_raw(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *c = NULL, *neighbor = NULL, *p = NULL, *s = NULL;
	struct rb_node *tgt = NULL;
	int uprising = 1;
	if(!node || !root || !root->tmpl || !root->tmpl->free_node)
		return;
	if(node->l && node->r) {
		neighbor = rb_predecessor(node);
		rb_swap_nodes(root, node, neighbor);
	}
	p = node->p;
	c = (NULL == node->l) ? node->r : node->l;
	if(RB_COLOR_RED == node->clr) {
		rb_delete_one_child(node, root);
		return;
	} else if((RB_COLOR_BLACK == node->clr) && c && (RB_COLOR_RED == c->clr)) {
		/*
		 * If node is black, and has one child, it must be red ... ???
		 */
		c->p = p;
		c->clr = RB_COLOR_BLACK;
		rb_delete_one_child(node, root);
		return;
	}
	/*
	 * Tricky case :
	 * The node is black and is childless (leaf-children).
	 */
	tgt = node;
	s = rb_sibling(node);
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
				rotate_left(p, root);
			} else {
				rotate_right(p, root);
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
		rb_delete_one_child(tgt, root);
		return;
	}
	if(RB_COLOR_BLACK == s->clr) {
		if((p->l == node) &&
			(!s->r || RB_COLOR_BLACK == s->r->clr) &&
			(s->l && RB_COLOR_RED == s->l->clr)) {
			s->clr = RB_COLOR_RED;
			s->l->clr = RB_COLOR_BLACK;
			rotate_right(s, root);
		} else if((p->r == node) &&
			(!s->l || RB_COLOR_BLACK == s->l->clr) &&
			(s->r && RB_COLOR_RED == s->r->clr)) {
			s->clr = RB_COLOR_RED;
			s->r->clr = RB_COLOR_BLACK;
			rotate_left(s, root);
		}
		s->clr = p->clr;
		p->clr = RB_COLOR_BLACK;
		if(node == p->l) {
			if(s->r)
				s->r->clr = RB_COLOR_BLACK;
			rotate_left(p, root);
		} else {
			if(s->l)
				s->l->clr = RB_COLOR_BLACK;
			rotate_right(p, root);
		}
		rb_delete_one_child(tgt, root);
	}
}

/**
 * rb_clear - Clear a RB-tree
 * @root: The tree to clear
 *
 * This function will simply delete any content
 * in the tree @root, using @rb_erase_free.
 */
void rb_clear(struct rb_root *root)
{
	if(!root)
		return;
	while(NULL != root->root) {
		rb_erase_free(root->root, root);
	}
}

/**
 * rb_merge - Merge two RB trees
 * @dst: The hosting tree
 * @src: The tree to merge with @dst
 *
 * This function moves the nodes in @src to @dst.
 * Finally, @src will be an empty tree whereas @dst will host
 * the merge result.
 */
int rb_merge(struct rb_root *dst, struct rb_root *src)
{
	int err = 0;
	struct rb_node *node = NULL;
	if(!dst || !src || (dst->tmpl != src->tmpl)) {
		return -EINVAL;
	}
	while(NULL != src->root) {
		node = src->root;
		rb_erase_raw(node, src);
		err = rb_insert_raw(dst, node);
		if(err) {
			break;
		}
	}
	return err;
}

/**
 * rb_find - Find a node using its charateristic parameters
 * @root: The tree in which to look up
 * @...: The parameters that identify the targeted node
 *
 * This function uses @rb_find_v to perform the work.
 * It allocates a temporary node to allow node comparison
 * using the order function. The tmp node is freed before
 * the function returns.
 *
 * As a consequence, the characteristic arguments are the ones
 * needed to allocate the node.
 */
struct rb_node *rb_find(struct rb_root *root, ...)
{
	va_list args;
	struct rb_node *n = NULL;
	va_start(args, root);
	n = rb_find_v(root, args);
	va_end(args);
	return n;
}

struct rb_node *rb_find_v(struct rb_root *root, va_list args)
{
	int order = 0;
	struct rb_node *tgt = NULL, *n = NULL;
	if(!root || !root->tmpl)
		return NULL;
	if(!(tgt = root->tmpl->alloc_node(args))) {
		return NULL;
	}
	n = root->root;
	while(n) {
		order = root->tmpl->order(tgt, n);
		if(0 > order) {
			n = n->l;
		} else if(0 < order) {
			n = n->r;
		} else {
			root->tmpl->free_node(tgt);
			return n;
		}
	}
	root->tmpl->free_node(tgt);
	return NULL;
}

static int rb_black_count(struct rb_node *node)
{
	int cnt = 0, tmp;
	if(!node)
		return 0;
	if(node->clr == RB_COLOR_BLACK) {
		cnt++;
	} else {
		if((node->l && node->l->clr == RB_COLOR_RED) ||
			(node->r && node->r->clr == RB_COLOR_RED)) {
			return -1;
		}
	}
	if(0 > (tmp = rb_black_count(node->l))) {
		return tmp;
	}
	if(tmp != rb_black_count(node->r)) {
		return -1;
	} else {
		return cnt + tmp;
	}
}

int rb_tree_is_sane(struct rb_root *tree)
{
	int res = 1;
	struct rb_node *n = NULL;
	if(!tree)
		return 0;
	if(!tree->root)
		return 0;
	res = rb_black_count(tree->root);
	return res;
}
