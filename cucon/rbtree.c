/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cucon/rbtree.h>
#include <cu/memory.h>
#include <cu/int.h>
#include <cu/ptr.h>
#include <assert.h>
#include <string.h>

/* Expensive check, define only for debugging! */
/* #define CHECK_TREE */

#define RIGHT(node) ((node)->right)
#define SET_RIGHT(node, r) ((node)->right = (r))
#ifdef CUCON_RBTREE_ENBALE_PACKED_COLOUR /* will most probably not be used */
#  define LEFT(node)					\
	((cucon_rbnode_t)((uintptr_t)(node)->left & ~(uintptr_t)1))
#  define SET_LEFT(node, l) \
	((node)->left = \
	 (cucon_rbnode_t)((uintptr_t)l | IS_RED(node)))
#  define IS_RED(node) (((uintptr_t)(node)->left) & 1)
#  define BECOME_RED(node)					\
	((node)->left = (cucon_rbnode_t)((uintptr_t)(node)->left | 1))
#  define BECOME_BLACK(node)				\
	((node)->left =							\
	 ((cucon_rbnode_t)((uintptr_t)(node)->left & ~(uintptr_t)1)))
#else
#  define LEFT(node) ((node)->left)
#  define SET_LEFT(node, l) ((node)->left = (l))
#  define IS_BLACK(node) ((node)->is_black)
#  define BECOME_BLACK(node) ((node)->is_black = 1)
#  define BECOME_RED(node) ((node)->is_black = 0)
#endif
#define IS_RED(node) !IS_BLACK(node)

void
cucon_rbtree_init(cucon_rbtree_t tree)
{
    tree->root = NULL;
}

cucon_rbtree_t
cucon_rbtree_new(void)
{
    cucon_rbtree_t tree = cu_gnew(struct cucon_rbtree);
    tree->root = NULL;
    return tree;
}

void
cucon_rbtree_cct_copy_ctive(cucon_rbtree_t tree, cucon_rbtree_t src)
{
    tree->root = src->root;
}

cucon_rbtree_t
cucon_rbtree_new_copy_ctive(cucon_rbtree_t src)
{
    cucon_rbtree_t tree = cu_gnew(struct cucon_rbtree);
    tree->root = src->root;
    return tree;
}

CU_SINLINE cucon_rbnode_t
_copy_ptrnode(cucon_rbnode_t src)
{
    cucon_rbnode_t dst = cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_rbnode)
				   + sizeof(void *));
    dst->is_black = src->is_black;
    *(void **)cucon_rbnode_mem(dst) = *(void **)cucon_rbnode_mem(src);
    return dst;
}

cucon_rbnode_t
cucon_rbnode_new_copy(cucon_rbnode_t src, size_t slot_size)
{
    cucon_rbnode_t dst
	= cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_rbnode) + slot_size);
    dst->is_black = src->is_black;
#ifndef NDEBUG
    dst->left = (void *)-1;
    dst->right = (void *)-1;
#endif
    return dst;
}

static void
cuconP_rbnode_copy_rec_node(cucon_rbnode_t src, cucon_rbnode_t *dst,
			  cucon_rbnode_copier_t node_new_copy)
{
    while (src) {
	*dst = cu_call(node_new_copy, src);
	cuconP_rbnode_copy_rec_node(LEFT(src), &LEFT(*dst), node_new_copy);
	src = RIGHT(src);
	dst = &RIGHT(*dst);
    }
    *dst = NULL;
}
void
cucon_rbtree_cct_copy_node(cucon_rbtree_t dst, cucon_rbtree_t src,
			 cucon_rbnode_copier_t node_new_copy)
{
    cuconP_rbnode_copy_rec_node(src->root, &dst->root, node_new_copy);
}
cucon_rbtree_t
cucon_rbtree_new_copy_node(cucon_rbtree_t src, cucon_rbnode_copier_t node_new_copy)
{
    cucon_rbtree_t tree = cu_gnew(struct cucon_rbtree);
    cucon_rbtree_cct_copy_node(tree, src, node_new_copy);
    return tree;
}

static void
cuconP_rbnode_copy_rec_ptr(cucon_rbnode_t src, cucon_rbnode_t *dst)
{
    while (src) {
	*dst = _copy_ptrnode(src);
	cuconP_rbnode_copy_rec_ptr(LEFT(src), &LEFT(*dst));
	src = RIGHT(src);
	dst = &RIGHT(*dst);
    }
    *dst = NULL;
}
void
cucon_rbtree_cct_copy_ptr(cucon_rbtree_t dst, cucon_rbtree_t src)
{
    cuconP_rbnode_copy_rec_ptr(src->root, &dst->root);
}

cucon_rbtree_t
cucon_rbtree_new_copy_ptr(cucon_rbtree_t src)
{
    cucon_rbtree_t dst = cu_gnew(struct cucon_rbtree);
    cuconP_rbnode_copy_rec_ptr(src->root, &dst->root);
    return dst;
}

