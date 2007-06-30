/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/ucset.h>
#include <cu/memory.h>
#include <cu/int.h>
#include <inttypes.h>

CU_SINLINE uintptr_t ucnode_key(cucon_ucset_t node)
{ return node->key & ~CU_UINTPTR_C(1); }

CU_SINLINE cu_bool_t
key_covers(uintptr_t key0, uintptr_t key1)
{
    uintptr_t mask0 = ~(key0 ^ (key0 - CU_UINTPTR_C(1)));
    return (key0 & mask0) == (key1 & mask0) && (key1 & ~mask0);
}
CU_SINLINE cu_bool_t
key_coverseq(uintptr_t key0, uintptr_t key1)
{
    uintptr_t mask0 = ~(key0 ^ (key0 - CU_UINTPTR_C(1)));
    return (key0 & mask0) == (key1 & mask0);
}

#if CUCON_UCSET_ENABLE_BITSET
CU_SINLINE cu_bool_t
key_at_bitlevel(uintptr_t key)
{
    if (key) {
	uintptr_t mask = key ^ (key - CU_UINTPTR_C(1));
	return mask < sizeof(cu_word_t)*8*CUPRIV_UCSET_BITSET_WORDCNT;
    }
    else
	return cu_false;
}
#endif

CU_SINLINE cucon_ucset_t
ucnode_new(uintptr_t key, cucon_ucset_t left, cucon_ucset_t right)
{
#if CUCON_UCSET_ENABLE_HCONS
    struct cucon_ucset_s node;
    node.key = key;
    node.left = left;
    node.right = right;
    return cudyn_hnew(cucon_ucset, &node);
#else
    cucon_ucset_t node = cu_gnew(struct cucon_ucset_s);
    node->key = key;
    node->left = left;
    node->right = right;
    return node;
#endif
}

#define left(t) ((t)->left)
#define right(t) ((t)->right)

#if CUCON_UCSET_ENABLE_BITSET
#define BITSET_WIDTH (CUPRIV_UCSET_BITSET_WORDCNT*sizeof(cu_word_t)*8)
#define BITSET_MASK  (BITSET_WIDTH - 1)

CU_SINLINE uintptr_t
leaf_key(uintptr_t key)
{
    return (key & ~BITSET_MASK) | ((BITSET_MASK + CU_UINTPTR_C(1)) >> 1);
}

CU_SINLINE cu_bool_t
ucleaf_find(cucon_ucset_t node, uintptr_t key)
{
    key &= BITSET_MASK;
    return !!(((cucon_ucset_leaf_t)node)->bitset[key/(sizeof(cu_word_t)*8)]
	      & (CU_WORD_C(1) << (key%(sizeof(cu_word_t)*8))));
}

CU_SINLINE cucon_ucset_t
ucleaf_new(uintptr_t key)
{
    int i;
#if CUCON_UCSET_ENABLE_HCONS
    struct cucon_ucset_leaf_s node;
    node.key = leaf_key(key);
    for (i = 0; i < CUPRIV_UCSET_BITSET_WORDCNT; ++i)
	node.bitset[i] = 0;
    key &= BITSET_MASK;
    node.bitset[key/(sizeof(cu_word_t)*8)]
	|= CU_WORD_C(1) << key%(sizeof(cu_word_t)*8);
    return (cucon_ucset_t)cudyn_hnew(cucon_ucset_leaf, &node);
#else
    cucon_ucset_leaf_t node = cu_gnew(struct cucon_ucset_leaf_s);
    node->key = leaf_key(key);
    for (i = 0; i < CUPRIV_UCSET_BITSET_WORDCNT; ++i)
	node->bitset[i] = 0;
    key &= BITSET_MASK;
    node->bitset[key/(sizeof(cu_word_t)*8)]
	|= CU_WORD_C(1) << key%(sizeof(cu_word_t)*8);
    return (cucon_ucset_t)node;
#endif
}

