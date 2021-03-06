/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2010  Petter Urkedal <paurkedal@eideticdew.org>
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
#include <cu/word.h>
#include <cu/ptr.h>
#include <cuoo/hctem.h>
#include <cuoo/intf.h>
#include <cuoo/hcobj.h>
#include <inttypes.h>

#define BITSET_LOG2_SIZEW 1
#define BITSET_SIZEW (1 << BITSET_LOG2_SIZEW)
#define BITSET_LOG2_WIDTH (BITSET_LOG2_SIZEW + CU_WORD_LOG2_P2WIDTH)
#define BITSET_WIDTH (BITSET_SIZEW*CU_WORD_P2WIDTH)
#define BITSET_MASK  (BITSET_WIDTH - 1)
#define LEAFLIKE_ZERO 1

struct cucon_ucset
{
#if CUCON_UCSET_ENABLE_HCONS
    CUOO_HCOBJ
#else
    CUOO_OBJ
#endif
    uintptr_t key;
    cucon_ucset_t left;
    cucon_ucset_t right;
};

#if CUCON_UCSET_ENABLE_HCONS
static cuoo_type_t _ucset_type;
static cuoo_type_t _ucset_leaf_type;

CU_SINLINE cuoo_type_t cucon_ucset_type()
{ return _ucset_type; }

CU_SINLINE cuoo_type_t cucon_ucset_leaf_type()
{ return _ucset_leaf_type; }
#endif

typedef struct cucon_ucset_leaf *cucon_ucset_leaf_t;
struct cucon_ucset_leaf
{
#if CUCON_UCSET_ENABLE_HCONS
    CUOO_HCOBJ
#else
    CUOO_OBJ
#endif
    uintptr_t key;
    cu_word_t bitset[BITSET_SIZEW];
};


CU_SINLINE cu_bool_t
_key_covers(uintptr_t key0, uintptr_t key1)
{
    uintptr_t mask0 = ~(key0 ^ (key0 - CU_UINTPTR_C(1)));
#ifdef LEAFLIKE_ZERO
    return key0 && (key0 & mask0) == (key1 & mask0) && ((key1 & ~mask0) || !key1);
#else
    return (key0 & mask0) == (key1 & mask0) && (key1 & ~mask0);
#endif
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
    uintptr_t mask;
#ifdef LEAFLIKE_ZERO
    if (!key)
	return cu_true;
#endif
    mask = key ^ (key - CU_UINTPTR_C(1));
    return mask < CU_WORD_P2WIDTH*BITSET_SIZEW;
}

CU_SINLINE uintptr_t
_key_join(uintptr_t k0, uintptr_t k1)
{
    uintptr_t k = cu_uintptr_dcover(k0 ^ k1);
    return (k0 & ~k) | ((k >> 1) + (uintptr_t)1);
}

CU_SINLINE uintptr_t _key_min(uintptr_t k) { return k & (k - (uintptr_t)1); }
CU_SINLINE uintptr_t _key_max(uintptr_t k) { return k | (k - (uintptr_t)1); }

CU_SINLINE uintptr_t
_ucnode_key(cucon_ucset_t node)
{ return node->key & ~CU_UINTPTR_C(1); }

CU_SINLINE cucon_ucset_t
_ucnode_new(uintptr_t key, cucon_ucset_t left, cucon_ucset_t right)
{
    cu_debug_assert(!_key_is_leaflike(key & ~CU_UINTPTR_C(1)));
#if CUCON_UCSET_ENABLE_HCONS
    cuoo_hctem_decl(cucon_ucset, tem);
    cuoo_hctem_init(cucon_ucset, tem);
    cuoo_hctem_get(cucon_ucset, tem)->key = key;
    cuoo_hctem_get(cucon_ucset, tem)->left = left;
    cuoo_hctem_get(cucon_ucset, tem)->right = right;
    return cuoo_hctem_new(cucon_ucset, tem);
#else
    cucon_ucset_t node = cu_gnew(struct cucon_ucset);
    node->key = key;
    node->left = left;
    node->right = right;
    return node;
#endif
}

#define left(t) ((t)->left)
#define right(t) ((t)->right)

CU_SINLINE uintptr_t
_leaf_key(uintptr_t key)
{
    return (key & ~BITSET_MASK) | ((BITSET_MASK + CU_UINTPTR_C(1)) >> 1);
}

CU_SINLINE cu_bool_t
_ucleaf_find(cucon_ucset_t node, uintptr_t key)
{
    key &= BITSET_MASK;
    return !!(((cucon_ucset_leaf_t)node)->bitset[key/CU_WORD_P2WIDTH]
	      & (CU_WORD_C(1) << (key%CU_WORD_P2WIDTH)));
}

#if !CUCON_UCSET_ENABLE_HCONS
static cu_bool_t
_ucleaf_eq(cucon_ucset_t lhs, cucon_ucset_t rhs)
{
    int k;
    cu_debug_assert(lhs->key == rhs->key);
    for (k = 0; k < BITSET_SIZEW; ++k)
	if (((cucon_ucset_leaf_t)lhs)->bitset[k] !=
	    ((cucon_ucset_leaf_t)rhs)->bitset[k])
	    return cu_false;
    return cu_true;
}
#endif

static cu_bool_t
_ucleaf_subeq(cucon_ucset_t lhs, cucon_ucset_t rhs)
{
    int k;
    cu_debug_assert(lhs->key == rhs->key);
    for (k = 0; k < BITSET_SIZEW; ++k)
	if (((cucon_ucset_leaf_t)lhs)->bitset[k] &
	    ~((cucon_ucset_leaf_t)rhs)->bitset[k])
	    return cu_false;
    return cu_true;
}

static size_t
_ucleaf_card(cucon_ucset_t node)
{
    int k;
    size_t card = node->key & 1;
    for (k = 0; k < BITSET_SIZEW; ++k)
	card += cu_word_bit_count(((cucon_ucset_leaf_t)node)->bitset[k]);
    return card;
}

static cu_bool_t
_ucleaf_is_singleton(cucon_ucset_t node)
{
    int k;
    for (k = 0; k < BITSET_SIZEW; ++k) {
	size_t cnt = cu_word_bit_count(((cucon_ucset_leaf_t)node)->bitset[k]);
	if (cnt > 1)
	    return cu_false;
	else if (cnt) {
	    while (++k < BITSET_SIZEW)
		if (((cucon_ucset_leaf_t)node)->bitset[k])
		    return cu_false;
	    return cu_true;
	}
    }
    return cu_false;
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
    cucon_ucset_leaf_t node = cu_gnew(struct cucon_ucset_leaf);
#endif
    cu_debug_assert(_key_is_leaflike(key));
    node->key = _leaf_key(key);
    for (i = 0; i < BITSET_SIZEW; ++i)
	node->bitset[i] = 0;
    key &= BITSET_MASK;
    node->bitset[key/CU_WORD_P2WIDTH]
	|= CU_WORD_C(1) << key%CU_WORD_P2WIDTH;
#if CUCON_UCSET_ENABLE_HCONS
    return (cucon_ucset_t)cuoo_hctem_new(cucon_ucset_leaf, tem);
#else
    return (cucon_ucset_t)node;
#endif
}

