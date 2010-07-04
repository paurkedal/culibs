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

#include <cucon/ucmap.h>
#include <cucon/ucmap_priv.h>
#include <cu/memory.h>
#include <cu/int.h>
#include <cuoo/hctem.h>
#include <cuoo/intf.h>
#include <inttypes.h>

#define _node_left	cucon_ucmap_node_left
#define _node_right	cucon_ucmap_node_right
#define _node_key	cucon_ucmap_node_key
#define _node_has_value	cucon_ucmap_node_has_value
#define _node_value	cucon_ucmap_node_value
#define _node_value_ptr	cucon_ucmap_node_value_ptr

CU_SINLINE cu_bool_t
_key_covers(uintptr_t key0, uintptr_t key1)
{
    uintptr_t mask0 = ~(key0 ^ (key0 - 1));
    return (key0 & mask0) == (key1 & mask0) && (key1 & ~mask0);
}
CU_SINLINE cu_bool_t
_key_coverseq(uintptr_t key0, uintptr_t key1)
{
    uintptr_t mask0 = ~(key0 ^ (key0 - 1));
    return (key0 & mask0) == (key1 & mask0);
}

/* Find the tightest cover for k0 and k1, provided that neither of these cover
 * the other. */
CU_SINLINE uintptr_t
_key_join(uintptr_t k0, uintptr_t k1)
{
    uintptr_t k = cu_ulong_dcover(k0 ^ k1);
    cu_debug_assert(k >= 2);
    k = (k0 & ~k) | ((k >> 1) + (uintptr_t)1);
    return k;
}

static cucon_ucmap_t
_node_new_val(uintptr_t key, cucon_ucmap_t left, cucon_ucmap_t right,
	      uintptr_t val)
{
    left = (cucon_ucmap_t)((uintptr_t)left | 1);
#if CUCONP_UCMAP_ENABLE_HCONS
    cuoo_hctem_decl(cucon_ucmap, tem);
    cuoo_hctem_init(cucon_ucmap, tem);
    cuoo_hctem_get(cucon_ucmap, tem)->key = key;
    cuoo_hctem_get(cucon_ucmap, tem)->left = left;
    cuoo_hctem_get(cucon_ucmap, tem)->right = right;
    cuoo_hctem_get(cucon_ucmap, tem)->value = (uintptr_t)val;
    return cuoo_hctem_new(cucon_ucmap, tem);
#else
    cucon_ucmap_t node = cu_gnew(struct cucon_ucmap);
    node->key = key;
    node->left = left;
    node->right = right;
    node->value = (uintptr_t)val;
    return node;
#endif
}

static cucon_ucmap_t
_node_new_noval(uintptr_t key, cucon_ucmap_t left, cucon_ucmap_t right)
{
#if CUCONP_UCMAP_ENABLE_HCONS
    cuoo_hctem_decl(cucon_ucmap, tem);
    cuoo_hctem_init(cucon_ucmap, tem);
    cuoo_hctem_get(cucon_ucmap, tem)->key = key;
    cuoo_hctem_get(cucon_ucmap, tem)->left = left;
    cuoo_hctem_get(cucon_ucmap, tem)->right = right;
    cuoo_hctem_get(cucon_ucmap, tem)->value = 0;
    return cuoo_hctem_new(cucon_ucmap, tem);
#else
    cucon_ucmap_t node = cu_gnew(struct cucon_ucmap);
    node->key = key;
    node->left = left;
    node->right = right;
    node->value = 0;
    return node;
#endif
}

static cucon_ucmap_t
_node_new_lr(cucon_ucmap_t src_node, cucon_ucmap_t left, cucon_ucmap_t right)
{
    left = (cucon_ucmap_t)((uintptr_t)left | ((uintptr_t)src_node->left & 1));
#if CUCONP_UCMAP_ENABLE_HCONS
    cuoo_hctem_decl(cucon_ucmap, tem);
    cuoo_hctem_init(cucon_ucmap, tem);
    cuoo_hctem_get(cucon_ucmap, tem)->key = src_node->key;
    cuoo_hctem_get(cucon_ucmap, tem)->left = left;
    cuoo_hctem_get(cucon_ucmap, tem)->right = right;
    cuoo_hctem_get(cucon_ucmap, tem)->value = src_node->value;
    return cuoo_hctem_new(cucon_ucmap, tem);
#else
    cucon_ucmap_t node = cu_gnew(struct cucon_ucmap);
    node->key = src_node->key;
    node->left = left;
    node->right = right;
    node->value = src_node->value;
    return node;
#endif
}