cu_clos_def(_merge_ptr_cb, cu_prot(void, void *value),
	    (cucon_rbtree_t dst; cu_clop(cmp, int, void *, void *);))
{
    cu_clos_self(_merge_ptr_cb);
    cucon_rbtree_insert2p_ptr(self->dst, self->cmp, value);
}
void
cucon_rbtree_merge2p(cucon_rbtree_t dst, cucon_rbtree_t src,
		       cu_clop(cmp, int, void *, void *))
{
    _merge_ptr_cb_t cb;
    cb.dst = dst;
    cb.cmp = cmp;
    cucon_rbtree_iter_ptr(src, _merge_ptr_cb_prep(&cb));
}

cu_clos_def(_merge_ptr_ctive_cb, cu_prot(void, void *value),
	    (cucon_rbtree_t dst; cu_clop(cmp, int, void *, void *);))
{
    cu_clos_self(_merge_ptr_ctive_cb);
    cucon_rbtree_insert2p_ptr(self->dst, self->cmp, value);
}
void
cucon_rbtree_cmerge2p(cucon_rbtree_t dst, cucon_rbtree_t src,
			     cu_clop(cmp, int, void *, void *))
{
    _merge_ptr_ctive_cb_t cb;
    cb.dst = dst;
    cb.cmp = cmp;
    cucon_rbtree_iter_ptr(src, _merge_ptr_ctive_cb_prep(&cb));
}

#ifdef CHECK_TREE
static void
cuconP_rbnode_check(cucon_rbnode_t x, int black_count, size_t *node_count)
{
tail_rec:
    if (x == NULL) {
	assert(black_count == 1);
	return;
    }
    ++*node_count;
    if (IS_RED(x)) {
	assert(!LEFT(x) || IS_BLACK(LEFT(x)));
	assert(!RIGHT(x) || IS_BLACK(RIGHT(x)));
    }
    else
	--black_count;
    cuconP_rbnode_check(LEFT(x), black_count, node_count);
    x = RIGHT(x);
    goto tail_rec;
}
static void
cuconP_rbtree_check(cucon_rbtree_t tree)
{
    cucon_rbnode_t x;
    size_t node_count = 0;
    int black_count = 1;
    x = tree->root;
    if (!x)
	return;
    while (x) {
	if (IS_BLACK(x))
	    ++black_count;
	x = x->left;
    }
    cuconP_rbnode_check(tree->root, black_count, &node_count);
    assert(node_count == tree->root->is_black);
}
static size_t
cuconP_rbnode_count(cucon_rbnode_t node)
{
    if (!node)
	return 0;
    else
	return 1
	    + cuconP_rbnode_count(LEFT(node))
	    + cuconP_rbnode_count(RIGHT(node));
}
#endif


/*	    z		x		⟵ sp[0]
 *	  /	==RR=>	  \
 *	x		    z
 *	 \	<=RL==	   /
 *	  y		  y
 */
CU_SINLINE void
cuconP_rbnode_rotate_right(cucon_rbnode_t *sp)
{
    cucon_rbnode_t z = sp[0];
    cucon_rbnode_t x = LEFT(z);
    SET_LEFT(z, RIGHT(x));

    if (sp[1]) {
	if (RIGHT(sp[1]) == z)
	    SET_RIGHT(sp[1], x);
	else
	    SET_LEFT(sp[1], x);
    }

    SET_RIGHT(x, z);
    sp[0] = x;
}
CU_SINLINE void
cuconP_rbnode_rotate_left(cucon_rbnode_t *sp)
{
    cucon_rbnode_t x = sp[0];
    cucon_rbnode_t z = RIGHT(x);
    SET_RIGHT(x, LEFT(z));

    if (sp[1]) {
	if (RIGHT(sp[1]) == x)
	    SET_RIGHT(sp[1], z);
	else
	    SET_LEFT(sp[1], z);
    }

    SET_LEFT(z, x);
    sp[0] = z;
}

CU_SINLINE void
cuconP_rbnode_rotate_right_ctive(cucon_rbnode_t *sp,
				 cucon_rbnode_copier_t node_new_copy,
				 cucon_rbnode_t new_node)
{
    cucon_rbnode_t z = sp[0];
    cucon_rbnode_t x_src = LEFT(z);
    cucon_rbnode_t x_dst;
    if (x_src == new_node)
	x_dst = x_src;
    else {
	x_dst = cu_call(node_new_copy, x_src);
	SET_LEFT(x_dst, LEFT(x_src));
    }
    SET_LEFT(z, RIGHT(x_src));
    if (sp[1]) {
	if (RIGHT(sp[1]) == z)
	    SET_RIGHT(sp[1], x_dst);
	else
	    SET_LEFT(sp[1], x_dst);
    }
    SET_RIGHT(x_dst, z);
    sp[0] = x_dst;
}
CU_SINLINE void
cuconP_rbnode_rotate_left_ctive(cucon_rbnode_t *sp,
				cucon_rbnode_copier_t node_new_copy,
				cucon_rbnode_t new_node)
{
    cucon_rbnode_t x = sp[0];
    cucon_rbnode_t z_src = RIGHT(x);
    cucon_rbnode_t z_dst;
    if (z_src == new_node)
	z_dst = z_src;
    else {
	z_dst = cu_call(node_new_copy, z_src);
	SET_RIGHT(z_dst, RIGHT(z_src));
    }
    SET_RIGHT(x, LEFT(z_src));
    if (sp[1]) {
	if (RIGHT(sp[1]) == x)
	    SET_RIGHT(sp[1], z_dst);
	else
	    SET_LEFT(sp[1], z_dst);
    }
    SET_LEFT(z_dst, x);
    sp[0] = z_dst;
}