CU_SINLINE cucon_ucset_t
_ucleaf_insert(cucon_ucset_t src_node, uintptr_t add_key)
{
#if CUCON_UCSET_ENABLE_HCONS
    cucon_ucset_leaf_t node;
    cuoo_hctem_decl(cucon_ucset_leaf, tem);
    cuoo_hctem_init(cucon_ucset_leaf, tem);
    node = cuoo_hctem_get(cucon_ucset_leaf, tem);
    memcpy(cu_ptr_add(node, CUOO_HCOBJ_SHIFT),
	   cu_ptr_add(src_node, CUOO_HCOBJ_SHIFT),
	   sizeof(struct cucon_ucset_leaf) - CUOO_HCOBJ_SHIFT);
#else
    cucon_ucset_leaf_t node = cu_gnew(struct cucon_ucset_leaf);
    memcpy(node, src_node, sizeof(struct cucon_ucset_leaf));
#endif
    cu_debug_assert(_leaf_key(add_key) == (src_node->key & ~(uintptr_t)1));
    add_key &= BITSET_MASK;
    node->bitset[add_key/CU_WORD_P2WIDTH]
	|= CU_WORD_C(1) << add_key%CU_WORD_P2WIDTH;
#if CUCON_UCSET_ENABLE_HCONS
    return (cucon_ucset_t)cuoo_hctem_new(cucon_ucset_leaf, tem);
#else
    return (cucon_ucset_t)node;
#endif
}

CU_SINLINE cucon_ucset_t
_ucleaf_erase(cucon_ucset_t src_node, uintptr_t del_key)
{
    int i;
    cu_word_t bit, bits;
#if CUCON_UCSET_ENABLE_HCONS
    cucon_ucset_leaf_t node;
    cuoo_hctem_decl(cucon_ucset_leaf, tem);
#else
    cucon_ucset_leaf_t node;
#endif

    cu_debug_assert(_key_is_leaflike(del_key));
    cu_debug_assert(_key_is_leaflike(_ucnode_key(src_node)));
    cu_debug_assert(_leaf_key(del_key) == (src_node->key & ~(uintptr_t)1));
    del_key &= BITSET_MASK;
    bits = ((cucon_ucset_leaf_t)src_node)->bitset[del_key/CU_WORD_P2WIDTH];
    bit = CU_WORD_C(1) << del_key%CU_WORD_P2WIDTH;
    if (!(bits & bit))
	return src_node;
    bits &= ~bit;

#if CUCON_UCSET_ENABLE_HCONS
    cuoo_hctem_init(cucon_ucset_leaf, tem);
    node = cuoo_hctem_get(cucon_ucset_leaf, tem);
    memcpy(cu_ptr_add(node, CUOO_HCOBJ_SHIFT),
	   cu_ptr_add(src_node, CUOO_HCOBJ_SHIFT),
	   sizeof(struct cucon_ucset_leaf) - CUOO_HCOBJ_SHIFT);
#else
    node = cu_gnew(struct cucon_ucset_leaf);
    memcpy(node, src_node, sizeof(struct cucon_ucset_leaf));
#endif
    node->bitset[del_key/CU_WORD_P2WIDTH] = bits;
    for (i = 0; i < BITSET_SIZEW; ++i)
	if (node->bitset[i]) {
#if CUCON_UCSET_ENABLE_HCONS
	    return (cucon_ucset_t)cuoo_hctem_new(cucon_ucset_leaf, tem);
#else
	    return (cucon_ucset_t)node;
#endif
	}
    return NULL;
}

CU_SINLINE cucon_ucset_t
_ucleaf_union(cucon_ucset_t lhs, cucon_ucset_t rhs)
{
    int i;
#if CUCON_UCSET_ENABLE_HCONS
    cucon_ucset_leaf_t node;
    cuoo_hctem_decl(cucon_ucset_leaf, tem);
    cuoo_hctem_init(cucon_ucset_leaf, tem);
    node = cuoo_hctem_get(cucon_ucset_leaf, tem);
#else
    cucon_ucset_leaf_t node = cu_gnew(struct cucon_ucset_leaf);
#endif
    node->key = lhs->key;
    for (i = 0; i < BITSET_SIZEW; ++i)
	node->bitset[i] = ((cucon_ucset_leaf_t)lhs)->bitset[i]
			| ((cucon_ucset_leaf_t)rhs)->bitset[i];
#if CUCON_UCSET_ENABLE_HCONS
    return (cucon_ucset_t)cuoo_hctem_new(cucon_ucset_leaf, tem);
#else
    return (cucon_ucset_t)node;
#endif
}

CU_SINLINE cucon_ucset_t
_ucleaf_isecn(cucon_ucset_t lhs, cucon_ucset_t rhs)
{
    int i;
    cu_bool_t have_nonzero = cu_false;
#if CUCON_UCSET_ENABLE_HCONS
    cucon_ucset_leaf_t node;
    cuoo_hctem_decl(cucon_ucset_leaf, tem);
    cuoo_hctem_init(cucon_ucset_leaf, tem);
    node = cuoo_hctem_get(cucon_ucset_leaf, tem);
#else
    cucon_ucset_leaf_t node = cu_gnew(struct cucon_ucset_leaf);
#endif
    node->key = lhs->key;
    for (i = 0; i < BITSET_SIZEW; ++i) {
	uintptr_t bits;
	bits = ((cucon_ucset_leaf_t)lhs)->bitset[i]
	     & ((cucon_ucset_leaf_t)rhs)->bitset[i];
	if (bits)
	    have_nonzero = cu_true;
	node->bitset[i] = bits;
    }
    if (have_nonzero) {
#if CUCON_UCSET_ENABLE_HCONS
	return (cucon_ucset_t)cuoo_hctem_new(cucon_ucset_leaf, tem);
#else
	return (cucon_ucset_t)node;
#endif
    }
    else
	return NULL;
}

