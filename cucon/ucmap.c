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

#include <cucon/ucmap.h>
#include <cu/memory.h>
#include <cu/int.h>
#include <cuoo/halloc.h>
#include <cuoo/intf.h>
#include <inttypes.h>

#if cuconP_UCMAP_EXIST_IN_LEFT
#  define node_left(t) ((cucon_ucmap_t)((uintptr_t)(t)->left & ~(uintptr_t)1))
#  define have_value(t) ((uintptr_t)(t)->left & 1)
#  define value_ptr(t) ((void *)(t)->value)
#  define value_int(t) ((t)->value)
#else
#  define node_left(t) ((t)->left)
#  define have_value(t) (!!(t)->value)
#  define value_ptr(t) ((void *)(t)->value)
#  define value_int(t) ((t)->value + cucon_ucmap_int_none)
#endif
#define node_right(t) ((t)->right)

CU_SINLINE uintptr_t ucnode_key(cucon_ucmap_t node) { return node->key; }

CU_SINLINE cu_bool_t
key_covers(uintptr_t key0, uintptr_t key1)
{
    uintptr_t mask0 = ~(key0 ^ (key0 - 1));
    return (key0 & mask0) == (key1 & mask0) && (key1 & ~mask0);
}
CU_SINLINE cu_bool_t
key_coverseq(uintptr_t key0, uintptr_t key1)
{
    uintptr_t mask0 = ~(key0 ^ (key0 - 1));
    return (key0 & mask0) == (key1 & mask0);
}

CU_SINLINE cucon_ucmap_t
ucnode_new(uintptr_t key, cucon_ucmap_t left, cucon_ucmap_t right,
	   uintptr_t val)
{
#if cuconP_UCMAP_EXIST_IN_LEFT
    left = (cucon_ucmap_t)((uintptr_t)left | 1);
#endif
#if cuconP_UCMAP_ENABLE_HCONS
    cuoo_hctem_decl(cucon_ucmap, tem);
    cuoo_hctem_init(cucon_ucmap, tem);
    cuoo_hctem_get(cucon_ucmap, tem)->key = key;
    cuoo_hctem_get(cucon_ucmap, tem)->left = left;
    cuoo_hctem_get(cucon_ucmap, tem)->right = right;
    cuoo_hctem_get(cucon_ucmap, tem)->value = (uintptr_t)val;
    return cuoo_hctem_new(cucon_ucmap, tem);
#else
    cucon_ucmap_t node = cu_gnew(struct cucon_ucmap_s);
    node->key = key;
    node->left = left;
    node->right = right;
    node->value = (uintptr_t)val;
    return node;
#endif
}

CU_SINLINE cucon_ucmap_t
ucnode_new_noval(uintptr_t key, cucon_ucmap_t left, cucon_ucmap_t right)
{
#if cuconP_UCMAP_ENABLE_HCONS
    cuoo_hctem_decl(cucon_ucmap, tem);
    cuoo_hctem_init(cucon_ucmap, tem);
    cuoo_hctem_get(cucon_ucmap, tem)->key = key;
    cuoo_hctem_get(cucon_ucmap, tem)->left = left;
    cuoo_hctem_get(cucon_ucmap, tem)->right = right;
    cuoo_hctem_get(cucon_ucmap, tem)->value = 0;
    return cuoo_hctem_new(cucon_ucmap, tem);
#else
    cucon_ucmap_t node = cu_gnew(struct cucon_ucmap_s);
    node->key = key;
    node->left = left;
    node->right = right;
    node->value = 0;
    return node;
#endif
}

cucon_ucmap_t
cuconP_ucmap_insert_raw(cucon_ucmap_t node, uintptr_t key, uintptr_t val)
{
    uintptr_t node_key;
    if (node == NULL) {
	return ucnode_new(key, NULL, NULL, val);
    }

    node_key = ucnode_key(node);
    if (node_key == key) {
	if (node->value == (uintptr_t)val)
	    return node;
	else
	    return ucnode_new(node->key, node_left(node), node_right(node),
			      val);
    }
    if (key_covers(node_key, key)) {
	if (key < node_key) {
	    cucon_ucmap_t new_left;
	    new_left = cuconP_ucmap_insert_raw(node_left(node), key, val);
	    if (new_left != node_left(node))
		return ucnode_new(node->key, new_left, node_right(node), val);
	    else
		return node;
	}
	else {
	    cucon_ucmap_t new_right;
	    new_right = cuconP_ucmap_insert_raw(node_right(node), key, val);
	    if (new_right != node_right(node))
		return ucnode_new(node->key, node_left(node), new_right, val);
	    else
		return node;
	}
    }

    if (key_covers(key, node_key)) {
	if (node_key < key)
	    return ucnode_new(key, node, NULL, val);
	else
	    return ucnode_new(key, NULL, node, val);
    }

    else {
	cucon_ucmap_t key_node;
	uintptr_t j;
	key_node = ucnode_new(key, NULL, NULL, val);
	j = cu_ulong_dcover(key ^ node_key);
	cu_debug_assert(j >= 2);
	j = (key & ~j) | ((j + 1) >> 1);
	if (key < node_key)
	    return ucnode_new_noval(j, key_node, node);
	else
	    return ucnode_new_noval(j, node, key_node);
    }
}

