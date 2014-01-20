

#include "inttree.h"

int main(int argc, char *argv[])
{
	struct rb_root root;
	inttree_init_tree(&root);
	rb_insert(&root, 1);
	rb_insert(&root, 12);
	rb_insert(&root, 25);
	rb_insert(&root, 9);
	rb_insert(&root, 6);
	inttree_dump(&root);
	return 0;
}