CU_SINLINE void
cuconP_rbtree_rebalance_dtive(cucon_rbnode_t *sp)
{
    /* Rebalance */
    BECOME_RED(sp[0]);
    while (sp[1] && IS_RED(sp[1])) {
	/* here, sp[1] is not root either, because root is not red */
	if (sp[1] == LEFT(sp[2])) {
	    cucon_rbnode_t y = RIGHT(sp[2]);
	    if (y && IS_RED(y)) {
		BECOME_BLACK(sp[1]);
		BECOME_BLACK(y);
		sp += 2;
		BECOME_RED(sp[0]);
	    }
	    else {
		if (sp[0] == RIGHT(sp[1]))
		    cuconP_rbnode_rotate_left(sp + 1);
		BECOME_BLACK(sp[1]);
		BECOME_RED(sp[2]);
		cuconP_rbnode_rotate_right(sp + 2);
	    }
	}
	else {
	    cucon_rbnode_t y = LEFT(sp[2]);
	    if (y && IS_RED(y)) {
		BECOME_BLACK(sp[1]);
		BECOME_BLACK(y);
		sp += 2;
		BECOME_RED(sp[0]);
	    }
	    else {
		if (sp[0] == LEFT(sp[1]))
		    cuconP_rbnode_rotate_right(sp + 1);
		BECOME_BLACK(sp[1]);
		BECOME_RED(sp[2]);
		cuconP_rbnode_rotate_left(sp + 2);
	    }
	}
    }
}

cu_bool_t
cucon_rbtree_insert1m_mem(cucon_rbtree_t tree, cu_clop(compare, int, void *),
			  size_t slot_size, cu_ptr_ptr_t slot)
{
    size_t tree_size = cucon_rbtree_size(tree);
    size_t stack_size = 2*cu_ulong_floor_log2(tree_size + 1) + 3;
    cucon_rbnode_t *sp = cu_salloc(sizeof(cucon_rbnode_t)*stack_size);
    cucon_rbnode_t *sp_root;
    cucon_rbnode_t x;
    int cmp CU_NOINIT(0);
    sp += stack_size - 1;
    sp_root = sp - 1;
    sp[0] = NULL;

    /* Insert */
    x = tree->root;
    if (x != NULL)
	assert(IS_BLACK(x));
    while (x != NULL) {
	*--sp = x;
	cmp = cu_call(compare, cucon_rbnode_mem(x));
	if (cmp < 0)
	    x = x->left;
	else if (cmp > 0)
	    x = x->right;
	else {
	    *(void **)slot = cucon_rbnode_mem(x);
	    assert(tree->root->is_black == tree_size);
	    return cu_false;
	}
    }
    *--sp = x = cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_rbnode)
			   + slot_size);
    if (sp[1]) {
	if (cmp < 0)
	    sp[1]->left = x;
	else
	    sp[1]->right = x;
    }
    else
	tree->root = x;
    *(void **)slot = cucon_rbnode_mem(x);
    cuconP_rbtree_rebalance_dtive(sp);
    tree->root = *sp_root;
    tree->root->is_black = ++tree_size;

#ifdef CHECK_TREE
    cuconP_rbtree_check(tree);
#endif
    return cu_true;
}

cu_bool_t
cucon_rbtree_insert2p_node(cucon_rbtree_t tree,
			   cu_clop(compare, int, void *, void *),
			   void *key,
			   size_t node_size, size_t node_offset,
			   cucon_rbnode_t *node_out)
{
    size_t tree_size = cucon_rbtree_size(tree);
    size_t stack_size = 2*cu_ulong_floor_log2(tree_size + 1) + 3;
    cucon_rbnode_t *sp = cu_salloc(sizeof(cucon_rbnode_t)*stack_size);
    cucon_rbnode_t *sp_root;
    cucon_rbnode_t x;
    int cmp CU_NOINIT(0);
    sp += stack_size - 1;
    sp_root = sp - 1;
    sp[0] = NULL;

    /* Insert */
    x = tree->root;
    if (x != NULL)
	assert(IS_BLACK(x));
    while (x != NULL) {
	*--sp = x;
	cmp = cu_call(compare, key, *(void **)cucon_rbnode_mem(x));
	if (cmp < 0)
	    x = x->left;
	else if (cmp > 0)
	    x = x->right;
	else {
	    *node_out = x;
	    assert(tree->root->is_black == tree_size);
	    return cu_false;
	}
    }
    x = cu_ptr_add(cu_galloc(node_size), node_offset);
    *--sp = x;
    if (sp[1]) {
	if (cmp < 0)
	    sp[1]->left = x;
	else
	    sp[1]->right = x;
    }
    else
	tree->root = x;
    *(void **)cucon_rbnode_mem(x) = key;
    *node_out = x;
    cuconP_rbtree_rebalance_dtive(sp);
    tree->root = *sp_root;
    tree->root->is_black = ++tree_size;

#ifdef CHECK_TREE
    cuconP_rbtree_check(tree);
#endif
    return cu_true;
}