cucon_ucmap_t
cucon_ucmap_singleton(uintptr_t key, uintptr_t val)
{
    return _node_new_val(key, NULL, NULL, val);
}

static cucon_ucmap_t
_ucmap_from_sorted_array(uintptr_t max_key,
			 struct cucon_ucmap_element (**arr_io), size_t *len_io)
{
    uintptr_t cur_key, next_key, cur_max, value;
    cucon_ucmap_t cur_node, right_node;

    if (*len_io == 0)
	return NULL;

    cur_key = (*arr_io)->key;
    if (cur_key > max_key)
	return NULL;

    /* Create a node for the first element and any subsequent elements which
     * belong to it's right branch. */
    value = (*arr_io)->value;
    do {
	++*arr_io; --*len_io;
	if (*len_io == 0)
	    return _node_new_val(cur_key, NULL, NULL, value);
    } while ((*arr_io)->key == cur_key);

    next_key = (*arr_io)->key;
    if (_key_covers(cur_key, next_key)) {
	cur_max = cur_key | (cur_key - 1);
	right_node = _ucmap_from_sorted_array(cur_max, arr_io, len_io);
	cur_node = _node_new_val(cur_key, NULL, right_node, value);
	if (*len_io == 0)
	    return cur_node;
    }
    else
	cur_node = _node_new_val(cur_key, NULL, NULL, value);

    /* Move outwards by exiting the left node.  On each iteration the next key
     * either covers our current node on the left, or we can create such a key
     * by joining the new key with the current. */
    do {
	next_key = (*arr_io)->key;
	if (next_key > max_key)
	    break;

	if (_key_covers(next_key, cur_key)) {
	    value = (*arr_io)->value;
	    do {
		++*arr_io; --*len_io;
		if (*len_io == 0)
		    return _node_new_val(next_key, cur_node, NULL, value);
	    } while ((*arr_io)->key == next_key);
	    cur_key = next_key;
	    cur_max = cur_key | (cur_key - 1);
	    right_node = _ucmap_from_sorted_array(cur_max, arr_io, len_io);
	    cur_node = _node_new_val(cur_key, cur_node, right_node, value);
	}
	else {
	    if (!_key_covers(cur_key, next_key))
		cur_key = _key_join(cur_key, next_key);
	    cur_max = cur_key | (cur_key - 1);
	    right_node = _ucmap_from_sorted_array(cur_max, arr_io, len_io);
	    cur_node = _node_new_noval(cur_key, cur_node, right_node);
	}
    } while (*len_io);
    return cur_node;
}

cucon_ucmap_t
cucon_ucmap_from_sorted_array(struct cucon_ucmap_element *arr, size_t len)
{
    cucon_ucmap_t M = _ucmap_from_sorted_array(UINTPTR_MAX, &arr, &len);
    cu_debug_assert(!len);
    return M;
}

cucon_ucmap_t
cucon_ucmap_insert(cucon_ucmap_t node, uintptr_t key, uintptr_t val)
{
    uintptr_t node_key;
    if (node == NULL)
	return _node_new_val(key, NULL, NULL, val);

    node_key = _node_key(node);
    if (node_key == key) {
	if (node->value == (uintptr_t)val)
	    return node;
	else
	    return _node_new_val(node->key,
				 _node_left(node), _node_right(node), val);
    }
    if (_key_covers(node_key, key)) {
	if (key < node_key) {
	    cucon_ucmap_t new_left;
	    new_left = cucon_ucmap_insert(_node_left(node), key, val);
	    if (new_left != _node_left(node))
		return _node_new_lr(node, new_left, _node_right(node));
	    else
		return node;
	}
	else {
	    cucon_ucmap_t new_right;
	    new_right = cucon_ucmap_insert(_node_right(node), key, val);
	    if (new_right != _node_right(node))
		return _node_new_lr(node, _node_left(node), new_right);
	    else
		return node;
	}
    }

    if (_key_covers(key, node_key)) {
	if (node_key < key)
	    return _node_new_val(key, node, NULL, val);
	else
	    return _node_new_val(key, NULL, node, val);
    }

    else {
	cucon_ucmap_t key_node;
	uintptr_t j;
	key_node = _node_new_val(key, NULL, NULL, val);
	j = _key_join(key, node_key);
	if (key < node_key)
	    return _node_new_noval(j, key_node, node);
	else
	    return _node_new_noval(j, node, key_node);
    }
}