CU_SINLINE cucon_ucset_t
_ucleaf_minus(cucon_ucset_t lhs, cucon_ucset_t rhs)
{
    int i;
    cu_bool_t have_nonzero = cu_false;
#if CUCON_UCSET_ENABLE_HCONS
    cucon_ucset_leaf_t node;
    cuoo_hctem_decl(cucon_ucset_leaf, tem);
    cuoo_hctem_init(cucon_ucset_leaf, tem);
    node = cuoo_hctem_get(cucon_ucset_leaf, tem);
#else
    cucon_ucset_leaf_t node = cu_gnew(struct cucon_ucset_leaf);
#endif
    node->key = lhs->key;
    for (i = 0; i < BITSET_SIZEW; ++i) {
	uintptr_t bits;
	bits = ((cucon_ucset_leaf_t)lhs)->bitset[i]
	     & ~((cucon_ucset_leaf_t)rhs)->bitset[i];
	node->bitset[i] = bits;
	if (bits)
	    have_nonzero = cu_true;
	node->bitset[i] = bits;
    }
    if (have_nonzero) {
#if CUCON_UCSET_ENABLE_HCONS
	return (cucon_ucset_t)cuoo_hctem_new(cucon_ucset_leaf, tem);
#else
	return (cucon_ucset_t)node;
#endif
    }
    else
	return NULL;
}

CU_SINLINE cucon_ucset_t
_ucleaf_filter(cucon_ucset_t src_node, cu_clop(f, cu_bool_t, uintptr_t))
{
    int i, key;
    cu_bool_t have_change = cu_false, have_nonzero = cu_false;
#if CUCON_UCSET_ENABLE_HCONS
    cucon_ucset_leaf_t node;
    cuoo_hctem_decl(cucon_ucset_leaf, tem);
    cuoo_hctem_init(cucon_ucset_leaf, tem);
    node = cuoo_hctem_get(cucon_ucset_leaf, tem);
#else
    cucon_ucset_leaf_t node = cu_gnew(struct cucon_ucset_leaf);
#endif
    node->key = src_node->key;
    key = node->key & ~BITSET_MASK;
    for (i = 0; i < BITSET_SIZEW; ++i) {
	cu_word_t bit;
	cu_word_t bits = ((cucon_ucset_leaf_t)src_node)->bitset[i];
	for (bit = 1; bit; bit <<= 1) {
	    if (bits & bit) {
		if (!cu_call(f, key)) {
		    bits &= ~bit;
		    have_change = cu_true;
		}
	    }
	    ++key;
	}
	if (bits != 0)
	    have_nonzero = cu_true;
	node->bitset[i] = bits;
    }
    if (have_change) {
	if (have_nonzero) {
#if CUCON_UCSET_ENABLE_HCONS
	    return (cucon_ucset_t)cuoo_hctem_new(cucon_ucset_leaf, tem);
#else
	    return (cucon_ucset_t)node;
#endif
	}
	else
	    return NULL;
    }
    else
	return src_node;
}

#if !CUCON_UCSET_ENABLE_HCONS
cu_bool_t
cucon_ucset_eq(cucon_ucset_t set0, cucon_ucset_t set1)
{
    while (set0 != set1) {
	if (!set0 || !set1)
	    return cu_false;
	else if (set0->key != set1->key)
	    return cu_false;
	else if (_key_is_leaflike(_ucnode_key(set0)))
	    return _ucleaf_eq(set0, set1);
	else if (!cucon_ucset_eq(set0->left, set1->left))
	    return cu_false;
	else {
	    set0 = set0->right;
	    set1 = set1->right;
	}
    }
    return cu_true;
}
#endif

cu_bool_t
cucon_ucset_subeq(cucon_ucset_t lhs, cucon_ucset_t rhs)
{
    while (lhs != rhs) {
	uintptr_t lhs_key, rhs_key;
	if (!lhs)
	    return cu_true;
	if (!rhs)
	    return cu_false;
	lhs_key = _ucnode_key(lhs);
	rhs_key = _ucnode_key(rhs);
	if (lhs_key == rhs_key) {
	    if (_key_is_leaflike(lhs_key))
		return _ucleaf_subeq(lhs, rhs);
	    if ((lhs->key & 1) > (rhs->key & 1))
		return cu_false;
	    if (!cucon_ucset_subeq(lhs->left, rhs->left))
		return cu_false;
	    lhs = lhs->right;
	    rhs = rhs->right;
	}
	else if (_key_covers(rhs_key, lhs_key)) {
	    if (lhs_key < rhs_key)
		return cucon_ucset_subeq(lhs, rhs->left);
	    else
		return cucon_ucset_subeq(lhs, rhs->right);
	}
	else
	    return cu_false;
    }
    return cu_true;
}

cu_bool_t
cucon_ucset_disjoint(cucon_ucset_t lhs, cucon_ucset_t rhs)
{
    while (lhs && rhs) {
	uintptr_t lhs_key, rhs_key;
	if (lhs == rhs)
	    return cu_false;
	lhs_key = _ucnode_key(lhs);
	rhs_key = _ucnode_key(rhs);
	if (lhs_key == rhs_key) {
	    if (_key_is_leaflike(lhs_key)) {
		int i;
		for (i = 0; i < BITSET_SIZEW; ++i)
		    if (((cucon_ucset_leaf_t)lhs)->bitset[i] &
			((cucon_ucset_leaf_t)rhs)->bitset[i])
			return cu_false;
		return cu_true;
	    }
	    else if ((unsigned int)lhs->key & (unsigned int)rhs->key & 1)
		return cu_false;
	    else if (!cucon_ucset_disjoint(lhs->left, rhs->left))
		return cu_false;
	    lhs = lhs->right;
	    rhs = rhs->right;
	}
	else if (_key_covers(rhs_key, lhs_key)) {
	    if (lhs_key < rhs_key)
		rhs = rhs->left;
	    else
		rhs = rhs->right;
	}
	else if (_key_covers(lhs_key, rhs_key)) {
	    if (rhs_key < lhs_key)
		lhs = lhs->left;
	    else
		lhs = lhs->right;
	}
	else
	    return cu_true;
    }
    return cu_true;
}

cucon_ucset_t
cucon_ucset_singleton(uintptr_t key)
{
    if (_key_is_leaflike(key))
	return _ucleaf_new(key);
    else
	return _ucnode_new(key | 1, NULL, NULL);
}

cu_bool_t
cucon_ucset_is_singleton(cucon_ucset_t set)
{
    uintptr_t key = _ucnode_key(set);
    if (_key_is_leaflike(key))
	return _ucleaf_is_singleton(set);
    else
	return set->left == NULL && set->right == NULL;
}