cu_bool_t
cucon_rbtree_insert2p_ptr(cucon_rbtree_t tree,
			  cu_clop(compare, int, void *, void *),
			  cu_ptr_ptr_t ptr_io)
{
    cucon_rbnode_t node;
    if (cucon_rbtree_insert2p_node(
		tree, compare, *(void **)ptr_io,
		sizeof(struct cucon_rbnode) + sizeof(void *), 0,
		&node)) {
	*(void **)ptr_io = cucon_rbnode_ptr(node);
	return cu_true;
    }
    else
	return cu_false;
}

cu_bool_t
cucon_rbtree_cinsert1m_mem(cucon_rbtree_t tree,
			   cu_clop(slot_compare, int, void *),
			   cucon_rbnode_copier_t node_new_copy,
			   size_t slot_size,
			   cu_ptr_ptr_t slot)
{
    size_t tree_size = cucon_rbtree_size(tree);
    size_t stack_size = 2*cu_ulong_floor_log2(tree_size + 1) + 3;
    cucon_rbnode_t *sp = cu_salloc(sizeof(cucon_rbnode_t)*stack_size);
    cucon_rbnode_t *sp_root, *sp_leaf;
    cucon_rbnode_t x, x1;
    int cmp;
    sp += stack_size - 1;
    sp_root = sp - 1;
    sp[0] = NULL;

    /* Insert */
    x = tree->root;
    if (!x) {
	x = cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_rbnode) + slot_size);
	SET_LEFT(x, NULL);
	SET_RIGHT(x, NULL);
	x->is_black = 1;
	*(void **)slot = cucon_rbnode_mem(x);
	tree->root = x;
	return cu_true;
    }
    assert(IS_BLACK(x));
    while (x != NULL) {
	*--sp = x;
	cmp = cu_call(slot_compare, cucon_rbnode_mem(x));
	if (cmp < 0)
	    x = x->left;
	else if (cmp > 0)
	    x = x->right;
	else {
	    *(void **)slot = cucon_rbnode_mem(x);
	    assert(tree->root->is_black == tree_size);
	    return cu_false;
	}
    }
    *--sp = cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_rbnode) + slot_size);
    sp_leaf = sp;
    x1 = cu_call(node_new_copy, sp_root[0]);
    for (sp = sp_root - 1; sp != sp_leaf; --sp) {
	cucon_rbnode_t x0 = cu_call(node_new_copy, sp[0]);
	if (sp[0] == LEFT(sp[1])) {
	    SET_LEFT(x1, x0);
	    SET_RIGHT(x1, RIGHT(sp[1]));
	}
	else {
	    SET_RIGHT(x1, x0);
	    SET_LEFT(x1, LEFT(sp[1]));
	}
	sp[1] = x1;
	x1 = x0;
	assert(sp_root - sp < stack_size - 2);
    }
    x = sp[0];
    if (cmp < 0) {
	SET_LEFT(x1, x);
	SET_RIGHT(x1, RIGHT(sp[1]));
    }
    else {
	SET_RIGHT(x1, x);
	SET_LEFT(x1, LEFT(sp[1]));
    }
    sp[1] = x1;
    *(void **)slot = cucon_rbnode_mem(x);

#ifdef CHECK_TREE
    tree->root = sp_root[0];
    assert(cuconP_rbnode_count(tree->root) == tree_size + 1);