cucon_ucmap_t
cucon_ucmap_erase(cucon_ucmap_t node, uintptr_t key)
{
    uintptr_t node_key;
    if (node == NULL)
	return node;

    node_key = _node_key(node);
    if (node_key == key) {
	if (_node_has_value(node)) {
	    if (!_node_left(node))
		return _node_right(node);
	    if (!_node_right(node))
		return _node_left(node);
	    return _node_new_noval(node->key,
				   _node_left(node), _node_right(node));
	}
	else
	    return node;
    }
    else if (_key_covers(node_key, key)) {
	if (key < node_key) {
	    cucon_ucmap_t new_left;
	    new_left = cucon_ucmap_erase(_node_left(node), key);
	    if (!new_left && !_node_has_value(node))
		return _node_right(node);
	    else if (new_left != _node_left(node))
		return _node_new_lr(node, new_left, _node_right(node));
	    else
		return node;
	}
	else {
	    cucon_ucmap_t new_right;
	    new_right = cucon_ucmap_erase(_node_right(node), key);
	    if (!new_right && !_node_has_value(node))
		return _node_left(node);
	    else if (new_right != _node_right(node))
		return _node_new_lr(node, _node_left(node), new_right);
	    else
		return node;
	}
    }
    else
	return node;
}

cu_bool_t
cucon_ucmap_contains(cucon_ucmap_t node, uintptr_t key)
{
    uintptr_t node_key;
tailcall:
    if (!node) return cu_false;
    node_key = _node_key(node);
    if (key == node_key) return _node_has_value(node);
    if (_key_coverseq(node_key, key)) {
	if (key < node_key) node = _node_left(node);
	else                node = _node_right(node);
	goto tailcall;
    }
    return cu_false;
}

cu_bool_t
cucon_ucmap_find(cucon_ucmap_t node, uintptr_t key, uintptr_t *val_out)
{
    uintptr_t node_key;
tailcall:
    if (!node) return cu_false;
    node_key = _node_key(node);
    if (key == node_key) {
	if (!_node_has_value(node)) return cu_false;
	*val_out = _node_value(node);
	return cu_true;
    }
    if (_key_coverseq(node_key, key)) {
	if (key < node_key) node = _node_left(node);
	else                node = _node_right(node);
	goto tailcall;
    }
    return cu_false;
}

void *
cucon_ucmap_find_ptr(cucon_ucmap_t node, uintptr_t key)
{
    uintptr_t node_key;
tailcall:
    if (!node)
	return NULL;
    node_key = _node_key(node);
    if (key == node_key) {
	if (!_node_has_value(node))
	    return NULL;
	return _node_value_ptr(node);
    }
    if (_key_coverseq(node_key, key)) {
	if (key < node_key)
	    node = _node_left(node);
	else
	    node = _node_right(node);
	goto tailcall;
    }
    else
	return NULL;
}

int
cucon_ucmap_find_int(cucon_ucmap_t node, uintptr_t key)
{
    uintptr_t node_key;
tailcall:
    if (!node)
	return cucon_ucmap_int_none;
    node_key = _node_key(node);
    if (key == node_key) {
	if (!_node_has_value(node))
	    return cucon_ucmap_int_none;
	return _node_value(node);
    }
    if (_key_coverseq(node_key, key)) {
	if (key < node_key)
	    node = _node_left(node);
	else
	    node = _node_right(node);
	goto tailcall;
    }
    else
	return cucon_ucmap_int_none;
}