static cucon_ucset_t
_ucset_leaf_from_sorted_array(uintptr_t **arr_io, size_t *len_io)
{
    cucon_ucset_leaf_t leaf;
#if CUCON_UCSET_ENABLE_HCONS
    cuoo_hctem_decl(cucon_ucset_leaf, tem);
#endif
    uintptr_t cur_key, cur_key_max;
    int j;

    if (*len_io == 0)
	return NULL;
    cur_key = **arr_io;

#if CUCON_UCSET_ENABLE_HCONS
    cuoo_hctem_init(cucon_ucset_leaf, tem);
    leaf = cuoo_hctem_get(cucon_ucset_leaf, tem);
#else
    leaf = cu_gnew(struct cucon_ucset_leaf);
#endif
    cur_key_max = cur_key | BITSET_MASK;

    leaf->key = _leaf_key(cur_key);
    do {
	do { ++*arr_io; --*len_io; } while (*len_io && **arr_io == cur_key);
	j = cur_key & BITSET_MASK;
	leaf->bitset[j / CU_WORD_WIDTH] |= (uintptr_t)1 << (j % CU_WORD_WIDTH);
	if (*len_io == 0) break;
	cur_key = **arr_io;
    } while (cur_key <= cur_key_max);

#if CUCON_UCSET_ENABLE_HCONS
    return (cucon_ucset_t)cuoo_hctem_new(cucon_ucset_leaf, tem);
#else
    return (cucon_ucset_t)leaf;
#endif
}

static cucon_ucset_t
_ucset_from_sorted_array(uintptr_t max_key,
			 uintptr_t **arr_io, size_t *len_io)
{
    uintptr_t cur_key, key, right_max;
    cucon_ucset_t cur_node, right_node;

    cur_key = **arr_io;
    if (!*len_io || cur_key > max_key)
	return NULL;

    if (_key_is_leaflike(cur_key)) {
	cur_node = _ucset_leaf_from_sorted_array(arr_io, len_io);
	cur_key = _ucnode_key(cur_node);
    }
    else {
	do { ++*arr_io; --*len_io; } while (*len_io && **arr_io == cur_key);
	right_node = _ucset_from_sorted_array(_key_max(cur_key), arr_io, len_io);
	cur_node = _ucnode_new(cur_key | (uintptr_t)1, NULL, right_node);
    }
    while (*len_io && **arr_io <= max_key) {
	cur_key = key = _key_join(cur_key, **arr_io);
	right_max = _key_max(key);
	if (**arr_io == key) {
	    do { ++*arr_io; --*len_io; } while (*len_io && **arr_io == key);
	    key |= (uintptr_t)1;
	}
	right_node = _ucset_from_sorted_array(right_max, arr_io, len_io);
	cur_node = _ucnode_new(key, cur_node, right_node);
    }
    return cur_node;
}

cucon_ucset_t
cucon_ucset_from_sorted_array(uintptr_t *arr, size_t len)
{
    cucon_ucset_t S = _ucset_from_sorted_array(UINTPTR_MAX, &arr, &len);
    cu_debug_assert(!len);
    return S;
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
	    cucon_ucset_t new_left = cucon_ucset_insert(left(node), key);
	    if (new_left != left(node))
		return _ucnode_new(node->key, new_left, right(node));
	    else
		return node;
	}
	else {
	    cucon_ucset_t new_right = cucon_ucset_insert(right(node), key);
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
	j = cu_uintptr_dcover(key ^ node_key);
	cu_debug_assert(j >= 2);
	j = (key & ~j) | ((j >> 1) + 1);
	if (key < node_key)
	    return _ucnode_new(j, key_node, node);
	else
	    return _ucnode_new(j, node, key_node);
    }
}

cucon_ucset_t
cucon_ucset_erase(cucon_ucset_t node, uintptr_t key)
{
    uintptr_t node_key;
    if (node == NULL)
	return NULL;
    node_key = _ucnode_key(node);
    if (key == node_key) {
	if (_key_is_leaflike(node_key))
	    return _ucleaf_erase(node, key);
	else if (node->key & 1) {
	    if (left(node) == NULL)
		return right(node);
	    if (right(node) == NULL)
		return left(node);
	    return _ucnode_new(node->key & ~CU_UINTPTR_C(1),
			       left(node), right(node));
	}
	else
	    return node;
    }
    else if (_key_covers(node_key, key)) {
	if (_key_is_leaflike(node_key))
	    return _ucleaf_erase(node, key);
	else if (key < node_key) {
	    cucon_ucset_t new_left = cucon_ucset_erase(left(node), key);
	    if (new_left != left(node)) {
		if (new_left == NULL && !(node->key & 1))
		    return right(node);
		return _ucnode_new(node->key, new_left, right(node));
	    }
	    else
		return node;
	}
	else {
	    cucon_ucset_t new_right = cucon_ucset_erase(right(node), key);
	    if (new_right != right(node)) {
		if (new_right == NULL && !(node->key & 1))
		    return left(node);
		return _ucnode_new(node->key, left(node), new_right);
	    }
	    else
		return node;
	}
    }
    else
	return node;
}

cucon_ucset_t
cucon_ucset_union(cucon_ucset_t lhs, cucon_ucset_t rhs)
{
    uintptr_t lhs_key, rhs_key;

    if (lhs == NULL)
	return rhs;
    if (rhs == NULL)
	return lhs;

    lhs_key = _ucnode_key(lhs);
    rhs_key = _ucnode_key(rhs);
    if (lhs_key == rhs_key) {
	if (_key_is_leaflike(lhs_key))
	    return _ucleaf_union(lhs, rhs);
	else {
	    cucon_ucset_t left = cucon_ucset_union(lhs->left, rhs->left);
	    cucon_ucset_t right = cucon_ucset_union(lhs->right, rhs->right);
	    return _ucnode_new(lhs->key | rhs->key, left, right);
	}
    }
    else if (_key_covers(rhs_key, lhs_key)) {
	cu_debug_assert(!_key_is_leaflike(rhs_key));
	if (lhs_key < rhs_key) {
	    cucon_ucset_t left = cucon_ucset_union(lhs, rhs->left);
	    return _ucnode_new(rhs->key, left, rhs->right);
	}
	else {
	    cucon_ucset_t right = cucon_ucset_union(lhs, rhs->right);
	    return _ucnode_new(rhs->key, rhs->left, right);
	}
    }
    else if (_key_covers(lhs_key, rhs_key))
	return cucon_ucset_union(rhs, lhs);
    else {
	uintptr_t j;
	j = cu_uintptr_dcover(lhs_key ^ rhs_key);
	cu_debug_assert(j >= 2);
	j = (lhs_key & ~j) | ((j >> 1) + 1);
	if (lhs_key < rhs_key)
	    return _ucnode_new(j, lhs, rhs);
	else
	    return _ucnode_new(j, rhs, lhs);
    }
}

