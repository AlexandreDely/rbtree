
#ifndef _INTTREE_H_
#define _INTTREE_H_

#include "rbtree.h"

struct int_rb_node {
	int k;
	struct rb_node node;
};

void inttree_init_tree(struct rb_root *tree);
struct rb_root *inttree_create(void);
void inttree_dump(struct rb_root *root);

#endif // _INTTREE_H_