size_t
cucon_ucmap_card(cucon_ucmap_t node)
{
    size_t count = 0;
    while (node) {
	count += cucon_ucmap_card(_node_left(node));
	if (_node_has_value(node))
	    ++count;
	node = _node_right(node);
    }
    return count;
}

cu_bool_t
cucon_ucmap_iterA(cu_clop(f, cu_bool_t, uintptr_t k, uintptr_t v),
		  cucon_ucmap_t M)
{
    while (M) {
	if (!cucon_ucmap_iterA(f, _node_left(M))) return cu_false;
	if (_node_has_value(M)) {
	    if (!cu_call(f, _node_key(M), _node_value(M)))
		return cu_false;
	}
	M = _node_right(M);
    }
    return cu_true;
}

size_t
cucon_ucmap_clipped_card(cucon_ucmap_t M, uintptr_t k_min, uintptr_t k_max)
{
    uintptr_t k_M;
    if (k_min <= k_max) {
	size_t count = 0;
	while (M) {
	    k_M = _node_key(M);
	    if (k_M < k_min)
		M = _node_right(M);
	    else if (k_M > k_max)
		M = _node_left(M);
	    else {
		if (_node_has_value(M)) ++count;
		count += cucon_ucmap_clipped_card(_node_left(M), k_min, k_max);
		M = _node_right(M);
	    }
	}
	return count;
    }
    else
	return cucon_ucmap_clipped_card(M, k_min + 1, UINTPTR_MAX)
	     + cucon_ucmap_clipped_card(M, 0, k_max - 1);
}

static cu_bool_t
_iterA_above(cu_clop(f, cu_bool_t, uintptr_t, uintptr_t),
	     cucon_ucmap_t M, uintptr_t k_min)
{
    uintptr_t k_M;

tail_call:
    if (!M) return cu_true;

    k_M = _node_key(M);
    if (k_M < k_min) {
	M = _node_right(M);
	goto tail_call;
    }

    return _iterA_above(f, _node_left(M), k_min)
	&& (!_node_has_value(M) || cu_call(f, _node_key(M), _node_value(M)))
	&& cucon_ucmap_iterA(f, _node_right(M));
}

static cu_bool_t
_iterA_below(cu_clop(f, cu_bool_t, uintptr_t, uintptr_t),
	     cucon_ucmap_t M, uintptr_t k_max)
{
    uintptr_t k_M;

tail_call:
    if (!M) return cu_true;

    k_M = _node_key(M);
    if (k_M > k_max) {
	M = _node_left(M);
	goto tail_call;
    }

    return cucon_ucmap_iterA(f, _node_left(M))
	&& (!_node_has_value(M) || cu_call(f, _node_key(M), _node_value(M)))
	&& _iterA_below(f, _node_right(M), k_max);
}

cu_bool_t
cucon_ucmap_iterA_clipped(cu_clop(f, cu_bool_t, uintptr_t k, uintptr_t v),
			  cucon_ucmap_t M, uintptr_t k_min, uintptr_t k_max)
{
    uintptr_t k_M;
    if (k_min <= k_max) {
	while (M) {
	    k_M = _node_key(M);
	    if (k_M < k_min)
		M = _node_right(M);
	    else if (k_M > k_max)
		M = _node_left(M);
	    else
		return _iterA_above(f, _node_left(M), k_min)
		    && (!_node_has_value(M)
			|| cu_call(f, _node_key(M), _node_value(M)))
		    && _iterA_below(f, _node_right(M), k_max);
	}
	return cu_true;
    }
    else
	return _iterA_below(f, M, k_max)
	    && _iterA_above(f, M, k_min);
}

void
cucon_ucmap_iter_ptr(cucon_ucmap_t node,
		     cu_clop(f, void, uintptr_t key, void *val))
{
    while (node) {
	uintptr_t key = _node_key(node);
	cucon_ucmap_iter_ptr(_node_left(node), f);
	if (_node_has_value(node))
	    cu_call(f, key, _node_value_ptr(node));
	node = _node_right(node);
    }
}