cucon_ucset_t
cucon_ucset_isecn(cucon_ucset_t lhs, cucon_ucset_t rhs)
{
    uintptr_t lhs_key, rhs_key;

    if (lhs == NULL || rhs == NULL)
	return NULL;

    lhs_key = _ucnode_key(lhs);
    rhs_key = _ucnode_key(rhs);
    if (lhs_key == rhs_key) {
	if (_key_is_leaflike(lhs_key))
	    return _ucleaf_isecn(lhs, rhs);
	else {
	    uintptr_t key = lhs->key & rhs->key;
	    cucon_ucset_t left = cucon_ucset_isecn(lhs->left, rhs->left);
	    cucon_ucset_t right = cucon_ucset_isecn(lhs->right, rhs->right);
	    if (!(key & 1)) {
		if (!left)  return right;
		if (!right) return left;
	    }
	    return _ucnode_new(key, left, right);
	}
    }
    else if (_key_covers(rhs_key, lhs_key)) {
	cu_debug_assert(!_key_is_leaflike(rhs_key));
	if (lhs_key < rhs_key)
	    return cucon_ucset_isecn(lhs, rhs->left);
	else
	    return cucon_ucset_isecn(lhs, rhs->right);
    }
    else if (_key_covers(lhs_key, rhs_key))
	return cucon_ucset_isecn(rhs, lhs);
    else
	return NULL;
}

cucon_ucset_t
cucon_ucset_compl(cucon_ucset_t lhs, cucon_ucset_t rhs)
{
    uintptr_t lhs_key, rhs_key;

    if (lhs == NULL)
	return NULL;
    if (rhs == NULL)
	return lhs;

    lhs_key = _ucnode_key(lhs);
    rhs_key = _ucnode_key(rhs);
    if (lhs_key == rhs_key) {
	if (_key_is_leaflike(lhs_key))
	    return _ucleaf_minus(lhs, rhs);
	else {
	    uintptr_t key = lhs->key & ~(rhs->key & CU_UINTPTR_C(1));
	    cucon_ucset_t left = cucon_ucset_compl(lhs->left, rhs->left);
	    cucon_ucset_t right = cucon_ucset_compl(lhs->right, rhs->right);
	    if (!(key & 1)) {
		if (!left)  return right;
		if (!right) return left;
	    }
	    return _ucnode_new(key, left, right);
	}
    }
    else if (_key_covers(lhs_key, rhs_key)) {
	if (rhs_key < lhs_key) {
	    cucon_ucset_t left = cucon_ucset_compl(lhs->left, rhs);
	    if (left == NULL && !(lhs->key & 1))
		return lhs->right;
	    return _ucnode_new(lhs->key, left, lhs->right);
	}
	else {
	    cucon_ucset_t right = cucon_ucset_compl(lhs->right, rhs);
	    if (right == NULL && !(lhs->key & 1))
		return lhs->left;
	    return _ucnode_new(lhs->key, lhs->left, right);
	}
    }
    else if (_key_covers(rhs_key, lhs_key)) {
	if (lhs_key < rhs_key)
	    return cucon_ucset_compl(lhs, rhs->left);
	else
	    return cucon_ucset_compl(lhs, rhs->right);
    }
    else
	return lhs;
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
	    uintptr_t k = key & ~BITSET_MASK;
	    do
		if (_ucleaf_find(node, k) && !cu_call(cb, k))
		    return cu_false;
	    while (++k & BITSET_MASK);
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
	    uintptr_t k = key & ~BITSET_MASK;
	    do
		if (_ucleaf_find(node, k))
		    cu_call(f, k);
	    while (++k & BITSET_MASK);
	    return;
	}
	else {
	    cucon_ucset_iter(node->left, f);
	    if ((node->key & 1))
		cu_call(f, key);
	    node = node->right;
	}
    }
}