CU_SINLINE cucon_ucset_t
ucleaf_new_copy(cucon_ucset_t src_node, uintptr_t key)
{
    int i;
#if CUCON_UCSET_ENABLE_HCONS
    struct cucon_ucset_leaf_s node;
    node.key = leaf_key(key);
    for (i = 0; i < CUPRIV_UCSET_BITSET_WORDCNT; ++i)
	node.bitset[i] = ((cucon_ucset_leaf_t)src_node)->bitset[i];
    key &= BITSET_MASK;
    node.bitset[key/(sizeof(cu_word_t)*8)]
	|= CU_WORD_C(1) << key%(sizeof(cu_word_t)*8);
    return (cucon_ucset_t)cudyn_hnew(cucon_ucset_leaf, &node);
#else
    cucon_ucset_leaf_t node = cu_gnew(struct cucon_ucset_leaf_s);
    node->key = leaf_key(key);
    for (i = 0; i < CUPRIV_UCSET_BITSET_WORDCNT; ++i)
	node->bitset[i] = ((cucon_ucset_leaf_t)src_node)->bitset[i];
    key &= BITSET_MASK;
    node->bitset[key/(sizeof(cu_word_t)*8)] |= CU_WORD_C(1) << key%(sizeof(cu_word_t)*8);
    return (cucon_ucset_t)node;
#endif
}
#endif /* CUCON_UCSET_ENABLE_BITSET */

cucon_ucset_t
cucon_ucset_insert(cucon_ucset_t node, uintptr_t key)
{
    uintptr_t node_key;
    if (node == NULL) {
#if CUCON_UCSET_ENABLE_BITSET
	if (key_at_bitlevel(key))
	    return ucleaf_new(key);
#endif
	return ucnode_new(key | 1, NULL, NULL);
    }

    node_key = ucnode_key(node);
#if CUCON_UCSET_ENABLE_BITSET
    if (key_at_bitlevel(node_key)) {
	if (key_covers(node_key, key)) {
	    if (ucleaf_find(node, key))
		return node;
	    else
		return ucleaf_new_copy(node, key);
	}
    }
    else {
#endif
	if (node_key == key) {
	    if (node->key & 1)
		return node;
	    else
		return ucnode_new(node->key | 1, left(node), right(node));
	}
	if (key_covers(node_key, key)) {
	    if (key < node_key) {
		cucon_ucset_t new_left;
		new_left = cucon_ucset_insert(left(node), key);
		if (new_left != left(node))
		    return ucnode_new(node->key, new_left, right(node));
		else
		    return node;
	    }
	    else {
		cucon_ucset_t new_right;
		new_right = cucon_ucset_insert(right(node), key);
		if (new_right != right(node))
		    return ucnode_new(node->key, left(node), new_right);
		else
		    return node;
	    }
	}
#if CUCON_UCSET_ENABLE_BITSET
    }
#endif

    if (key_covers(key, node_key)) {
	if (node_key < key)
	    return ucnode_new(key | 1, node, NULL);
	else
	    return ucnode_new(key | 1, NULL, node);
    }

    else {
	cucon_ucset_t key_node;
	uintptr_t j;
#if CUCON_UCSET_ENABLE_BITSET
	if (key_at_bitlevel(key))
	    key_node = ucleaf_new(key);
	else
#endif
	    key_node = ucnode_new(key | 1, NULL, NULL);
	j = cu_ulong_dcover(key ^ node_key);
	cu_debug_assert(j >= 2);
	j = (key & ~j) | ((j + 1) >> 1);
	if (key < node_key)
	    return ucnode_new(j, key_node, node);
	else
	    return ucnode_new(j, node, key_node);
    }
}

cu_bool_t
cucon_ucset_find(cucon_ucset_t node, uintptr_t key)
{
    uintptr_t node_key;
tailcall:
    if (!node)
	return cu_false;
    node_key = ucnode_key(node);
#if CUCON_UCSET_ENABLE_BITSET
    if (key_at_bitlevel(node_key))
	return key_coverseq(node_key, key) && ucleaf_find(node, key);
#endif
    if (key == node_key)
	return !!(node->key & 1);
    if (key_coverseq(node_key, key)) {
	if (key < node_key)
	    node = left(node);
	else
	    node = right(node);
	goto tailcall;
    }
    else
	return cu_false;
}

cu_bool_t
cucon_ucset_conj(cucon_ucset_t node, cu_clop(cb, cu_bool_t, uintptr_t key))
{
    while (node) {
	uintptr_t key = ucnode_key(node);
#if CUCON_UCSET_ENABLE_BITSET
	if (key_at_bitlevel(key)) {
	    uintptr_t k;
	    for (k = (key & ~BITSET_MASK); k < key; ++k)
		if (ucleaf_find(node, k) && !cu_call(cb, k))
		    return cu_false;
	    if ((node->key & 1) && !cu_call(cb, key))
		return cu_false;
	    for (k = key + 1; k <= (key | BITSET_MASK); ++k)
		if (ucleaf_find(node, k) && !cu_call(cb, k))
		    return cu_false;
	    return cu_true;
	}
#endif
	if (!cucon_ucset_conj(node->left, cb))
	    return cu_false;
	if ((node->key & 1) && !cu_call(cb, key))
	    return cu_false;
	node = node->right;
    }
    return cu_true;
}