void
cucon_ucmap_iter_int(cucon_ucmap_t node,
		     cu_clop(f, void, uintptr_t key, int val))
{
    while (node) {
	uintptr_t key = _node_key(node);
	cucon_ucmap_iter_int(_node_left(node), f);
	if (_node_has_value(node))
	    cu_call(f, key, _node_value(node));
	node = _node_right(node);
    }
}

cu_bool_t
cucon_ucmap_conj_ptr(cucon_ucmap_t node,
		     cu_clop(cb, cu_bool_t, uintptr_t key, void *val))
{
    while (node) {
	uintptr_t key = _node_key(node);
	if (!cucon_ucmap_conj_ptr(_node_left(node), cb))
	    return cu_false;
	if (_node_has_value(node) && !cu_call(cb, key, _node_value_ptr(node)))
	    return cu_false;
	node = _node_right(node);
    }
    return cu_true;
}

cu_bool_t
cucon_ucmap_conj_int(cucon_ucmap_t node,
		     cu_clop(cb, cu_bool_t, uintptr_t key, int val))
{
    while (node) {
	uintptr_t key = _node_key(node);
	if (!cucon_ucmap_conj_int(_node_left(node), cb))
	    return cu_false;
	if (_node_has_value(node) && !cu_call(cb, key, _node_value(node)))
	    return cu_false;
	node = _node_right(node);
    }
    return cu_true;
}

#if CUCONF_SIZEOF_INTPTR_T != CUCONF_SIZEOF_VOID_P

cu_bool_t
cucon_pcmap_conj_ptr(cucon_pcmap_t node,
		     cu_clop(cb, cu_bool_t, void *key, void *val))
{
    while (node) {
#define node ((cucon_ucmap_t)node)
	void *key = (void *)_node_key(node);
	if (!cucon_pcmap_conj_ptr(_node_left(node), cb))
	    return cu_false;
	if (_node_has_value(node))
	    if (!cu_call(cb, (void *)key, _node_value_ptr(node)))
		return cu_false;
#undef node
	node = _node_right(node);
    }
    return cu_true;
}

cu_bool_t
cucon_pcmap_conj_int(cucon_pcmap_t node,
		     cu_clop(cb, cu_bool_t, void *key, int val))
{
    while (node) {
#define node ((cucon_ucmap_t)node)
	void *key = (void *)_node_key(node);
	if (!cucon_pcmap_conj_int(_node_left(node), cb))
	    return cu_false;
	if (_node_has_value(node))
	    if (!cu_call(cb, (void *)key, _node_value(node)))
		return cu_false;
#undef node
	node = _node_right(node);
    }
    return cu_true;
}

#endif

cucon_ucmap_t
cucon_ucmap_left_union(cucon_ucmap_t M0, cucon_ucmap_t M1)
{
    uintptr_t k0, k1;
    cucon_ucmap_t M_left, M_right;

    if (!M0) return M1;
    if (!M1) return M0;

    k0 = _node_key(M0);
    k1 = _node_key(M1);
    if (k0 == k1) {
	M_left = cucon_ucmap_left_union(_node_left(M0), _node_left(M1));
	M_right = cucon_ucmap_left_union(_node_right(M0), _node_right(M1));
	return _node_new_lr(_node_has_value(M0)? M0 : M1, M_left, M_right);
    }
    else if (_key_covers(k0, k1)) {
	if (k1 < k0) {
	    M_left = cucon_ucmap_left_union(_node_left(M0), M1);
	    return _node_new_lr(M0, M_left, _node_right(M0));
	}
	else {
	    M_right = cucon_ucmap_left_union(_node_right(M0), M1);
	    return _node_new_lr(M0, _node_left(M0), M_right);
	}
    }
    else if (_key_covers(k1, k0)) {
	if (k0 < k1) {
	    M_left = cucon_ucmap_left_union(M0, _node_left(M1));
	    return _node_new_lr(M1, M_left, _node_right(M1));
	}
	else {
	    M_right = cucon_ucmap_left_union(M0, _node_right(M1));
	    return _node_new_lr(M1, _node_left(M1), M_right);
	}
    }
    else {
	uintptr_t k = _key_join(k0, k1);
	if (k0 < k1)
	    return _node_new_noval(k, M0, M1);
	else
	    return _node_new_noval(k, M1, M0);
    }
}

