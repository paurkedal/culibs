/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_PMAP_H
#define CUCON_PMAP_H

#include <stdlib.h>
#include <cucon/fwd.h>
#include <cucon/umap.h>
#include <cu/memory.h>
#include <cu/util.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_pmap_h cucon/pmap.h: Pointer-Keyed Hash Map
 * @{\ingroup cucon_maps_and_sets_mod
 *
 * This header implements hash maps keyed on pointers, and with arbitrary
 * value slots.  The slots are arbitrary sized memory which is stored on the
 * nodes for best efficiency.  The slots are accessed directly using the \c
 * *_mem functions, whereas some specialised functions are provided for
 * pointers and integers.
 *
 * \see cucon_pset_h
 * \see cucon_umap_h */

/*!A pointer-keyed map with variable-sized inline value slots. */
struct cucon_pmap_s
{
    struct cucon_umap_s impl;
};

struct cucon_pmap_node_s
{
    struct cucon_umap_node_s impl;
};

typedef struct cucon_pmap_node_s *cucon_pmap_node_t;
typedef cucon_umap_it_t cucon_pmap_it_t;

CU_SINLINE void *cucon_pmap_node_key(cucon_pmap_node_t node)
{ return (void *)cucon_umap_node_key(&node->impl); }

/*!Construct \a map as an empty property map. */
CU_SINLINE void cucon_pmap_init(cucon_pmap_t map)
{ cucon_umap_init(&map->impl); }

/*!Return an empty property map. */
CU_SINLINE cucon_pmap_t cucon_pmap_new(void)
{ return (cucon_pmap_t)cucon_umap_new(); }

/*!Construct \a dst as a copy of \a src but dropping all value slots. */
CU_SINLINE void cucon_pmap_init_copy_void(cucon_pmap_t dst, cucon_pmap_t src)
{ return cucon_umap_init_copy_void(&dst->impl, &src->impl); }

/*!Return a copy of \a src after dropping all value slots. */
CU_SINLINE cucon_pmap_t cucon_pmap_new_copy_void(cucon_pmap_t src)
{ return (cucon_pmap_t)cucon_umap_new_copy_void(&src->impl); }

/*!Construct \a dst as a copy of \a src assuming slots are \a slot_size bytes
 * which can be copied with memcpy. */
CU_SINLINE void cucon_pmap_init_copy_mem(cucon_pmap_t dst, cucon_pmap_t src,
					 size_t slot_size)
{ cucon_umap_init_copy_mem(&dst->impl, &src->impl, slot_size); }

/*!Return a copy of \a src assuming slots are \a slot_size bytes which can be
 * copied with memcpy. */
CU_SINLINE cucon_pmap_t cucon_pmap_new_copy_mem(cucon_pmap_t src,
						size_t slot_size)
{ return (cucon_pmap_t)cucon_umap_new_copy_mem(&src->impl, slot_size); }

/*!\copydoc cucon_umap_init_copy_mem_ctor */
CU_SINLINE void cucon_pmap_init_copy_mem_ctor(
    cucon_pmap_t dst, cucon_pmap_t src, size_t slot_size,
    cu_clop(value_init_copy, void, void *dst_slot, void *src_slot,
	    uintptr_t key))
{ cucon_umap_init_copy_mem_ctor(&dst->impl, &src->impl, slot_size,
				value_init_copy); }
/* TODO. Fix key argument. */

/*!\copydoc cucon_umap_init_copy_node */
CU_SINLINE void cucon_pmap_init_copy_node(
    cucon_pmap_t dst, cucon_pmap_t src,
    cu_clop(node_alloc_copy, cucon_pmap_node_t, void *, uintptr_t))
{
    cucon_umap_init_copy_node(&dst->impl, &src->impl,
	(cu_clop(, cucon_umap_node_t, void *, uintptr_t))node_alloc_copy);
}
/* TODO. Fix key argument. */

CU_SINLINE cucon_pmap_node_t cucon_pmap_node_alloc(size_t slot_size)
{ return cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_pmap_node_s) + slot_size); }

CU_SINLINE void *cucon_pmap_node_get_mem(cucon_pmap_node_t node)
{ return node + 1; }

/*!\copydoc cucon_umap_swap */
CU_SINLINE void cucon_pmap_swap(cucon_pmap_t map0, cucon_pmap_t map1)
{ cucon_umap_swap(&map0->impl, &map1->impl); }