#if CUCON_UCSET_WORD_CNT%2 != 0
#  error CUCON_UCSET_WORD_CNT must be a multiple of 2
#endif

uintptr_t
cucon_ucset_min_ukey(cucon_ucset_t set)
{
    uintptr_t min = UINTPTR_MAX;
    while (set) {
	uintptr_t key = ucnode_key(set);
#if CUCON_UCSET_ENABLE_BITSET
	if (key_at_bitlevel(key)) {
	    int i, j;
	    j = set->key & 1
		? CUPRIV_UCSET_BITSET_WORDCNT/2
		: CUPRIV_UCSET_BITSET_WORDCNT;
	    for (i = 0; i < j; ++i) {
		uintptr_t x = ((cucon_ucset_leaf_t)set)->bitset[i];
		if (x)
		    return (key & ~BITSET_MASK) + cu_ulong_log2_lowbit(x)
			 + i*sizeof(cu_word_t)*8;
	    }
	    return key;
	}
#endif
	min = key;
	set = set->left;
    }
    return min;
}

uintptr_t
cucon_ucset_max_ukey(cucon_ucset_t set)
{
    uintptr_t max = 0;
    while (set) {
	uintptr_t key = ucnode_key(set);
#if CUCON_UCSET_ENABLE_BITSET
	if (key_at_bitlevel(key)) {
	    int i, j;
	    j = set->key & 1
		? CUPRIV_UCSET_BITSET_WORDCNT/2
		: 0;
	    for (i = CUPRIV_UCSET_BITSET_WORDCNT - 1; i >= j; --i) {
		uintptr_t x = ((cucon_ucset_leaf_t)set)->bitset[i];
		if (x)
		    return (key & ~BITSET_MASK) + cu_ulong_floor_log2(x)
			 + i*sizeof(cu_word_t)*8;
	    }
	    return key;
	}
#endif
	max = key;
	set = set->right;
    }
    return max;
}

static void
ucset_dump(cucon_ucset_t tree, int ind, FILE *out)
{
    int i = ind++;
    if (tree) {
#if CUCON_UCSET_ENABLE_BITSET
	if (key_at_bitlevel(ucnode_key(tree))) {
	    while (i--)
		fputs("  ", out);
	    fprintf(out, "+ 0x%"PRIdPTR":", ucnode_key(tree));
#if 1
	    for (i = 0; i < BITSET_WIDTH; ++i) {
		if (i % 16 == 0)
		    fputc(' ', out);
		fputc(ucleaf_find(tree, i)? '1' : '0', out);
	    }
#else
	    for (i = 0; i < CUPRIV_UCSET_BITSET_WORDCNT; ++i)
		fprintf(out, " %lx", ((cucon_ucset_leaf_t)tree)->bitset[i]);
#endif
	    fputc('\n', out);
	    return;
	}
#endif
	ucset_dump(left(tree), ind, out);
	while (i--)
	    fputs("  ", out);
	fprintf(out, "%c 0x%lx\n", tree->key & 1? '+' : '-',
		(unsigned long)ucnode_key(tree));
	ucset_dump(right(tree), ind, out);
    }
}

void
cucon_ucset_dump(cucon_ucset_t tree, FILE *out)
{
    fprintf(out, "cucon_ucset_t @ %p\n", tree);
    ucset_dump(tree, 2, out);
}

#if CUCON_UCSET_ENABLE_HCONS
cudyn_stdtype_t cuconP_ucset_type;
cudyn_stdtype_t cuconP_ucset_leaf_type;
#endif

void
cuconP_ucset_init()
{
#if CUCON_UCSET_ENABLE_HCONS
    cuconP_ucset_type =
	cudyn_stdtype_new_hcs(sizeof(struct cucon_ucset_s) - CU_HCOBJ_SHIFT);
#  if CUCON_UCSET_ENABLE_BITSET
    cuconP_ucset_leaf_type =
	cudyn_stdtype_new_hcs(sizeof(struct cucon_ucset_leaf_s)
			      - CU_HCOBJ_SHIFT);
#  endif
#endif
}

