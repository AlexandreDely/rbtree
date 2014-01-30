

#include "inttree.h"

int main(int argc, char *argv[])
{
	struct rb_root root;
	struct int_rb_node *ze_root = NULL, *ze_pred = NULL;
	struct rb_node *prev = NULL;
	inttree_init_tree(&root);
	rb_insert(&root, 1);
	rb_insert(&root, 12);
	rb_insert(&root, 25);
	rb_insert(&root, 9);
	rb_insert(&root, 6);
	ze_root = rb_entry(root.root, struct int_rb_node, node);
	printf("Root is : %d\n", ze_root->k);
	prev = rb_predecessor(root.root);
	ze_pred = rb_entry(prev, struct int_rb_node, node);
	printf("Prev is : %d (%p)\n->l : %p | ->r : %p\n", ze_pred->k, prev, prev->l, prev->r);
	rb_erase_raw(root.root, &root);
	inttree_dump(&root);
	return 0;
}