#endif

    /* Rebalance */
    BECOME_RED(sp[0]);
    while (sp[1] && IS_RED(sp[1])) {
	/* here, sp[1] is not root either, because root is not red */
	if (sp[1] == LEFT(sp[2])) {
	    cucon_rbnode_t y = RIGHT(sp[2]);
	    assert(y != x);
	    if (y && IS_RED(y)) {
		cucon_rbnode_t y_cp = cu_call(node_new_copy, y);
		SET_RIGHT(sp[2], y_cp);
		SET_LEFT(y_cp, LEFT(y));
		SET_RIGHT(y_cp, RIGHT(y));
		BECOME_BLACK(sp[1]);
		BECOME_BLACK(y_cp);
		sp += 2;
		BECOME_RED(sp[0]);
	    }
	    else {
		if (sp[0] == RIGHT(sp[1]))
		    cuconP_rbnode_rotate_left_ctive(sp + 1, node_new_copy, x);
		BECOME_BLACK(sp[1]);
		BECOME_RED(sp[2]);
		cuconP_rbnode_rotate_right_ctive(sp + 2, node_new_copy, x);
	    }
	}
	else {
	    cucon_rbnode_t y = LEFT(sp[2]);
	    assert(y != x);
	    if (y && IS_RED(y)) {
		cucon_rbnode_t y_cp = cu_call(node_new_copy, y);
		SET_LEFT(sp[2], y_cp);
		SET_RIGHT(y_cp, RIGHT(y));
		SET_LEFT(y_cp, LEFT(y));
		BECOME_BLACK(sp[1]);
		BECOME_BLACK(y_cp);
		sp += 2;
		BECOME_RED(sp[0]);
	    }
	    else {
		if (sp[0] == LEFT(sp[1]))
		    cuconP_rbnode_rotate_right_ctive(sp + 1, node_new_copy, x);
		BECOME_BLACK(sp[1]);
		BECOME_RED(sp[2]);
		cuconP_rbnode_rotate_left_ctive(sp + 2, node_new_copy, x);
	    }
	}
    }
    tree->root = *sp_root;
    tree->root->is_black = ++tree_size;

#ifdef CHECK_TREE
    cuconP_rbtree_check(tree);
#endif
    return cu_true;
}

cu_clop_def(ptrnode_new_copy, cucon_rbnode_t, cucon_rbnode_t src)
{
    cucon_rbnode_t node = cucon_rbnode_new_copy(src, sizeof(void *));
    *(void **)cucon_rbnode_mem(node) = *(void **)cucon_rbnode_mem(src);
    return node;
}

cu_clos_def(_ptrslot_compare,
	    cu_prot(int, void *slot),
	    (cu_clop(ptr_compare, int, void *, void *); void *ptr;))
{
    cu_clos_self(_ptrslot_compare);
    return cu_call(self->ptr_compare, self->ptr, *(void **)slot);
}
cu_bool_t
cucon_rbtree_cinsert2p_ptr(cucon_rbtree_t tree,
			   cu_clop(ptr_compare, int, void *, void *),
			   cu_ptr_ptr_t ptr_io)
{
    void **slot;
    _ptrslot_compare_t cmp;
    cmp.ptr_compare = ptr_compare;
    cmp.ptr = *(void **)ptr_io;
    if (cucon_rbtree_cinsert1m_mem(
	    tree, _ptrslot_compare_prep(&cmp), ptrnode_new_copy,
	    sizeof(void *), &slot)) {
	*slot = *(void **)ptr_io;
	return cu_true;
    }
    else {
	*(void **)ptr_io = *slot;
	return cu_false;
    }
}

cu_clos_def(_imem_slot_cmp,
	    cu_prot(int, void *slot),
    ( size_t key_size;
      void *key_data; ))
{
    cu_clos_self(_imem_slot_cmp);
    return memcmp(self->key_data, slot, self->key_size);
}
cu_clos_def(_imem_node_copy,
	    cu_prot(cucon_rbnode_t, cucon_rbnode_t node),
    ( size_t slot_size; ))
{
    cu_clos_self(_imem_node_copy);
    cucon_rbnode_t new_node = cucon_rbnode_new_copy(node, self->slot_size);
    memcpy(cucon_rbnode_mem(new_node), cucon_rbnode_mem(node),
	   self->slot_size);
    return new_node;
}

cu_bool_t
cucon_rbtree_insert_imem_ctive(cucon_rbtree_t tree,
			       size_t key_size, void *key,
			       size_t slot_size, cu_ptr_ptr_t slot_o)
{
    _imem_slot_cmp_t cmp;
    _imem_node_copy_t cpy;
    void *new_slot;
    cmp.key_size = key_size;
    cmp.key_data = key;
    cpy.slot_size = slot_size;
    if (cucon_rbtree_cinsert1m_mem(tree,
				   _imem_slot_cmp_prep(&cmp),
				   _imem_node_copy_prep(&cpy),
				   slot_size, &new_slot)) {
	memcpy(new_slot, key, key_size);
	*(void **)slot_o = new_slot;
	return cu_true;
    }
    else {
	*(void **)slot_o = new_slot;
	return cu_false;
    }
}



/* Erase
 * ----- */