/*!\copydoc cucon_umap_insert_init_node */
CU_SINLINE cu_bool_t
cucon_pmap_insert_init_node(cucon_pmap_t map, cucon_pmap_node_t node)
{ return cucon_umap_insert_init_node(&map->impl, &node->impl); }

/*!If \a key is not in \a map, this call allocates a node of size \a node_size,
 * initialises a \c cucon_pmap_node_s from offset 0 with the key \a key,
 * inserts the node into \a map, assigns it to \c *\a node_out, and returns
 * true.  Otherwise, returns false. */
CU_SINLINE cu_bool_t
cucon_pmap_insert_new_node(cucon_pmap_t map, void const *key,
			   size_t node_size, cu_ptr_ptr_t node_out)
{ return cucon_umap_insert_new_node(&map->impl, (uintptr_t)key,
				    node_size, node_out); }

/*!\copydoc cucon_umap_insert_mem */
CU_SINLINE cu_bool_t
cucon_pmap_insert_mem(cucon_pmap_t map, void const *key,
		      size_t slot_size, cu_ptr_ptr_t slot)
{ return cucon_umap_insert_mem(&map->impl, (uintptr_t)key, slot_size, slot); }

/*!\copydoc cucon_umap_insert_void */
CU_SINLINE cu_bool_t
cucon_pmap_insert_void(cucon_pmap_t map, void const *key)
{ return cucon_umap_insert_void(&map->impl, (uintptr_t)key); }

/*!\copydoc cucon_umap_insert_ptr */
CU_SINLINE cu_bool_t
cucon_pmap_insert_ptr(cucon_pmap_t map, void const *key, void *ptr)
{ return cucon_umap_insert_ptr(&map->impl, (uintptr_t)key, ptr); }

/*!\copydoc cucon_umap_insert_int */
CU_SINLINE cu_bool_t
cucon_pmap_insert_int(cucon_pmap_t map, void const *key, int val)
{ return cucon_umap_insert_int(&map->impl, (uintptr_t)key, val); }

/*!\copydoc cucon_umap_insert_general */
CU_SINLINE cu_bool_t
cucon_pmap_insert_general(cucon_pmap_t map, void const *key,
			  cu_clop0(alloc_node, cucon_pmap_node_t),
			  cucon_pmap_node_t *node_out)
{ return cucon_umap_insert_general(&map->impl, (uintptr_t)key,
		(cu_clop0(, cucon_umap_node_t))alloc_node,
		(cucon_umap_node_t *)node_out); }

/*!\copydoc cucon_umap_replace_ptr */
CU_SINLINE void *
cucon_pmap_replace_ptr(cucon_pmap_t map, void const *key, void *ptr)
{ return cucon_umap_replace_ptr(&map->impl, (uintptr_t)key, ptr); }

/*!\copydoc cucon_umap_replace_int */
CU_SINLINE int
cucon_pmap_replace_int(cucon_pmap_t map, void const *key, int val)
{ return cucon_umap_replace_int(&map->impl, (uintptr_t)key, val); }

/*!\copydoc cucon_umap_erase */
CU_SINLINE cu_bool_t cucon_pmap_erase(cucon_pmap_t map, void const *key)
{ return cucon_umap_erase(&map->impl, (uintptr_t)key); }

/*!\copydoc cucon_umap_erase_ptr */
CU_SINLINE void *cucon_pmap_erase_ptr(cucon_pmap_t map, void const *key)
{ return cucon_umap_erase_ptr(&map->impl, (uintptr_t)key); }

/*!\copydoc cucon_umap_erase_int */
CU_SINLINE int cucon_pmap_erase_int(cucon_pmap_t map, void const *key)
{ return cucon_umap_erase_int(&map->impl, (uintptr_t)key); }

/*!\copydoc cucon_umap_erase */
CU_SINLINE cu_bool_t
cucon_pmap_erase_keep_cap(cucon_pmap_t map, void const *key)
{ return cucon_umap_erase_keep_cap(&map->impl, (uintptr_t)key); }

/*!\copydoc cucon_umap_pop_any_node */
CU_SINLINE cucon_pmap_node_t
cucon_pmap_pop_any_node(cucon_pmap_t map)
{ return (cucon_pmap_node_t)cucon_umap_pop_any_node(&map->impl); }

/*!\copydoc cucon_umap_pop_any_key */
CU_SINLINE void const *
cucon_pmap_pop_any_key(cucon_pmap_t map)
{ return (void const *)cucon_umap_pop_any_key(&map->impl); }