void *
cucon_ucmap_find(cucon_ucmap_t node, uintptr_t key)
{
    uintptr_t node_key;
tailcall:
    if (!node)
	return NULL;
    node_key = ucnode_key(node);
    if (key == node_key) {
#if cuconP_UCMAP_EXIST_IN_LEFT
	if (!have_value(node))
	    return NULL;
#endif
	return value_ptr(node);
    }
    if (key_coverseq(node_key, key)) {
	if (key < node_key)
	    node = node_left(node);
	else
	    node = node_right(node);
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
    node_key = ucnode_key(node);
    if (key == node_key) {
#if cuconP_UCMAP_EXIST_IN_LEFT
	if (!have_value(node))
	    return cucon_ucmap_int_none;
#endif
	return value_int(node);
    }
    if (key_coverseq(node_key, key)) {
	if (key < node_key)
	    node = node_left(node);
	else
	    node = node_right(node);
	goto tailcall;
    }
    else
	return cucon_ucmap_int_none;
}

cu_bool_t
cucon_ucmap_conj(cucon_ucmap_t node,
		 cu_clop(cb, cu_bool_t, uintptr_t key, void *val))
{
    while (node) {
	uintptr_t key = ucnode_key(node);
	if (!cucon_ucmap_conj(node_left(node), cb))
	    return cu_false;
	if (have_value(node) && !cu_call(cb, key, value_ptr(node)))
	    return cu_false;
	node = node_right(node);
    }
    return cu_true;
}

cu_bool_t
cucon_ucmap_conj_int(cucon_ucmap_t node,
		     cu_clop(cb, cu_bool_t, uintptr_t key, int val))
{
    while (node) {
	uintptr_t key = ucnode_key(node);
	if (!cucon_ucmap_conj_int(node_left(node), cb))
	    return cu_false;
	if (have_value(node) && !cu_call(cb, key, value_int(node)))
	    return cu_false;
	node = node_right(node);
    }
    return cu_true;
}

#if CUCONF_SIZEOF_INTPTR_T != CUCONF_SIZEOF_VOID_P

cu_bool_t
cucon_pcmap_conj(cucon_pcmap_t node,
		 cu_clop(cb, cu_bool_t, void *key, void *val))
{
    while (node) {
#define node ((cucon_ucmap_t)node)
	void *key = (void *)ucnode_key(node);
	if (!cucon_pcmap_conj(node_left(node), cb))
	    return cu_false;
	if (have_value(node) && !cu_call(cb, (void *)key, value_ptr(node)))
	    return cu_false;
#undef node
	node = node_right(node);
    }
    return cu_true;
}

cu_bool_t
cucon_pcmap_conj_int(cucon_pcmap_t node,
		     cu_clop(cb, cu_bool_t, void *key, int val))
{
    while (node) {
#define node ((cucon_ucmap_t)node)
	void *key = (void *)ucnode_key(node);
	if (!cucon_pcmap_conj_int(node_left(node), cb))
	    return cu_false;
	if (have_value(node) && !cu_call(cb, (void *)key, value_int(node)))
	    return cu_false;
#undef node
	node = node_right(node);
    }
    return cu_true;
}

#endif

uintptr_t
cucon_ucmap_min_ukey(cucon_ucmap_t map)
{
    uintptr_t min = UINTPTR_MAX;
    while (map) {
	min = map->key;
	map = node_left(map);
    }
    return min;
}

uintptr_t
cucon_ucmap_max_ukey(cucon_ucmap_t map)
{
    uintptr_t max = 0;
    while (map) {
	max = map->key;
	map = node_right(map);
    }
    return max;
}

static void
ucmap_dump(cucon_ucmap_t tree, int ind, FILE *out)
{
    int i = ind++;
    if (tree) {
	ucmap_dump(node_left(tree), ind, out);
	while (i--)
	    fputs("  ", out);
	if (have_value(tree))
	    fprintf(out, "+ 0x%lx ↦ %p\n",
		    (long)ucnode_key(tree), value_ptr(tree));
	else
	    fprintf(out, "- 0x%lx\n", (long)ucnode_key(tree));
	ucmap_dump(node_right(tree), ind, out);
    }
}

void
cucon_ucmap_dump(cucon_ucmap_t tree, FILE *out)
{
    fprintf(out, "cucon_ucmap_t @ %p\n", tree);
    ucmap_dump(tree, 2, out);
}

static cu_word_t
ucmap_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN: return (cu_word_t)cucon_ucmap_dump;
	default: return CUOO_IMPL_NONE;
    }
}

#if cuconP_UCMAP_ENABLE_HCONS
cuoo_type_t cuconP_ucmap_type;
#endif

void
cuconP_ucmap_init()
{
#if cuconP_UCMAP_ENABLE_HCONS
    cuconP_ucmap_type = cuoo_type_new_opaque_hcs(
	ucmap_impl, sizeof(struct cucon_ucmap_s) - CUOO_HCOBJ_SHIFT);
#endif
}