static void
_rbtree_erase(cucon_rbtree_t tree,
	      cucon_rbnode_t *sp_root, cucon_rbnode_t *sp_z, cucon_rbnode_t z)
{
    cucon_rbnode_t *sp;
    cucon_rbnode_t x, y;
    int is_unbalanced;
    size_t tree_size = cucon_rbtree_size(tree);

    /* Find an adjacent descendant 'y' with a nil child, extract it
     * and insert in in place of 'z'. */
    sp = sp_z;
    if (!z->left) {
	y = z;
	x = z->right;
	is_unbalanced = IS_BLACK(y);
	/* 'x' may be NULL */
    }
    else if (!z->right) {
	y = z;
	x = z->left;
	is_unbalanced = IS_BLACK(y);
    }
    else {
	/* Successor 'y'. */
	y = z->right;
	*--sp = y;
	if (y->left) {
	    do {
		y = y->left;
		*--sp = y;
	    }
	    while (y->left);
	    x = y->right;
	    sp[1]->left = x;
	}
	else {
	    x = y->right;
	    z->right = x;
	}
	is_unbalanced = IS_BLACK(y);

	/* Insert 'y' in place of 'z'. */
	y->left = z->left;
	y->right = z->right;
	y->is_black = z->is_black;
	if (sp_z[1]) {
	    if (z == sp_z[1]->left)
		sp_z[1]->left = y;
	    else
		sp_z[1]->right = y;
	}
	sp_z[0] = y;
	goto skip_move;
    }

    /* Move up 'x' to the old position of 'y'. */
    if (sp[1]) {
	if (y == sp[1]->left)
	    sp[1]->left = x;
	else
	    sp[1]->right = x;
    }
skip_move:
    sp[0] = x;

    /* Rebalance. */
    if (is_unbalanced) {
	cucon_rbnode_t z;
	while (sp[1] && (!x || IS_BLACK(x))) {
	    if (x == LEFT(sp[1])) {
		y = RIGHT(sp[1]);
		if (IS_RED(y)) {
		    z = sp[1];
		    BECOME_BLACK(y);
		    BECOME_RED(sp[1]);
		    cuconP_rbnode_rotate_left(sp + 1);
		    sp[0] = z;
		    *--sp = x;
		    y = RIGHT(z);
		    assert(sp[2]->left == sp[1] || sp[2]->right == sp[1]);
		    assert(sp[1]->left == sp[0] || sp[1]->right == sp[0]);
		}
		if ((!LEFT(y) || IS_BLACK(LEFT(y))) &&
		    (!RIGHT(y) || IS_BLACK(RIGHT(y)))) {
		    BECOME_RED(y);
		    ++sp;
		}
		else {
		    if (!RIGHT(y) || IS_BLACK(RIGHT(y))) {
			BECOME_BLACK(LEFT(y));
			BECOME_RED(y);
			sp[0] = y;
			cuconP_rbnode_rotate_right(sp);
			y = RIGHT(sp[1]);
		    }
		    y->is_black = sp[1]->is_black;
		    BECOME_BLACK(sp[1]);
		    if (RIGHT(y))
			BECOME_BLACK(RIGHT(y));
		    cuconP_rbnode_rotate_left(sp + 1);
		    sp = sp_root;
		    break;
		}
	    }
	    else {
		y = LEFT(sp[1]);
		if (IS_RED(y)) {
		    z = sp[1];
		    BECOME_BLACK(y);
		    BECOME_RED(z);
		    cuconP_rbnode_rotate_right(sp + 1);
		    sp[0] = z;
		    *--sp = x;
		    y = LEFT(z);
		    assert(sp[2]->left == sp[1] || sp[2]->right == sp[1]);
		    assert(sp[1]->left == sp[0] || sp[1]->right == sp[0]);
		}
		if ((!RIGHT(y) || IS_BLACK(RIGHT(y))) &&
		    (!LEFT(y) || IS_BLACK(LEFT(y)))) {
		    BECOME_RED(y);
		    ++sp;
		}
		else {
		    if (!LEFT(y) || IS_BLACK(LEFT(y))) {
			BECOME_BLACK(RIGHT(y));
			BECOME_RED(y);
			sp[0] = y;
			cuconP_rbnode_rotate_left(sp);
			y = LEFT(sp[1]);
		    }
		    y->is_black = sp[1]->is_black;
		    BECOME_BLACK(sp[1]);
		    if (LEFT(y))
			BECOME_BLACK(LEFT(y));
		    cuconP_rbnode_rotate_right(sp + 1);
		    sp = sp_root;
		    break;
		}
	    }
	    x = sp[0];
	}
    }
    if (sp[0])
	BECOME_BLACK(sp[0]);

    /* Finish */
    tree->root = *sp_root;
    if (tree->root) {
	tree->root->is_black = --tree_size;
#ifdef CHECK_TREE
	cuconP_rbtree_check(tree);
#endif
    }
}

#define RBTREE_ERASE(tree, z, cmp_expr)					\
    size_t tree_size = cucon_rbtree_size(tree);				\
    size_t stack_size = 2*cu_ulong_floor_log2(tree_size + 1) + 3;	\
    cucon_rbnode_t *sp_z = cu_salloc(sizeof(cucon_rbnode_t)*stack_size);\
    cucon_rbnode_t *sp_root;						\
    cucon_rbnode_t z;							\
    int cmp;								\
    sp_z += stack_size - 1;						\
    sp_root = sp_z - 1;							\
    sp_z[0] = NULL;							\
									\
    /* Find the node 'z' to erase. */					\
    z = tree->root;							\
    while (z != NULL) {							\
	*--sp_z = z;							\
	cmp = cmp_expr;							\
	if (cmp < 0)							\
	    z = z->left;						\
	else if (cmp > 0)						\
	    z = z->right;						\
	else								\
	    break;							\
    }									\
    if (z == NULL)							\
	return NULL;							\
    _rbtree_erase(tree, sp_root, sp_z, z);

