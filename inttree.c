
#include "inttree.h"
#include <string.h>

static struct rb_node *inttree_alloc_node(va_list args);
static void inttree_free_node(struct rb_node *n);
static int inttree_order(struct rb_node *n1, struct rb_node *n2);



static struct rb_tree_tmpl inttree_tmpl = {
		.alloc_node = inttree_alloc_node,
		.free_node = inttree_free_node,
		.ins_preprocess = NULL,
		.del_preprocess = NULL,
		.order = inttree_order
};

void inttree_init_tree(struct rb_root *tree)
{
	if(!tree)
		return;
	tree->tmpl = &inttree_tmpl;
	tree->root = NULL;
}

struct rb_root *inttree_create(void)
{
	struct rb_root *tree = NULL;
	if(!(tree = malloc(sizeof (struct rb_root)))) {
		return NULL;
	}
	inttree_init_tree(tree);
	return tree;
}

static struct rb_node *inttree_alloc_node(va_list args)
{
	int k = 0;
	k = va_arg(args, int);
	struct int_rb_node *node = NULL;
	if(!(node = malloc(sizeof (struct int_rb_node)))) {
		return NULL;
	}
	node->k = k;
	memset(&node->node, 0, sizeof (struct rb_node));
	return &node->node;
}

static void inttree_free_node(struct rb_node *n)
{
	if(!n)
		return;
	free(rb_entry(n, struct int_rb_node, node));
}

static int inttree_order(struct rb_node *n1, struct rb_node *n2)
{
	struct int_rb_node *a1 = NULL, *a2 = NULL;
	if(!n1 && !n2)
		return 0;
	if(!n1)
		return -1;
	if(!n2)
		return 1;

	a1 = rb_entry(n1, struct int_rb_node, node);
	a2 = rb_entry(n2, struct int_rb_node, node);
	return (a1->k >= a2->k) - (a2->k >= a1->k);
}

void inttree_dump(struct rb_root *root)
{
	int i = 0;
	struct rb_node *n = rb_leftmost(root);
	struct int_rb_node *int_n;
	while(n) {
		int_n = rb_entry(n, struct int_rb_node, node);
		printf("Node #%d : %d\n", i, int_n->k);
		n = rb_successor(n);
		i++;
	}
	return;
}
