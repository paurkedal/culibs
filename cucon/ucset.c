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
#include <cuoo/halloc.h>
#include <cuoo/intf.h>
#include <inttypes.h>

CU_SINLINE cu_bool_t
_key_covers(uintptr_t key0, uintptr_t key1)
{
    uintptr_t mask0 = ~(key0 ^ (key0 - CU_UINTPTR_C(1)));
    return (key0 & mask0) == (key1 & mask0) && (key1 & ~mask0);
}
CU_SINLINE cu_bool_t
_key_coverseq(uintptr_t key0, uintptr_t key1)
{
    uintptr_t mask0 = ~(key0 ^ (key0 - CU_UINTPTR_C(1)));
    return (key0 & mask0) == (key1 & mask0);
}

CU_SINLINE cu_bool_t
_key_is_leaflike(uintptr_t key)
{
    if (key) {
	uintptr_t mask = key ^ (key - CU_UINTPTR_C(1));
	return mask < CU_WORD_WIDTH*CUPRIV_UCSET_BITSET_WORDCNT;
    }
    else
	return cu_false;
}

CU_SINLINE uintptr_t
_ucnode_key(cucon_ucset_t node)
{ return node->key & ~CU_UINTPTR_C(1); }

CU_SINLINE cucon_ucset_t
_ucnode_new(uintptr_t key, cucon_ucset_t left, cucon_ucset_t right)
{
#if CUCON_UCSET_ENABLE_HCONS
    cuoo_hctem_decl(cucon_ucset, tem);
    cuoo_hctem_init(cucon_ucset, tem);
    cuoo_hctem_get(cucon_ucset, tem)->key = key;
    cuoo_hctem_get(cucon_ucset, tem)->left = left;
    cuoo_hctem_get(cucon_ucset, tem)->right = right;
    return cuoo_hctem_new(cucon_ucset, tem);
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

#define BITSET_WIDTH (CUPRIV_UCSET_BITSET_WORDCNT*CU_WORD_WIDTH)
#define BITSET_MASK  (BITSET_WIDTH - 1)

CU_SINLINE uintptr_t
_leaf_key(uintptr_t key)
{
    return (key & ~BITSET_MASK) | ((BITSET_MASK + CU_UINTPTR_C(1)) >> 1);
}

CU_SINLINE cu_bool_t
_ucleaf_find(cucon_ucset_t node, uintptr_t key)
{
    key &= BITSET_MASK;
    return !!(((cucon_ucset_leaf_t)node)->bitset[key/CU_WORD_WIDTH]
	      & (CU_WORD_C(1) << (key%CU_WORD_WIDTH)));
}

static size_t
_ucleaf_card(cucon_ucset_t node)
{
    int k;
    size_t card = node->key & 1;
    for (k = 0; k < CUPRIV_UCSET_BITSET_WORDCNT; ++k)
	card += cu_word_bit_count(((cucon_ucset_leaf_t)node)->bitset[k]);
    return card;
}

CU_SINLINE cucon_ucset_t
_ucleaf_new(uintptr_t key)
{
    int i;
#if CUCON_UCSET_ENABLE_HCONS
    cucon_ucset_leaf_t node;
    cuoo_hctem_decl(cucon_ucset_leaf, tem);
    cuoo_hctem_init(cucon_ucset_leaf, tem);
    node = cuoo_hctem_get(cucon_ucset_leaf, tem);
#else
    cucon_ucset_leaf_t node = cu_gnew(struct cucon_ucset_leaf_s);
#endif
    node->key = _leaf_key(key);
    for (i = 0; i < CUPRIV_UCSET_BITSET_WORDCNT; ++i)
	node->bitset[i] = 0;
    key &= BITSET_MASK;
    node->bitset[key/CU_WORD_WIDTH]
	|= CU_WORD_C(1) << key%CU_WORD_WIDTH;
#if CUCON_UCSET_ENABLE_HCONS
    return (cucon_ucset_t)cuoo_hctem_new(cucon_ucset_leaf, tem);
#else
    return (cucon_ucset_t)node;
#endif
}

CU_SINLINE cucon_ucset_t
_ucleaf_insert(cucon_ucset_t src_node, uintptr_t add_key)
{
    int i;
#if CUCON_UCSET_ENABLE_HCONS
    cucon_ucset_leaf_t node;
    cuoo_hctem_decl(cucon_ucset_leaf, tem);
    cuoo_hctem_init(cucon_ucset_leaf, tem);
    node = cuoo_hctem_get(cucon_ucset_leaf, tem);
#else
    cucon_ucset_leaf_t node = cu_gnew(struct cucon_ucset_leaf_s);
#endif
    cu_debug_assert(_leaf_key(add_key) == (src_node->key & ~(uintptr_t)1));
    node->key = src_node->key;
    for (i = 0; i < CUPRIV_UCSET_BITSET_WORDCNT; ++i)
	node->bitset[i] = ((cucon_ucset_leaf_t)src_node)->bitset[i];
    add_key &= BITSET_MASK;
    node->bitset[add_key/CU_WORD_WIDTH]
	|= CU_WORD_C(1) << add_key%CU_WORD_WIDTH;
#if CUCON_UCSET_ENABLE_HCONS
    return (cucon_ucset_t)cuoo_hctem_new(cucon_ucset_leaf, tem);
#else
    return (cucon_ucset_t)node;
#endif
}

cucon_ucset_t
cucon_ucset_insert(cucon_ucset_t node, uintptr_t key)
{
    uintptr_t node_key;
    if (node == NULL) {
	if (_key_is_leaflike(key))
	    return _ucleaf_new(key);
	else
	    return _ucnode_new(key | 1, NULL, NULL);
    }

    node_key = _ucnode_key(node);
    if (key == node_key) {
	if (_key_is_leaflike(key))
	    return _ucleaf_insert(node, key);
	else if (node->key & 1)
	    return node;
	else
	    return _ucnode_new(node->key | 1, left(node), right(node));
    }
    else if (_key_covers(node_key, key)) {
	if (_key_is_leaflike(node_key)) {
	    if (_ucleaf_find(node, key))
		return node;
	    else
		return _ucleaf_insert(node, key);
	}
	else if (key < node_key) {
	    cucon_ucset_t new_left;
	    new_left = cucon_ucset_insert(left(node), key);
	    if (new_left != left(node))
		return _ucnode_new(node->key, new_left, right(node));
	    else
		return node;
	}
	else {
	    cucon_ucset_t new_right;
	    new_right = cucon_ucset_insert(right(node), key);
	    if (new_right != right(node))
		return _ucnode_new(node->key, left(node), new_right);
	    else
		return node;
	}
    }
    else if (_key_covers(key, node_key)) {
	if (node_key < key)
	    return _ucnode_new(key | 1, node, NULL);
	else
	    return _ucnode_new(key | 1, NULL, node);
    }
    else {
	cucon_ucset_t key_node;
	uintptr_t j;
	if (_key_is_leaflike(key))
	    key_node = _ucleaf_new(key);
	else
	    key_node = _ucnode_new(key | 1, NULL, NULL);
	j = cu_ulong_dcover(key ^ node_key);
	cu_debug_assert(j >= 2);
	j = (key & ~j) | ((j + 1) >> 1);
	if (key < node_key)
	    return _ucnode_new(j, key_node, node);
	else
	    return _ucnode_new(j, node, key_node);
    }
}

cu_bool_t
cucon_ucset_find(cucon_ucset_t node, uintptr_t key)
{
    uintptr_t node_key;
    while (node) {
	node_key = _ucnode_key(node);
	if (!_key_coverseq(node_key, key))
	    return cu_false;
	else if (_key_is_leaflike(node_key))
	    return _ucleaf_find(node, key);
	else if (key == node_key)
	    return !!(node->key & 1);
	else if (key < node_key)
	    node = left(node);
	else
	    node = right(node);
    }
    return cu_false;
}

cu_bool_t
cucon_ucset_conj(cucon_ucset_t node, cu_clop(cb, cu_bool_t, uintptr_t key))
{
    while (node) {
	uintptr_t key = _ucnode_key(node);
	if (_key_is_leaflike(key)) {
	    uintptr_t k;
	    for (k = (key & ~BITSET_MASK); k <= (key | BITSET_MASK); ++k)
		if (_ucleaf_find(node, k) && !cu_call(cb, k))
		    return cu_false;
	    return cu_true;
	}
	else {
	    if (!cucon_ucset_conj(node->left, cb))
		return cu_false;
	    if ((node->key & 1) && !cu_call(cb, key))
		return cu_false;
	    node = node->right;
	}
    }
    return cu_true;
}

void
cucon_ucset_iter(cucon_ucset_t node, cu_clop(f, void, uintptr_t key))
{
    while (node) {
	uintptr_t key = _ucnode_key(node);
	if (_key_is_leaflike(key)) {
	    uintptr_t k;
	    for (k = (key & ~BITSET_MASK); k <= (key | BITSET_MASK); ++k)
		if (_ucleaf_find(node, k))
		    cu_call(f, k);
	    return;
	}
	else {
	    if ((node->key & 1))
		cu_call(f, key);
	    cucon_ucset_iter(node->left, f);
	    node = node->right;
	}
    }
}

size_t
cucon_ucset_card(cucon_ucset_t node)
{
    size_t card = 0;
    while (node) {
	uintptr_t key = _ucnode_key(node);
	if (_key_is_leaflike(key))
	    return card + _ucleaf_card(node);
	if (node->key & 1)
	    ++card;
	card += cucon_ucset_card(node->left);
	node = node->right;
    }
    return card;
}

#if CUCON_UCSET_WORD_CNT%2 != 0
#  error CUCON_UCSET_WORD_CNT must be a multiple of 2
#endif

uintptr_t
cucon_ucset_min_ukey(cucon_ucset_t set)
{
    uintptr_t min = UINTPTR_MAX;
    while (set) {
	uintptr_t key = _ucnode_key(set);
	if (_key_is_leaflike(key)) {
	    int i;
	    for (i = 0; i < CUPRIV_UCSET_BITSET_WORDCNT; ++i) {
		uintptr_t x = ((cucon_ucset_leaf_t)set)->bitset[i];
		if (x)
		    return (key & ~BITSET_MASK) + cu_ulong_log2_lowbit(x)
			 + i*CU_WORD_WIDTH;
	    }
	    return key;
	}
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
	uintptr_t key = _ucnode_key(set);
	if (_key_is_leaflike(key)) {
	    int i;
	    for (i = CUPRIV_UCSET_BITSET_WORDCNT - 1; i >= 0; --i) {
		uintptr_t x = ((cucon_ucset_leaf_t)set)->bitset[i];
		if (x)
		    return (key & ~BITSET_MASK) + cu_ulong_floor_log2(x)
			 + i*CU_WORD_WIDTH;
	    }
	    return key;
	}
	max = key;
	set = set->right;
    }
    return max;
}

cu_clos_def(_ucset_image_helper, cu_prot(void, uintptr_t key),
    ( cucon_ucset_t image;
      cu_clop(f, uintptr_t, uintptr_t); ))
{
    cu_clos_self(_ucset_image_helper);
    self->image = cucon_ucset_insert(self->image, cu_call(self->f, key));
}

cucon_ucset_t
cucon_ucset_image(cucon_ucset_t set, cu_clop(f, uintptr_t, uintptr_t))
{
    _ucset_image_helper_t cb;
    cb.image = NULL;
    cucon_ucset_iter(set, _ucset_image_helper_prep(&cb));
    return cb.image;
}

static void
_ucset_dump(cucon_ucset_t tree, int ind, FILE *out)
{
    int i = ind++;
    if (tree) {
	uintptr_t key = _ucnode_key(tree);
	if (_key_is_leaflike(key)) {
	    while (i--)
		fputs("  ", out);
	    fprintf(out, "%c 0x%"PRIxPTR":", tree->key & 1? '+' : '-', key);
#if 0
	    for (i = 0; i < BITSET_WIDTH; ++i) {
		if (i % 16 == 0)
		    fputc(' ', out);
		fputc(_ucleaf_find(tree, i)? '1' : '0', out);
	    }
#else
	    for (i = 0; i < CUPRIV_UCSET_BITSET_WORDCNT; ++i)
		fprintf(out, " 0x%"CU_PRIxWORD,
			((cucon_ucset_leaf_t)tree)->bitset[i]);
#endif
	    fputc('\n', out);
	    return;
	}
	_ucset_dump(left(tree), ind, out);
	while (i--)
	    fputs("  ", out);
	fprintf(out, "%c 0x%"PRIxPTR"\n", tree->key & 1? '+' : '-',
		_ucnode_key(tree));
	_ucset_dump(right(tree), ind, out);
    }
}

void
cucon_ucset_dump(cucon_ucset_t tree, FILE *out)
{
    fprintf(out, "cucon_ucset_t @ %p\n", tree);
    _ucset_dump(tree, 2, out);
}

cu_word_t cuconP_ucset_foprint = CUOO_IMPL_NONE;

static cu_word_t
_ucset_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN: return (cu_word_t)cucon_ucset_dump;
	case CUOO_INTF_FOPRINT_FN: return cuconP_ucset_foprint;
	default: return CUOO_IMPL_NONE;
    }
}

#if CUCON_UCSET_ENABLE_HCONS
cuoo_type_t cuconP_ucset_type;
cuoo_type_t cuconP_ucset_leaf_type;
#endif

void
cuconP_ucset_init()
{
    cu_debug_assert_once();
#if CUCON_UCSET_ENABLE_HCONS
    cuconP_ucset_type = cuoo_type_new_opaque_hcs(
	_ucset_impl, sizeof(struct cucon_ucset_s) - CUOO_HCOBJ_SHIFT);
    cuconP_ucset_leaf_type = cuoo_type_new_opaque_hcs(
	_ucset_impl, sizeof(struct cucon_ucset_leaf_s) - CUOO_HCOBJ_SHIFT);
#endif
}