cucon_rbnode_t
cucon_rbtree_erase2p(cucon_rbtree_t tree,
		     cu_clop(compare, int, void *, void *), void *key)
{
    RBTREE_ERASE(tree, z, cu_call(compare, key, cucon_rbnode_ptr(z)));
    return z;
}

void *
cucon_rbtree_erase1m(cucon_rbtree_t tree, cu_clop(compare, int, void *))
{
    RBTREE_ERASE(tree, z, cu_call(compare, cucon_rbnode_mem(z)));
    return cucon_rbnode_mem(z);
}



/* Find
 * ---- */

cucon_rbnode_t
cucon_rbtree_find2p_node(cucon_rbtree_t tree,
			 cu_clop(compare, int, void *, void *), void *key)
{
    cucon_rbnode_t x = tree->root;
    while (x != NULL) {
	int cmp = cu_call(compare, key, cucon_rbnode_ptr(x));
	if (cmp < 0)
	    x = x->left;
	else if (cmp > 0)
	    x = x->right;
	else
	    return x;
    }
    return NULL;
}

void *
cucon_rbtree_find1m_mem(cucon_rbtree_t tree, cu_clop(compare, int, void *))
{
    cucon_rbnode_t x = tree->root;
    while (x != NULL) {
	int cmp = cu_call(compare, cucon_rbnode_mem(x));
	if (cmp < 0)
	    x = x->left;
	else if (cmp > 0)
	    x = x->right;
	else
	    return cucon_rbnode_mem(x);
    }
    return NULL;
}

void *
cucon_rbtree_find2p_ptr(cucon_rbtree_t tree,
			cu_clop(compare, int, void *, void *),
			void *key)
{
    cucon_rbnode_t x = tree->root;
    while (x != NULL) {
	int cmp = cu_call(compare, key, cucon_rbnode_ptr(x));
	if (cmp < 0)
	    x = x->left;
	else if (cmp > 0)
	    x = x->right;
	else
	    return cucon_rbnode_ptr(x);
    }
    return NULL;
}

void
cucon_rbtree_nearest2p(cucon_rbtree_t tree,
		       cu_clop(cmp2, int, void *, void *), void *key,
		       cucon_rbnode_t *below_out,
		       cucon_rbnode_t *equal_out,
		       cucon_rbnode_t *above_out)
{
    cucon_rbnode_t below = NULL, equal = NULL, above = NULL;
    cucon_rbnode_t x = tree->root;
    while (x != NULL) {
	int cmp = cu_call(cmp2, key, cucon_rbnode_ptr(x));
	if (cmp < 0) {
	    above = x;
	    x = x->left;
	}
	else if (cmp > 0) {
	    below = x;
	    x = x->right;
	}
	else {
	    equal = x;
	    x = equal->left;
	    while (x != NULL) {
		below = x;
		x = x->right;
	    }
	    x = equal->right;
	    while (x != NULL) {
		above = x;
		x = x->left;
	    }
	    break;
	}
    }
    *above_out = above;
    *equal_out = equal;
    *below_out = below;
}

void
cuconP_rbnode_dump_as_graphviz(cucon_rbnode_t x,
			       cu_clop(label, void, void *, FILE *),
			       FILE *out)
{
    cucon_rbnode_t y;
tail_rec:
    y = x->left;
    if (cu_clop_is_null(label))
	fprintf(out, "v%p[color=%s];\n", (void *)x,
		IS_RED(x)? "red" : "black");
    else {
	fprintf(out, "v%p[color=%s,label=\"", (void *)x,
		IS_RED(x)? "red" : "black");
	cu_call(label, cucon_rbnode_mem(x), out);
	fprintf(out, "\"];\n");
    }
    if (y) {
	fprintf(out, "v%p->v%p;\n", (void *)x, (void *)y);
	cuconP_rbnode_dump_as_graphviz(y, label, out);
    }
    else
	fprintf(out, "v%p->v%p; v%p[label=\"\"];\n",
		(void *)x, (char *)x + 1, (char *)x + 1);
    y = x->right;
    if (y) {
	fprintf(out, "v%p->v%p;\n", (void *)x, (void *)y);
	x = y;
	goto tail_rec;
    }
    else
	fprintf(out, "v%p->v%p; v%p[label=\"\"];\n", (void *)x,
		(char *)x + 2, (char *)x + 2);
}

