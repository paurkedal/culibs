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

#include <cucon/rbmap.h>
#include <cu/memory.h>
#include <cu/ptr.h>

void
cucon_rbmap_init(cucon_rbmap_t map, cu_clop(cmp, int, void *, void *))
{
    cucon_rbtree_init(cu_to(cucon_rbtree, map));
    map->cmp = cmp;
}

cucon_rbmap_t
cucon_rbmap_new(cu_clop(cmp, int, void *, void *))
{
    cucon_rbmap_t map = cu_gnew(struct cucon_rbmap_s);
    cucon_rbtree_init(cu_to(cucon_rbtree, map));
    map->cmp = cmp;
    return map;
}

void *
cucon_rbmap_find_mem(cucon_rbmap_t map, void *key)
{
    cucon_rbnode_t node;
    node = cucon_rbtree_find2p_node(cu_to(cucon_rbtree, map), map->cmp, key);
    if (node)
	return cu_ptr_add(cucon_rbnode_mem(node), sizeof(void *));
    else
	return NULL;
}

void *
cucon_rbmap_find_ptr(cucon_rbmap_t map, void *key)
{
    cucon_rbnode_t node;
    node = cucon_rbtree_find2p_node(cu_to(cucon_rbtree, map), map->cmp, key);
    if (node)
	return *(void **)cu_ptr_add(cucon_rbnode_mem(node), sizeof(void *));
    else
	return NULL;
}

cu_bool_t
cucon_rbmap_insert_mem(cucon_rbmap_t map, void *key,
		       size_t slot_size, cu_ptr_ptr_t slot)
{
    cucon_rbnode_t node;
    cu_bool_t st;
    st = cucon_rbtree_insert2p_node(
	    cu_to(cucon_rbtree, map), map->cmp, key,
	    sizeof(struct cucon_rbnode_s) + sizeof(void *) + slot_size, 0,
	    &node);
    *(void **)slot = cu_ptr_add(cucon_rbnode_mem(node), sizeof(void *));
    return st;
}

cu_bool_t
cucon_rbmap_insert_ptr(cucon_rbmap_t map, void *key, void *val)
{
    cucon_rbnode_t node;
    if (cucon_rbtree_insert2p_node(
		cu_to(cucon_rbtree, map), map->cmp, key,
		sizeof(struct cucon_rbnode_s) + 2*sizeof(void *), 0, &node)) {
	*(void **)cu_ptr_add(cucon_rbnode_mem(node), sizeof(void *)) = val;
	return cu_true;
    }
    else
	return cu_false;
}

cu_bool_t
cucon_rbmap_erase(cucon_rbmap_t map, void *key)
{
    cucon_rbnode_t node;
    node = cucon_rbtree_erase2p(cu_to(cucon_rbtree, map), map->cmp, key);
    return !!node;
}

static cu_bool_t
_rbmap_conj_mem(cucon_rbnode_t node,
		cu_clop(cb, cu_bool_t, void *key, void *val))
{
    while (node) {
	void **slot;
	if (!_rbmap_conj_mem(cucon_rbnode_left(node), cb))
	    return cu_false;
	slot = cucon_rbnode_mem(node);
	if (!cu_call(cb, slot[0], slot + 1))
	    return cu_false;
	node = cucon_rbnode_right(node);
    }
    return cu_true;
}

cu_bool_t
cucon_rbmap_conj_mem(cucon_rbmap_t map,
		     cu_clop(cb, cu_bool_t, void *key, void *val))
{
    cucon_rbnode_t x = cucon_rbtree_root(cu_to(cucon_rbtree, map));
    return _rbmap_conj_mem(x, cb);
}

static cu_bool_t
_rbmap_conj_ptr(cucon_rbnode_t node,
		cu_clop(cb, cu_bool_t, void *key, void *val))
{
    while (node) {
	void **slot;
	if (!_rbmap_conj_ptr(cucon_rbnode_left(node), cb))
	    return cu_false;
	slot = cucon_rbnode_mem(node);
	if (!cu_call(cb, slot[0], slot[1]))
	    return cu_false;
	node = cucon_rbnode_right(node);
    }
    return cu_true;
}

cu_bool_t
cucon_rbmap_conj_ptr(cucon_rbmap_t map,
		     cu_clop(cb, cu_bool_t, void *key, void *val))
{
    cucon_rbnode_t x = cucon_rbtree_root(cu_to(cucon_rbtree, map));
    return _rbmap_conj_ptr(x, cb);
}