uintptr_t
cucon_ucmap_min_ukey(cucon_ucmap_t map)
{
    uintptr_t min = UINTPTR_MAX;
    while (map) {
	min = map->key;
	map = _node_left(map);
    }
    return min;
}

uintptr_t
cucon_ucmap_max_ukey(cucon_ucmap_t map)
{
    uintptr_t max = 0;
    while (map) {
	max = map->key;
	map = _node_right(map);
    }
    return max;
}

static cucon_ucmap_t
_uclip_upper(cucon_ucmap_t M, uintptr_t k_min)
{
    uintptr_t k_M;
    cucon_ucmap_t M_left;

tail_call:
    if (M == NULL)
	return NULL;

    k_M = _node_key(M);
    if (k_M < k_min) {
	M = _node_right(M);
	goto tail_call;
    }

    M_left = _uclip_upper(_node_left(M), k_min);
    return _node_new_lr(M, M_left, _node_right(M));
}

static cucon_ucmap_t
_uclip_lower(cucon_ucmap_t M, uintptr_t k_max)
{
    uintptr_t k_M;
    cucon_ucmap_t M_right;

tail_call:
    if (M == NULL)
	return NULL;

    k_M = _node_key(M);
    if (k_M > k_max) {
	M = _node_left(M);
	goto tail_call;
    }

    M_right = _uclip_lower(_node_right(M), k_max);
    return _node_new_lr(M, _node_left(M), M_right);
}

cucon_ucmap_t
cucon_ucmap_uclip(cucon_ucmap_t M, uintptr_t k_min, uintptr_t k_max)
{
    uintptr_t k_M;
    cucon_ucmap_t M_left, M_right;

tail_call:
    if (M == NULL)
	return NULL;

    k_M = _node_key(M);
    if (k_M < k_min) {
	M = _node_right(M);
	goto tail_call;
    }
    if (k_M > k_max) {
	M = _node_left(M);
	goto tail_call;
    }

    M_left = _uclip_upper(_node_left(M), k_min);
    M_right = _uclip_lower(_node_right(M), k_max);
    return _node_new_lr(M, M_left, M_right);
}

cucon_ucmap_t
cucon_ucmap_clip(cucon_ucmap_t M, uintptr_t k_min, uintptr_t k_max)
{
    if (k_min <= k_max)
	return cucon_ucmap_uclip(M, k_min, k_max);
    else {
	cucon_ucmap_t M0 = _uclip_lower(M, k_max);
	cucon_ucmap_t M1 = _uclip_upper(M, k_min);
	return cucon_ucmap_left_union(M0, M1);
    }
}

cu_bool_t
cucon_ucmap_eq(cucon_ucmap_t map0, cucon_ucmap_t map1)
{
tail_call:
#if CUCONP_UCMAP_ENABLE_HCONS
    if (map0 == map1) return cu_true;
    if (map0 == NULL) return cu_false;
#else
    if (map0 == NULL) return map1 == NULL;
#endif
    if (map1 == NULL) return cu_false;
    if (_node_key(map0) != _node_key(map1)) return cu_false;
    if (_node_has_value(map0) != _node_has_value(map1)) return cu_false;
    if (_node_value(map0) != _node_value(map1)) return cu_false;

    if (!cucon_ucmap_eq(_node_left(map0), _node_left(map1)))
	return cu_false;

    map0 = _node_right(map0);
    map1 = _node_right(map1);
    goto tail_call;
}

cu_bool_t
cucon_ucmap_eq_ptr(cu_clop(f, cu_bool_t, void const *, void const *),
		   cucon_ucmap_t map0, cucon_ucmap_t map1)
{
tail_call:
#if CUCONP_UCMAP_ENABLE_HCONS
    if (map0 == map1) return cu_true;
    if (map0 == NULL) return cu_false;
#else
    if (map0 == NULL) return map1 == NULL;
#endif
    if (map1 == NULL) return cu_false;
    if (_node_key(map0) != _node_key(map1)) return cu_false;

    if (!cucon_ucmap_eq_ptr(f, _node_left(map0), _node_left(map1)))
	return cu_false;

    if (_node_has_value(map0)) {
	if (_node_has_value(map1)) {
	    if (!cu_call(f, _node_value_ptr(map0), _node_value_ptr(map1)))
		return cu_false;
	}
	else
	    return cu_false;
    }
    else if (_node_has_value(map1))
	return cu_false;

    map0 = _node_right(map0);
    map1 = _node_right(map1);
    goto tail_call;
}