void
cucon_rbtree_dump_as_graphviz(cucon_rbtree_t tree,
			      cu_clop(label, void, void *, FILE *), FILE *out)
{
    cucon_rbnode_t x = tree->root;
    fprintf(out, "digraph RB_tree {\n");
    if (x)
	cuconP_rbnode_dump_as_graphviz(x, label, out);
    fprintf(out, "}\n");
}

static void
cuconP_rbnode_iter_mem(cucon_rbnode_t node, cu_clop(cb, void, void *))
{
    while (node) {
	cuconP_rbnode_iter_mem(node->left, cb);
	cu_call(cb, CU_ALIGNED_PTR_END(node));
	node = node->right;
    }
}

void
cucon_rbtree_iter_mem(cucon_rbtree_t tree, cu_clop(cb, void, void *))
{
    cuconP_rbnode_iter_mem(tree->root, cb);
}

static void
cuconP_rbnode_iter_ptr(cucon_rbnode_t node, cu_clop(cb, void, void *))
{
    while (node) {
	cuconP_rbnode_iter_ptr(node->left, cb);
	cu_call(cb, *(void**)CU_ALIGNED_PTR_END(node));
	node = node->right;
    }
}

void
cucon_rbtree_iter_ptr(cucon_rbtree_t tree, cu_clop(cb, void, void *))
{
    cuconP_rbnode_iter_ptr(tree->root, cb);
}

static void
cuconP_rbnode_rev_iter_mem(cucon_rbnode_t node, cu_clop(cb, void, void *))
{
    while (node) {
	cuconP_rbnode_rev_iter_mem(node->right, cb);
	cu_call(cb, CU_ALIGNED_PTR_END(node));
	node = node->left;
    }
}

void
cucon_rbtree_rev_iter_mem(cucon_rbtree_t tree, cu_clop(cb, void, void *))
{
    cuconP_rbnode_rev_iter_mem(tree->root, cb);
}

static void
cuconP_rbnode_rev_iter_ptr(cucon_rbnode_t node, cu_clop(cb, void, void *))
{
    while (node) {
	cuconP_rbnode_rev_iter_ptr(node->right, cb);
	cu_call(cb, *(void**)CU_ALIGNED_PTR_END(node));
	node = node->left;
    }
}

void
cucon_rbtree_rev_iter_ptr(cucon_rbtree_t tree, cu_clop(cb, void, void *))
{
    cuconP_rbnode_rev_iter_ptr(tree->root, cb);
}

static cu_bool_t
cuconP_rbnode_conj_mem(cucon_rbnode_t node, cu_clop(cb, cu_bool_t, void *))
{
    while (node) {
	if (!cuconP_rbnode_conj_mem(node->left, cb))
	    return cu_false;
	if (!cu_call(cb, CU_ALIGNED_PTR_END(node)))
	    return cu_false;
	node = node->right;
    }
    return cu_true;
}
cu_bool_t
cucon_rbtree_conj_mem(cucon_rbtree_t tree, cu_clop(cb, cu_bool_t, void *))
{
    return cuconP_rbnode_conj_mem(tree->root, cb);
}

static cu_bool_t
cuconP_rbnode_conj_ptr(cucon_rbnode_t node, cu_clop(cb, cu_bool_t, void *))
{
    while (node) {
	if (!cuconP_rbnode_conj_ptr(node->left, cb))
	    return cu_false;
	if (!cu_call(cb, *(void **)CU_ALIGNED_PTR_END(node)))
	    return cu_false;
	node = node->right;
    }
    return cu_true;
}
cu_bool_t
cucon_rbtree_conj_ptr(cucon_rbtree_t tree, cu_clop(cb, cu_bool_t, void *))
{
    return cuconP_rbnode_conj_ptr(tree->root, cb);
}

static cu_bool_t
cuconP_rbnode_rev_conj_mem(cucon_rbnode_t node, cu_clop(cb, cu_bool_t, void *))
{
    while (node) {
	if (!cuconP_rbnode_rev_conj_mem(node->right, cb))
	    return cu_false;
	if (!cu_call(cb, CU_ALIGNED_PTR_END(node)))
	    return cu_false;
	node = node->left;
    }
    return cu_true;
}
cu_bool_t
cucon_rbtree_rev_conj_mem(cucon_rbtree_t tree, cu_clop(cb, cu_bool_t, void *))
{
    return cuconP_rbnode_rev_conj_mem(tree->root, cb);
}

static cu_bool_t
cuconP_rbnode_rev_conj_ptr(cucon_rbnode_t node, cu_clop(cb, cu_bool_t, void *))
{
    while (node) {
	if (!cuconP_rbnode_rev_conj_ptr(node->right, cb))
	    return cu_false;
	if (!cu_call(cb, *(void **)CU_ALIGNED_PTR_END(node)))
	    return cu_false;
	node = node->left;
    }
    return cu_true;
}
cu_bool_t
cucon_rbtree_rev_conj_ptr(cucon_rbtree_t tree, cu_clop(cb, cu_bool_t, void *))
{
    return cuconP_rbnode_rev_conj_ptr(tree->root, cb);
}