cucon_ucset_t
cucon_ucset_filter(cucon_ucset_t node, cu_clop(f, cu_bool_t, uintptr_t key))
{
    while (node) {
	uintptr_t key = _ucnode_key(node);
	if (_key_is_leaflike(key))
	    return _ucleaf_filter(node, f);
	else {
	    cucon_ucset_t left = cucon_ucset_filter(node->left, f);
	    cucon_ucset_t right = cucon_ucset_filter(node->right, f);
	    if ((node->key & 1) && cu_call(f, key))
		return _ucnode_new(key | 1, left, right);
	    else if (!left)
		return right;
	    else if (!right)
		return left;
	    else
		return _ucnode_new(key, left, right);
	}
    }
    return NULL;
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
cucon_ucset_umin(cucon_ucset_t set)
{
    uintptr_t min = UINTPTR_MAX;
    while (set) {
	uintptr_t key = _ucnode_key(set);
	if (_key_is_leaflike(key)) {
	    int i;
	    for (i = 0; i < BITSET_SIZEW; ++i) {
		cu_word_t x = ((cucon_ucset_leaf_t)set)->bitset[i];
		if (x)
		    return (key & ~BITSET_MASK) + cu_word_log2_lowbit(x)
			 + i*CU_WORD_P2WIDTH;
	    }
	    return key;
	}
	min = key;
	set = set->left;
    }
    return min;
}

uintptr_t
cucon_ucset_umax(cucon_ucset_t set)
{
    uintptr_t max = 0;
    while (set) {
	uintptr_t key = _ucnode_key(set);
	if (_key_is_leaflike(key)) {
	    int i;
	    for (i = BITSET_SIZEW - 1; i >= 0; --i) {
		cu_word_t x = ((cucon_ucset_leaf_t)set)->bitset[i];
		if (x)
		    return (key & ~BITSET_MASK) + cu_word_floor_log2(x)
			 + i*CU_WORD_P2WIDTH;
	    }
	    return key;
	}
	max = key;
	set = set->right;
    }
    return max;
}

intptr_t
cucon_ucset_smin(cucon_ucset_t node)
{
    uintptr_t node_key;
    intptr_t min_key;
    if (node == NULL)
	return INTPTR_MAX;
    min_key = INTPTR_MAX;
    node_key = _ucnode_key(node);
#ifndef LEAFLIKE_ZERO
    if (node_key == 0) {
	if (node->key & 1)
	    min_key = 0;
	node = node->right;
	if (node == NULL)
	    return min_key;
	node_key = _ucnode_key(node);
    }
#endif
    if (node_key == INTPTR_MIN) {
	if (node->key & 1)
	    return INTPTR_MIN;
	if (node->right)
	    node = node->right;
	else {
	    node = node->left;
	    if (node == NULL)
		return min_key;
	}
    }
    else if ((intptr_t)node_key > min_key)
	return 0;
    return cucon_ucset_umin(node);
}

intptr_t
cucon_ucset_smax(cucon_ucset_t node)
{
    uintptr_t node_key;
    intptr_t max_key;
    if (node == NULL)
	return INTPTR_MIN;
    max_key = INTPTR_MIN;
    node_key = _ucnode_key(node);
#ifndef LEAFLIKE_ZERO
    if (node_key == 0) {
	if (node->key & 1)
	    max_key = 0;
	node = node->right;
	if (node == NULL)
	    return max_key;
	node_key = _ucnode_key(node);
    }
#endif
    if (node_key == INTPTR_MIN) {
	if (node->left)
	    node = node->left;
	else if (max_key == 0)
	    return 0;
	else {
	    node = node->right;
	    if (node == NULL)
		return max_key;
	}
    }
    else if ((intptr_t)node_key < max_key)
	return 0;

    return cucon_ucset_umax(node);
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

cu_clos_def(_ucset_filter_image_helper, cu_prot(void, uintptr_t key),
    ( cucon_ucset_t image;
      cu_clop(f, cu_bool_t, uintptr_t *); ))
{
    cu_clos_self(_ucset_filter_image_helper);
    if (cu_call(self->f, &key))
	self->image = cucon_ucset_insert(self->image, key);
}

cucon_ucset_t
cucon_ucset_filter_image(cucon_ucset_t set, cu_clop(f, cu_bool_t, uintptr_t *))
{
    _ucset_filter_image_helper_t cb;
    cb.image = NULL;
    cucon_ucset_iter(set, _ucset_filter_image_helper_prep(&cb));
    return cb.image;
}

static cucon_ucset_t
_ucleaf_uclip(cucon_ucset_t node, uintptr_t clip_min, uintptr_t clip_max)
{
    uintptr_t node_key = node->key;
    uintptr_t node_min = node_key & ~BITSET_MASK;
    uintptr_t node_max = node_key | BITSET_MASK;
    if (clip_max < node_min || clip_min > node_max)
	return NULL;
    else if (clip_min <= node_min && clip_max >= node_max)
	return node;
    else {
	int i;
	uintptr_t bs_min, bs_max;
	cucon_ucset_leaf_t new_leaf;
	cu_bool_t is_empty = cu_true;
#if CUCON_UCSET_ENABLE_HCONS
	cuoo_hctem_decl(cucon_ucset_leaf, tem);
	cuoo_hctem_init(cucon_ucset_leaf, tem);
	new_leaf = cuoo_hctem_get(cucon_ucset_leaf, tem);
#else
	new_leaf = cu_gnew(struct cucon_ucset_leaf);
#endif
	bs_min = node_min;
	bs_max = node_min + CU_WORD_P2WIDTH - 1;
	for (i = 0; i < BITSET_SIZEW; ++i) {
	    if (clip_min > bs_max || clip_max < bs_min)
		new_leaf->bitset[i] = 0;
	    else {
		cu_word_t bitset;
		uintptr_t mask = ~CU_UINTPTR_C(0);
		if (clip_min > bs_min)
		    mask &= ~((CU_UINTPTR_C(1) << (clip_min - bs_min)) - 1);
		if (clip_max < bs_max)
		    mask &= (CU_UINTPTR_C(2) << (clip_max - bs_min)) - 1;
		bitset = ((cucon_ucset_leaf_t)node)->bitset[i] & mask;
		new_leaf->bitset[i] = bitset;
		if (bitset)
		    is_empty = cu_false;
	    }
	    bs_min += CU_WORD_P2WIDTH;
	    bs_max += CU_WORD_P2WIDTH;
	}
	if (is_empty)
	    return NULL;
	new_leaf->key = node->key;
#if CUCON_UCSET_ENABLE_HCONS
	return (cucon_ucset_t)cuoo_hctem_new(cucon_ucset_leaf, tem);
#else
	return (cucon_ucset_t)new_leaf;
#endif
    }
}

static cucon_ucset_t
_ucset_uclip(cucon_ucset_t node, uintptr_t clip_min, uintptr_t clip_max)
{
    uintptr_t node_key, node_mask, node_min, node_max;
tail_call:
    if (node == NULL)
	return NULL;
    node_key = _ucnode_key(node);
    node_mask = node_key ^ (node_key - 1);
    node_min = node_key & ~node_mask;
    node_max = node_key | node_mask;
    if (clip_min <= node_min && node_max <= clip_max)
	return node;
    else if (clip_max < node_min)
	return NULL;
    else if (clip_min > node_max)
	return NULL;
    else if (_key_is_leaflike(node_key))
	return _ucleaf_uclip(node, clip_min, clip_max);
    else if (clip_max < node_key) {
	node = node->left;
	goto tail_call;
    }
    else if (clip_min > node_key) {
	node = node->right;
	goto tail_call;
    }
    else {
	cucon_ucset_t new_left, new_right;
	cu_debug_assert(clip_min <= node_key && node_key <= clip_max);
	new_left = _ucset_uclip(node->left, clip_min, clip_max);
	new_right = _ucset_uclip(node->right, clip_min, clip_max);
	if (node->key & 1)
	    return _ucnode_new(node->key, new_left, new_right);
	else if (!new_left)
	    return new_right;
	else if (!new_right)
	    return new_left;
	else
	    return _ucnode_new(node->key, new_left, new_right);
    }
}

cucon_ucset_t
cucon_ucset_uclip(cucon_ucset_t node, uintptr_t clip_min, uintptr_t clip_max)
{
    if (clip_min > clip_max)
	return NULL;
    else
	return _ucset_uclip(node, clip_min, clip_max);
}

cucon_ucset_t
cucon_ucset_sclip(cucon_ucset_t node, intptr_t clip_min, intptr_t clip_max)
{
    if (clip_min > clip_max)
	return NULL;
    else if ((clip_min < 0) == (clip_max < 0))
	return _ucset_uclip(node, clip_min, clip_max);
    else {
	cucon_ucset_t S0 = _ucset_uclip(node, 0, clip_max);
	cucon_ucset_t S1 = _ucset_uclip(node, clip_min, UINTPTR_MAX);
	return cucon_ucset_union(S0, S1);
    }
}

static cucon_ucset_t
_ucleaf_from_monotonic(cu_clop(f, cu_bool_t, uintptr_t *),
		       cu_bool_t *have_next, uintptr_t *next_key,
		       uintptr_t clip_max)
{
    cucon_ucset_leaf_t leaf;
    uintptr_t key, max_key;
#if CUCON_UCSET_ENABLE_HCONS
    cuoo_hctem_decl(cucon_ucset_leaf, tem);
    cuoo_hctem_init(cucon_ucset_leaf, tem);
    leaf = cuoo_hctem_get(cucon_ucset_leaf, tem);
#else
    leaf = cu_gnew(struct cucon_ucset_leaf);
#endif
    cu_debug_assert(*have_next);
    max_key = cu_uintptr_min(clip_max, *next_key | BITSET_MASK);
    cu_debug_assert(*next_key <= max_key);
    leaf->key = (*next_key & ~BITSET_MASK) | ((BITSET_MASK + 1) >> 1);
    memset(leaf->bitset, 0, sizeof(leaf->bitset));
    while (*have_next && (key = *next_key) <= max_key) {
	key &= BITSET_MASK;
	leaf->bitset[key / CU_WORD_P2WIDTH] |=
	    CU_WORD_C(1) << (key % CU_WORD_P2WIDTH);
	*have_next = cu_call(f, next_key);
    }
#if CUCON_UCSET_ENABLE_HCONS
    return (cucon_ucset_t)cuoo_hctem_new(cucon_ucset_leaf, tem);
#else
    return (cucon_ucset_t)leaf;
#endif
}

static cucon_ucset_t
_ucset_from_monotonic(cu_clop(f, cu_bool_t, uintptr_t *),
		      cu_bool_t *have_next, uintptr_t *next_key,
		      uintptr_t clip_max)
{
    cucon_ucset_t left, right;
    uintptr_t left_key;
    if (!*have_next || *next_key > clip_max)
	return NULL;
    if (_key_is_leaflike(*next_key)) {
	left = _ucleaf_from_monotonic(f, have_next, next_key, clip_max);
	left_key = _ucnode_key(left);
    }
#ifndef LEAFLIKE_ZERO
    else if (*next_key == 0) {
	*have_next = cu_call(f, next_key);
	right = _ucset_from_monotonic(f, have_next, next_key, clip_max);
	return _ucnode_new(1, NULL, right);
    }
#endif
    else {
	left = NULL;
	left_key = *next_key - 1;
    }
    while (*have_next && *next_key <= clip_max) {
	uintptr_t key_diff = left_key ^ *next_key;
	uintptr_t span_lmask = key_diff ? cu_uintptr_dcover(key_diff)
					: ~CU_UINTPTR_C(0);
	uintptr_t span_min = left_key & ~span_lmask;
	uintptr_t span_max = left_key | span_lmask;
	uintptr_t span_key = span_min + ((span_lmask >> 1) + 1);
	if (span_key == *next_key) {
	    span_key |= 1;
	    *have_next = cu_call(f, next_key);
	}
	cu_debug_assert(span_max <= clip_max);
	cu_debug_assert(left_key != *next_key);
	right = _ucset_from_monotonic(f, have_next, next_key, span_max);
	left = _ucnode_new(span_key, left, right);
    }
    return left;
}

cu_clos_def(_ucset_translate_helper, cu_prot(cu_bool_t, uintptr_t *val_out),
    ( cucon_ucset_itr_t itr;
      intptr_t diff;
      uintptr_t clip_max;
      cu_bool_t cut_have_next;
      uintptr_t cut_next_val; ))
{
    cu_clos_self(_ucset_translate_helper);
    uintptr_t val;
    if (cucon_ucset_itr_at_end(self->itr))
	return cu_false;
    val = cucon_ucset_itr_get(self->itr);
    if (val > self->clip_max) {
	self->cut_next_val = val;
	self->cut_have_next = cu_true;
	return cu_false;
    }
    *val_out = val + self->diff;
    return cu_true;
}

cucon_ucset_t
cucon_ucset_translate_uclip(cucon_ucset_t set, intptr_t diff,
			    uintptr_t clip_min, uintptr_t clip_max)
{
    _ucset_translate_helper_t cb;
    uintptr_t next_key;
    cu_bool_t have_next;
    cucon_ucset_t S0, S1;

    _ucset_translate_helper_init(&cb);
    cb.diff = diff;
    cb.itr = cu_salloc(cucon_ucset_itr_size(set));
    cucon_ucset_itr_init(cb.itr, set);

    /* Skip elements in [0, clip_min - 1). */
    do {
	if (cucon_ucset_itr_at_end(cb.itr))
	    return NULL;
	next_key = cucon_ucset_itr_get(cb.itr);
	if (next_key > clip_max)
	    return NULL;
    } while (next_key < clip_min);
    have_next = cu_true;
    next_key += diff;

    /* Process elements in [clip_min, - diff). */
    cb.clip_max = - diff - 1;
    cb.cut_have_next = cu_false;
    if (cb.clip_max < clip_max) {
	S0 = _ucset_from_monotonic(_ucset_translate_helper_ref(&cb),
				   &have_next, &next_key, UINTPTR_MAX);
	if (!cb.cut_have_next)
	    return S0;
	if (cb.cut_next_val > clip_max)
	    return S0;
	have_next = cu_true;
	next_key = cb.cut_next_val + diff;
    }
    else
	S0 = NULL;

    /* Process elements in [- diff, clip_max]. */
    cb.clip_max = clip_max;
    S1 = _ucset_from_monotonic(_ucset_translate_helper_prep(&cb),
			       &have_next, &next_key, UINTPTR_MAX);

    /* Return the union. */
    return cucon_ucset_union(S0, S1);
}

cucon_ucset_t
cucon_ucset_translate_sclip(cucon_ucset_t set, intptr_t diff,
			    intptr_t clip_min, intptr_t clip_max)
{
    if (clip_min > clip_max)
	return NULL;
    else if ((clip_min < 0) == (clip_max < 0))
	return cucon_ucset_translate_uclip(set, diff, clip_min, clip_max);
    else {
	cucon_ucset_t S0, S1;
	S0 = cucon_ucset_translate_uclip(set, diff, 0, clip_max);
	S1 = cucon_ucset_translate_uclip(set, diff, clip_min, UINTPTR_MAX);
	return cucon_ucset_union(S0, S1);
    }
}

cucon_ucset_t
cucon_ucset_translate(cucon_ucset_t set, intptr_t diff)
{
    return cucon_ucset_translate_uclip(set, diff, 0, UINTPTR_MAX);
}

cu_clos_def(_ucset_fprint_helper, cu_prot(void, uintptr_t x),
    ( FILE *out;
      char const *format;
      int i; ))
{
    cu_clos_self(_ucset_fprint_helper);
    if (self->i++)
	fputs(", ", self->out);
    fprintf(self->out, self->format, x);
}

void
cucon_ucset_fprint_uintptr(cucon_ucset_t set, FILE *out)
{
    _ucset_fprint_helper_t cb;
    cb.out = out;
    cb.format = "%"PRIuPTR;
    cb.i = 0;
    fputc('{', out);
    cucon_ucset_iter(set, _ucset_fprint_helper_prep(&cb));
    fputc('}', out);
}

void
cucon_ucset_fprint_intptr(cucon_ucset_t set, FILE *out)
{
    _ucset_fprint_helper_t cb;
    cb.out = out;
    cb.format = "%"PRIdPTR;
    cb.i = 0;
    fputc('{', out);
    cucon_ucset_iter(set, _ucset_fprint_helper_prep(&cb));
    fputc('}', out);
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
	    for (i = 0; i < BITSET_SIZEW; ++i)
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
    fprintf(out, "cucon_ucset_t @ %p\n", (void *)tree);
    _ucset_dump(tree, 2, out);
}

size_t
cucon_ucset_itr_size(cucon_ucset_t set)
{
    uintptr_t key;
    int l;
    if (!set)
	return sizeof(struct cucon_ucset_itr) - sizeof(cucon_ucset_t);
    key = _ucnode_key(set);
    if (_key_is_leaflike(key))
	l = 1;
#ifndef LEAFLIKE_ZERO
    else if (key == 0)
	l = CUCONF_WIDTHOF_INTPTR_T + BITSET_LOG2_WIDTH + 2;
#endif
    else
	l = cu_uintptr_log2_lowbit(key) - BITSET_LOG2_WIDTH + 2;
    return sizeof(struct cucon_ucset_itr) + (l - 1)*sizeof(cucon_ucset_t);
}

/* Assumign itr is at a leaf, go to the first element. */
static void
_ucset_itr_reset_leaf(cucon_ucset_itr_t itr)
{
    int i;
    cucon_ucset_t node = itr->node_stack[itr->sp];
    for (i = 0; i < BITSET_SIZEW; ++i) {
	cu_word_t bitset = ((cucon_ucset_leaf_t)node)->bitset[i];
	if (bitset) {
	    itr->pos = cu_word_log2_lowbit(bitset) + i*CU_WORD_P2WIDTH;
	    return;
	}
    }
}

/* Move itr to the first element at or below the current element. */
static void
_ucset_itr_descend(cucon_ucset_itr_t itr)
{
    cucon_ucset_t node = itr->node_stack[itr->sp];
    for (;;) {
	uintptr_t key;
	key = _ucnode_key(node);
	if (_key_is_leaflike(key)) {
	    _ucset_itr_reset_leaf(itr);
	    return;
	}
	if (node->left == NULL)
	    return;
	node = node->left;
	itr->node_stack[++itr->sp] = node;
    }
}

static void
_ucset_itr_ascend(cucon_ucset_itr_t itr)
{
    while (--itr->sp >= 0 &&
	   itr->node_stack[itr->sp]->right == itr->node_stack[itr->sp + 1]);
}

void
cucon_ucset_itr_init(cucon_ucset_itr_t itr, cucon_ucset_t set)
{
    if (set == NULL)
	itr->sp = -1;
    else {
	itr->sp = 0;
	itr->pos = -1;
	itr->node_stack[0] = set;
	_ucset_itr_descend(itr);
    }
}

cucon_ucset_itr_t
cucon_ucset_itr_new(cucon_ucset_t set)
{
    cucon_ucset_itr_t itr = cu_galloc(cucon_ucset_itr_size(set));
    cucon_ucset_itr_init(itr, set);
    return itr;
}

uintptr_t
cucon_ucset_itr_get(cucon_ucset_itr_t itr)
{
    uintptr_t key, res;
    cucon_ucset_t node;
    cu_debug_assert(itr->sp != -1);
    node = itr->node_stack[itr->sp];
    key = _ucnode_key(node);
    if (_key_is_leaflike(key)) {
	int i, j;
	res = key - BITSET_WIDTH/2 + itr->pos;
	++itr->pos;
	i = itr->pos / CU_WORD_P2WIDTH;
	j = itr->pos % CU_WORD_P2WIDTH;
	while (i < BITSET_SIZEW) {
	    cu_word_t bitset = ((cucon_ucset_leaf_t)node)->bitset[i];
	    while (j < CU_WORD_P2WIDTH) {
		if (bitset & (CU_WORD_C(1) << j)) {
		    itr->pos = j + i*CU_WORD_P2WIDTH;
		    return res;
		}
		++j;
	    }
	    j = 0;
	    ++i;
	}
    }
    else if (node->right) {
	node = node->right;
	itr->node_stack[++itr->sp] = node;
	_ucset_itr_descend(itr);
	return key;
    }
    else
	res = key;

    /* No more element below the current node. */
    _ucset_itr_ascend(itr);
    if (itr->sp < 0)
	return res;
    node = itr->node_stack[itr->sp];
    cu_debug_assert(!_key_is_leaflike(_ucnode_key(node)));
    if (!(node->key & 1)) {
	node = node->right;
	itr->node_stack[++itr->sp] = node;
	_ucset_itr_descend(itr);
    }
    return res;
}

/* This is also referred from cufo/init_formats.c. */
cu_box_t cuconP_ucset_foprint = CU_BOX_NULL_FPTR_INIT;

#if CUCON_UCSET_ENABLE_HCONS
static void
_ucset_print(cuex_t e, FILE *out)
{ cucon_ucset_dump(e, out); }

static cu_box_t
_ucset_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN:
	    return cu_box_fptr(cuoo_intf_print_fn_t, _ucset_print);
	case CUOO_INTF_FOPRINT_FN:
	    return cuconP_ucset_foprint;
	default:
	    return CUOO_IMPL_NONE;
    }
}
#endif

void
cuconP_ucset_init()
{
    cu_debug_assert_once();
#if CUCON_UCSET_ENABLE_HCONS
    _ucset_type = cuoo_type_new_opaque_hcs(
	_ucset_impl, sizeof(struct cucon_ucset) - CUOO_HCOBJ_SHIFT);
    _ucset_leaf_type = cuoo_type_new_opaque_hcs(
	_ucset_impl, sizeof(struct cucon_ucset_leaf) - CUOO_HCOBJ_SHIFT);
#endif
}