/*!\copydoc cucon_umap_update_cap */
CU_SINLINE void cucon_pmap_update_cap(cucon_pmap_t map)
{ cucon_umap_update_cap(&map->impl); }

/*!\copydoc cucon_umap_find_node */
CU_SINLINE cucon_pmap_node_t
cucon_pmap_find_node(cucon_pmap_t map, void const *key)
{ return (cucon_pmap_node_t)cucon_umap_find_node(&map->impl, (uintptr_t)key); }

/*!\copydoc cucon_umap_find_mem */
CU_SINLINE void *cucon_pmap_find_mem(cucon_pmap_t map, void const *key)
{ return cucon_umap_find_mem(&map->impl, (uintptr_t)key); }

/*!\copydoc cucon_umap_find_ptr */
CU_SINLINE void *cucon_pmap_find_ptr(cucon_pmap_t map, void const *key)
{ return cucon_umap_find_ptr(&map->impl, (uintptr_t)key); }

/*!\copydoc cucon_umap_find_int */
CU_SINLINE int cucon_pmap_find_int(cucon_pmap_t map, void const *key)
{ return cucon_umap_find_int(&map->impl, (uintptr_t)key); }

/*!True iff \a map contains \a key, ignoring the slot. */
CU_SINLINE cu_bool_t cucon_pmap_find_void(cucon_pmap_t map, void const *key)
{ return cucon_pmap_find_node(map, key) != NULL; }

/*!Call \a cb on each key-value pair in \a map.  */
void cucon_pmap_iter_mem(cucon_pmap_t map,
			 cu_clop(cb, void, void const *key, void *value));
/*!Call \a cb on each key-value pair in \a map. */
void cucon_pmap_iter_ptr(cucon_pmap_t map,
			 cu_clop(cb, void, void const *key, void *value));

cu_bool_t cucon_pmap_conj_general(cucon_pmap_t map, size_t node_offset,
				  cu_clop(cb, cu_bool_t, void const*, void*));

/*!\copydoc cucon_umap_conj_mem */
cu_bool_t cucon_pmap_conj_mem(cucon_pmap_t map,
			      cu_clop(cb, cu_bool_t, void const *, void *));
cu_bool_t cucon_pmap_conj_ptr(cucon_pmap_t map,
			      cu_clop(cb, cu_bool_t, void const *, void *));

/*!Call \a cb on each key in \a map. */
void cucon_pmap_iter_keys(cucon_pmap_t map, cu_clop(cb, void, void const *));

/*!Perform a sequential conjunction of \a cb over keys in \a map. */
cu_bool_t cucon_pmap_conj_keys(cucon_pmap_t map,
			       cu_clop(cb, cu_bool_t, void const *));

/*!\copydoc cucon_umap_conj_node */
CU_SINLINE cu_bool_t
cucon_pmap_conj_node(cucon_pmap_t map,
		     cu_clop(cb, cu_bool_t, cucon_pmap_node_t node))
{
    return cucon_umap_conj_node(&map->impl,
				(cu_clop(, cu_bool_t, cucon_umap_node_t))cb);
}

/*!\copydoc cucon_umap_assign_isecn_union */
CU_SINLINE void
cucon_pmap_assign_isecn_union(cucon_pmap_t map0, cucon_pmap_t map1)
{ cucon_umap_assign_isecn_union(&map0->impl, &map1->impl); }

/*!\copydoc cucon_umap_move_isecn */
CU_SINLINE void
cucon_pmap_move_isecn(cucon_pmap_t dst, cucon_pmap_t src0, cucon_pmap_t src1)
{ return cucon_umap_move_isecn(&dst->impl, &src0->impl, &src1->impl); }

/*!\copydoc cucon_umap_assign_isecn */
CU_SINLINE void
cucon_pmap_assign_isecn(cucon_pmap_t dst, cucon_pmap_t src)
{ cucon_umap_assign_isecn(&dst->impl, &src->impl); }

/*!\copydoc cucon_umap_assign_union_void */
CU_SINLINE void
cucon_pmap_assign_union_void(cucon_pmap_t dst, cucon_pmap_t src)
{ cucon_umap_assign_union_void(&dst->impl, &src->impl); }

/*!\copydoc cucon_umap_dump_stats */
CU_SINLINE void
cucon_pmap_dump_stats(cucon_pmap_t map, FILE *out)
{ cucon_umap_dump_stats(&map->impl, out); }