int
cucon_ucmap_cmp(cucon_ucmap_t map0, cucon_ucmap_t map1)
{
    int c;

tail_call:
#if CUCONP_UCMAP_ENABLE_HCONS
    if (map0 == map1) return  0;
    if (map0 == NULL) return -1;
#else
    if (map0 == NULL) return map1 == NULL? 0 : -1;
#endif
    if (map1 == NULL) return  1;
    if (_node_key(map0) < _node_key(map1)) return -1;
    if (_node_key(map0) > _node_key(map1)) return  1;

    c = cucon_ucmap_cmp(_node_left(map0), _node_left(map1));
    if (c) return c;

    if (_node_has_value(map0)) {
	if (_node_has_value(map1)) {
	    if (_node_value(map0) < _node_value(map1)) return -1;
	    if (_node_value(map0) > _node_value(map1)) return  1;
	}
	else
	    return 1;
    }
    else if (_node_has_value(map1))
	return -1;

    map0 = _node_right(map0);
    map1 = _node_right(map1);
    goto tail_call;
}

int
cucon_ucmap_cmp_ptr(cu_clop(f, int, void const *, void const *),
		    cucon_ucmap_t map0, cucon_ucmap_t map1)
{
    int c;

tail_call:
#if CUCONP_UCMAP_ENABLE_HCONS
    if (map0 == map1) return  0;
    if (map0 == NULL) return -1;
#else
    if (map0 == NULL) return map1 == NULL? 0 : -1;
#endif
    if (map1 == NULL) return  1;
    if (_node_key(map0) < _node_key(map1)) return -1;
    if (_node_key(map0) > _node_key(map1)) return  1;

    c = cucon_ucmap_cmp_ptr(f, _node_left(map0), _node_left(map1));
    if (c) return c;

    if (_node_has_value(map0)) {
	if (_node_has_value(map1)) {
	    c = cu_call(f, _node_value_ptr(map0), _node_value_ptr(map1));
	    if (c) return c;
	}
	else
	    return 1;
    }
    else if (_node_has_value(map1))
	return -1;

    map0 = _node_right(map0);
    map1 = _node_right(map1);
    goto tail_call;
}

static void
_ucmap_dump(cucon_ucmap_t tree, int ind, FILE *out)
{
    int i = ind++;
    if (tree) {
	_ucmap_dump(_node_left(tree), ind, out);
	while (i--)
	    fputs("  ", out);
	if (_node_has_value(tree))
	    fprintf(out, "+ 0x%lx ↦ %p\n",
		    (long)_node_key(tree), _node_value_ptr(tree));
	else
	    fprintf(out, "- 0x%lx\n", (long)_node_key(tree));
	_ucmap_dump(_node_right(tree), ind, out);
    }
}

void
cucon_ucmap_dump(cucon_ucmap_t tree, FILE *out)
{
    fprintf(out, "cucon_ucmap_t @ %p\n", (void *)tree);
    _ucmap_dump(tree, 2, out);
}

static void
_ucmap_print(cuex_t e, FILE *out)
{ cucon_ucmap_dump(e, out); }

static cu_box_t
_ucmap_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN:
	    return cu_box_fptr(cuoo_intf_print_fn_t, _ucmap_print);
	default:
	    return CUOO_IMPL_NONE;
    }
}

#if CUCONP_UCMAP_ENABLE_HCONS
cuoo_type_t cuconP_ucmap_type;
#endif

void
cuconP_ucmap_init()
{
#if CUCONP_UCMAP_ENABLE_HCONS
    cuconP_ucmap_type = cuoo_type_new_opaque_hcs(
	_ucmap_impl, sizeof(struct cucon_ucmap) - CUOO_HCOBJ_SHIFT);
#endif
}