/*!\copydoc cucon_umap_size */
CU_SINLINE size_t
cucon_pmap_size(cucon_pmap_t map)
{ return map->impl.size; }

/*!\copydoc cucon_umap_is_empty */
CU_SINLINE cu_bool_t
cucon_pmap_is_empty(cucon_pmap_t map)
{return !map->impl.size;}

/*!\copydoc cucon_umap_eq_keys */
CU_SINLINE cu_bool_t
cucon_pmap_eq_keys(cucon_pmap_t map0, cucon_pmap_t map1)
{ return cucon_umap_eq_keys(&map0->impl, &map1->impl); }

/*!\copydoc cucon_umap_eq_mem */
CU_SINLINE cu_bool_t
cucon_pmap_eq_mem(cucon_pmap_t map0, cucon_pmap_t map1, size_t slot_size)
{ return cucon_umap_eq_mem(&map0->impl, &map1->impl, slot_size); }

/*!\copydoc cucon_umap_eq_ptr */
CU_SINLINE cu_bool_t cucon_pmap_eq_ptr(cucon_pmap_t map0, cucon_pmap_t map1)
{ return cucon_umap_eq_ptr(&map0->impl, &map1->impl); }

/*!\copydoc cucon_umap_hash_keys */
CU_SINLINE cu_hash_t cucon_pmap_hash_keys(cucon_pmap_t map)
{ return cucon_umap_hash_keys(&map->impl); }

/*!\copydoc cucon_umap_hash_mem */
CU_SINLINE cu_hash_t cucon_pmap_hash_mem(cucon_pmap_t map, size_t slot_size)
{ return cucon_umap_hash_mem(&map->impl, slot_size); }

/*!\copydoc cucon_umap_hash_ptr */
CU_SINLINE cu_hash_t cucon_pmap_hash_ptr(cucon_pmap_t map)
{ return cucon_umap_hash_ptr(&map->impl); }

/* A range over entries in 'tmap'. 'cucon_pmap_end_eq' is faster than
 * comparing with the result of 'cucon_pmap_end', even if the result of
 * 'cucon_pmap_end' is stored. */
CU_SINLINE cucon_pmap_it_t cucon_pmap_begin(cucon_pmap_t map)
{ return cucon_umap_begin(&map->impl); }

CU_SINLINE cucon_pmap_it_t cucon_pmap_end(cucon_pmap_t map)
{ return cucon_umap_end(&map->impl); }

CU_SINLINE cu_bool_t
cucon_pmap_end_eq(cucon_pmap_t map, cucon_pmap_it_t it)
{
    return it.node == (void *)-1;
}

#define cucon_pmap_it_eq(it0, it1) ((it0.node) == (it1.node))

CU_SINLINE cucon_pmap_it_t cucon_pmap_it_next(cucon_pmap_it_t it)
{ return cucon_umap_it_next(it); }

/* Return the key part at 'it'.  */
#define cucon_pmap_it_key(it) ((void*)(it).node->key)

/* Return the value part at 'it'. */
#define cucon_pmap_it_value_mem(it) ((void*)CU_ALIGNED_PTR_END((it).node))
#define cucon_pmap_it_value_ptr(it) (*(void**)CU_ALIGNED_PTR_END((it).node))

/*!Dump \a map to \a out assuming keys are \c cu_idr_t and slots are
 * pointers. */
void cucon_pmap_dump_idr_ptr(cucon_pmap_t map, FILE *out);

/*!\deprecated Use \ref cucon_pmap_init. */
#define cucon_pmap_cct			cucon_pmap_init
/*!\deprecated Use \ref cucon_pmap_init_copy_void. */
#define cucon_pmap_cct_copy_void	cucon_pmap_init_copy_void
/*!\deprecated Use \ref cucon_pmap_init_copy_mem. */
#define cucon_pmap_cct_copy_mem		cucon_pmap_init_copy_mem
/*!\deprecated Use \ref cucon_pmap_init_copy_mem_ctor. */
#define cucon_pmap_cct_copy_mem_ctor	cucon_pmap_init_copy_mem_ctor
/*!\deprecated Use \ref cucon_pmap_init_copy_node. */
#define cucon_pmap_cct_copy_node	cucon_pmap_init_copy_node

/*!@}*/
CU_END_DECLARATIONS

#endif
