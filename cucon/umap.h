/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_UMAP_H
#define CUCON_UMAP_H

#include <stdlib.h>
#include <stdio.h>
#include <cucon/fwd.h>
#include <cu/memory.h>
#include <cu/clos.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_umap_h cucon/umap.h: Integer-Keyed Hash Map
 ** @{ \ingroup cucon_maps_and_sets_mod
 **
 ** This header implements hash maps with \c uintptr_t keys and arbitrary value
 ** slots.  The slots are stored on the nodes for maximum efficiency.  You can
 ** still store and access arbitrary values using the \c *_mem functions,
 ** whereas some specialised functions are provided for pointers and integers.
 **
 ** \see cucon_uset_h
 ** \see cucon_pmap_h */

typedef struct cucon_umap_node *cucon_umap_node_t;

/** An integer-keyed map with variable-sized inline value slots. */
struct cucon_umap
{
    size_t size; /* the number of elements in the map. */
    size_t mask; /* = capacity - 1 */
    cucon_umap_node_t *arr;
};

struct cucon_umap_node
{
    cu_uintptr_t key;
    struct cucon_umap_node *next;
    /* variable size data */
};

/* This is used in sources produced by cuex_otcomp. */
#define CUCON_UMAP_NODE_INIT(key) { key }

CU_SINLINE cu_uintptr_t cucon_umap_node_key(cucon_umap_node_t node)
{ return node->key; }

typedef struct
{
    struct cucon_umap_node **node_head;
    struct cucon_umap_node *node;
} cucon_umap_it_t;

/** Construct \a map as an empty property map. */
void cucon_umap_init(cucon_umap_t map);

/** Return an empty property map. */
cucon_umap_t cucon_umap_new(void);

/** Return an empty property map with <tt>void *</tt> keys. */

/** Construct \a dst as a copy of \a src but dropping all value slots. */
void cucon_umap_init_copy_void(cucon_umap_t dst, cucon_umap_t src);

/** Return a copy of \a src after dropping all value slots. */
cucon_umap_t cucon_umap_new_copy_void(cucon_umap_t src);

/** Construct \a dst as a copy of \a src assuming slots are \a slot_size bytes
 ** which can be copied with memcpy. */
void cucon_umap_init_copy_mem(cucon_umap_t dst, cucon_umap_t src,
			      size_t slot_size);

/** Return a copy of \a src assuming slots are \a slot_size bytes which can be
 ** copied with memcpy. */
cucon_umap_t cucon_umap_new_copy_mem(cucon_umap_t src, size_t slot_size);

/** Copy \a src to \a dst, assuming all slots have the same size, where \a
 ** value_init_copy copies the value at \a src_slot to \a dst_slot. */
void cucon_umap_init_copy_mem_ctor(
    cucon_umap_t dst, cucon_umap_t src, size_t slot_size,
    cu_clop(value_init_copy, void, void *dst_slot, void *src_slot,
	    uintptr_t key));

/** Copy \a src to \a dst where \a src may have variable slot size.  \a
 ** node_alloc_copy shall call \a cucon_umap_alloc to allocate some \e node,
 ** and construct a copy of \a src_slot at the location given by \c
 ** cucon_umap_node_get_mem(node). */
void cucon_umap_init_copy_node(
    cucon_umap_t dst, cucon_umap_t src,
    cu_clop(node_alloc_copy, cucon_umap_node_t, void *src_slot, uintptr_t key));
#define cucon_umap_node_alloc(slot_size) \
	cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_umap_node) + slot_size)
#define cucon_umap_node_get_mem(node) \
	CU_ALIGNED_MARG_END(cucon_umap_node_t, node)

/** Swap the contents of \a map0 with \a map1. */
void cucon_umap_swap(cucon_umap_t map0, cucon_umap_t map1);

/** Insert \a node, which must be prepared with a key, into \a map. */
cu_bool_t cucon_umap_insert_init_node(cucon_umap_t map, cucon_umap_node_t node);

/** If \a key is not in \a map, allocates a node of size \a node_size,
 ** initialises it the key \a key, inserts it, assigns it to \c *\a node_out,
 ** and return true.  Otherwise, returns false.  This works if the full node
 ** inherits \c cucon_umap_node <i>at the top</i> of the struct. */
cu_bool_t cucon_umap_insert_new_node(cucon_umap_t map, uintptr_t key,
				     size_t node_size, cu_ptr_ptr_t node_out);

/** If \a key has a mapping in \a map, set \c *\a slot to a pointer to the
 ** value and return false, else create a new mapping to \a slot_size bytes of
 ** assigned to \c *\a slot and return true.  This the generic insert which
 ** supports arbitrary value types. */
cu_bool_t cucon_umap_insert_mem(cucon_umap_t map, uintptr_t key,
				size_t slot_size, cu_ptr_ptr_t slot);

/** Insert \a key into \a map unless it exists. Returns true iff the insertion
 ** was done. */
CU_SINLINE cu_bool_t cucon_umap_insert_void(cucon_umap_t map, uintptr_t key)
{ return cucon_umap_insert_mem(map, key, 0, NULL); }

/** Insert (\a key, \a ptr) into \a map if it does not exist.  Returns true iff
 ** the insertion was done.  This is a shortcut equivalent to calling \ref
 ** cucon_umap_insert_mem and initialising the slot with \a ptr. */
cu_bool_t cucon_umap_insert_ptr(cucon_umap_t map, uintptr_t key, void *ptr);

/** If \a key is not in \a map, map it to \a val and return true, else return
 ** false.  If \ref cucon_umap_find_int is used, then \a val should not be \c
 ** INT_MIN for the obvious reason.  This is a shortcut equivalent to calling
 ** \ref cucon_umap_insert_mem and initialising the slot with \a val. */
cu_bool_t cucon_umap_insert_int(cucon_umap_t map, uintptr_t key, int val);

/** If \a key is not in \a map, insert a node allocated with \a alloc_node
 ** mapped from \a key and return true, else return false.  In any case <tt>*\a
 ** node_out</tt> is set to the node of \a key.  This function lets you embed a
 ** node in your own data structures.  \a alloc_node must use garbage
 ** collection. */
cu_bool_t
cucon_umap_insert_general(cucon_umap_t map, uintptr_t key,
			  cu_clop0(alloc_node, cucon_umap_node_t),
			  cucon_umap_node_t *node_out);

/** If \a key is bound in \a map, assume it is bound to a pointer, replace the
 ** poiter with \a ptr and return the old pointer, else bind \a key to \a ptr
 ** and return NULL.
 ** \pre The slot of \a key, if present, must hold a pointer. */
void *cucon_umap_replace_ptr(cucon_umap_t map, uintptr_t key, void *ptr);

/** If \a key has a mapping in \a map, replace it with \a val and return the
 ** old mapping, else insert \a val and return \c INT_MIN.  As a special case,
 ** if \a val is INT_MIN, erase the mapping of \a key and return the old value
 ** or \c INT_MIN of none.
 ** \pre The solt of \a key, if present, must hold an \c int. */
int cucon_umap_replace_int(cucon_umap_t map, uintptr_t key, int val);

/** If \a key has a mapping in \a map, erase it and return true, else return
 ** false.  To optimise for many deletions, use \c _keep_cap variants and call
 ** \ref cucon_umap_update_cap once afterwards. */
cu_bool_t cucon_umap_erase(cucon_umap_t map, uintptr_t key);

/** If \a key has a mapping in \a map, erase it and return the pointer value
 ** stored in the slot, else return \c NULL.
 ** \pre The slot of \a key, if present, must hold a pointer. */
void *cucon_umap_erase_ptr(cucon_umap_t map, uintptr_t key);

/** If \a key has a mapping in \a map, erase it and return the \c int value
 ** stored in the slot, else return \c * INT_MIN.
 ** \pre The slot of \a key, if present, must hold an \c int. */
int cucon_umap_erase_int(cucon_umap_t map, uintptr_t key);

/** Same as \ref cucon_umap_erase, except the capacity of \a map is kept. */
cu_bool_t cucon_umap_erase_keep_cap(cucon_umap_t map, uintptr_t key);

/** Erase any element of \a map and return the erased node.  This uses a random
 ** number generator to prevent compromising the average case structure of \a
 ** map. */
cucon_umap_node_t cucon_umap_pop_any_node(cucon_umap_t map);

/** As \ref cucon_umap_pop_any_node, but return key instead of node. */
CU_SINLINE uintptr_t cucon_umap_pop_any_key(cucon_umap_t map)
{
    cucon_umap_node_t node = cucon_umap_pop_any_node(map);
    return node->key;
}

/** Update capacity after \c _keep_cap calls. */
void cucon_umap_update_cap(cucon_umap_t map);

/** Returns the internal node of \a key, or \c NULL if none. */
cucon_umap_node_t cucon_umap_find_node(cucon_umap_t map, uintptr_t key);

/** If \a key has a mapping in \a map, return a pointer to the slot, else
 ** return \c NULL. */
void *cucon_umap_find_mem(cucon_umap_t map, uintptr_t key);

/** If \a key has a mapping in \a map, interpret the slot as a pointer and
 ** return it, else return \c NULL.  */
void *cucon_umap_find_ptr(cucon_umap_t map, uintptr_t key);

/** If \a key is in \a map, return its mapping, else return \c INT_MIN.
 ** \pre The slot of \a key, if present, must hold an integer. */
int cucon_umap_find_int(cucon_umap_t map, uintptr_t key);

/** True iff \a map contains \a key, ignoring the slot. */
CU_SINLINE cu_bool_t cucon_umap_find_void(cucon_umap_t map, uintptr_t key)
{ return cucon_umap_find_node(map, key) != NULL; }

/** Call \a cb on each key-value pair in \a map. */
void cucon_umap_iter_mem(cucon_umap_t map,
			 cu_clop(cb, void, uintptr_t key, void *value));
/* TODO. cucon_umap_iter_ptr */

cu_bool_t cucon_umap_conj_general(cucon_umap_t map, size_t node_offset,
				  cu_clop(cb, cu_bool_t, uintptr_t, void *));

/** Sequentially conjunct \a cb over (key, value) pairs in \a map. */
cu_bool_t cucon_umap_conj_mem(cucon_umap_t map,
			      cu_clop(cb, cu_bool_t, uintptr_t, void *));
/* TODO. cucon_umap_conj_ptr */

/** Call \a cb on each key in \a map. */
void cucon_umap_iter_keys(cucon_umap_t map, cu_clop(cb, void, uintptr_t));

/** Call \a cb on each key of \a map in increasing order. */
void cucon_umap_iter_increasing_keys(cucon_umap_t map,
				     cu_clop(cb, void, uintptr_t));

/** Perform a sequantial conjunction of \a cb over keys in \a map. */
cu_bool_t cucon_umap_conj_keys(cucon_umap_t map,
			       cu_clop(cb, cu_bool_t, uintptr_t));

/** Sequential conjunction of \a cb over nodes in \a map. */
cu_bool_t cucon_umap_conj_node(cucon_umap_t map,
			       cu_clop(cb, cu_bool_t, cucon_umap_node_t node));

/** Move all pairs of \a map0 with keys not in \a map1 to \a map1.  This
 ** effectively assigns \a map0 ∩ \a map1 to \a map0 and \a map0 ∪ \a map1 to
 ** \a map1, where only keys are considered for the set operation tests. */
void cucon_umap_assign_isecn_union(cucon_umap_t map0, cucon_umap_t map1);

/** For each element in \a src0 ∩ \a src1, move one pair to \a dst and delete
 ** the other.  It is unspecified which of the values from the sources are
 ** moved.  If \a dst already contains the key, the elemets of both sources are
 ** simply discarded. */
void cucon_umap_move_isecn(cucon_umap_t dst,
			   cucon_umap_t src0, cucon_umap_t src1);

/** Assign to \a dst the intersection \a dst ∩ \a src, keeping the slots.  */
void cucon_umap_assign_isecn(cucon_umap_t dst, cucon_umap_t src);

/** Assign to \a dst the union of \a dst and \a src, with void slots for
 ** duplicated nodes. */
void cucon_umap_assign_union_void(cucon_umap_t dst, cucon_umap_t src);

/** For profiling use. */
void cucon_umap_dump_stats(cucon_umap_t map, FILE *out);

/** Return the number of elements in \a map. */
CU_SINLINE size_t cucon_umap_size(cucon_umap_t map) { return map->size; }

/** True iff \a map is empty. */
CU_SINLINE cu_bool_t cucon_umap_is_empty(cucon_umap_t map) {return!map->size;}

/** True iff \a map0 and \a map1 contains the same sets of keys. */
cu_bool_t cucon_umap_eq_keys(cucon_umap_t map0, cucon_umap_t map1);

/** True iff \a map0 and \a map1 are equal, assuming slot size is \a slot_size
 ** bytes and trivially comparable. */
cu_bool_t cucon_umap_eq_mem(cucon_umap_t map0, cucon_umap_t map1,
			    size_t slot_size);

/** True iff \a map0 and \a map1 are equal, assuming slots are pointers with
 ** pointer equality. */
CU_SINLINE cu_bool_t cucon_umap_eq_ptr(cucon_umap_t map0, cucon_umap_t map1)
{ return cucon_umap_eq_mem(map0, map1, sizeof(void *)); }

/** Returns a hash of the keys in \a map. */
cu_hash_t cucon_umap_hash_keys(cucon_umap_t map);

/** Returns a hash of the keys and slots of \a map, assuming slots are \a
 ** slot_size bytes long. */
cu_hash_t cucon_umap_hash_mem(cucon_umap_t map, size_t slot_size);

/** Returns a hash of the keys and slots of \a map, assuming the slots are
 ** pointers. */
CU_SINLINE cu_hash_t cucon_umap_hash_ptr(cucon_umap_t map)
{ return cucon_umap_hash_mem(map, sizeof(void *)); }

/** Prints the keys of \a map to \a out in set notation. */
void cucon_umap_print_keys(cucon_umap_t map, FILE *out);

/* A range over entries in \a map. \ref cucon_umap_end_eq is faster than
 * comparing with the result of \ref cucon_umap_end, even if the result of
 * \ref cucon_umap_end is stored. */
cucon_umap_it_t cucon_umap_begin(cucon_umap_t map);
cucon_umap_it_t cucon_umap_end(cucon_umap_t map);

CU_SINLINE cu_bool_t
cucon_umap_end_eq(cucon_umap_t map, cucon_umap_it_t it)
{
    return it.node == (void *)-1;
}

#define cucon_umap_it_eq(it0, it1) ((it0.node) == (it1.node))

cucon_umap_it_t cucon_umap_it_next(cucon_umap_it_t it);

/* Return the key part at 'it'.  */
#define cucon_umap_it_key(it) ((it).node->key)

/* Return the value part at 'it'. */
#define cucon_umap_it_value_mem(it) ((void*)CU_ALIGNED_PTR_END((it).node))
#define cucon_umap_it_value_ptr(it) (*(void**)CU_ALIGNED_PTR_END((it).node))

/*!\deprecated Use \ref cucon_umap_init. */
#define cucon_umap_cct			cucon_umap_init
/*!\deprecated Use \ref cucon_umap_init_copy_void. */
#define cucon_umap_cct_copy_void	cucon_umap_init_copy_void
/*!\deprecated Use \ref cucon_umap_init_copy_mem. */
#define cucon_umap_cct_copy_mem		cucon_umap_init_copy_mem
/*!\deprecated Use \ref cucon_umap_init_copy_mem_ctor. */
#define cucon_umap_cct_copy_mem_ctor	cucon_umap_init_copy_mem_ctor
/*!\deprecated Use \ref cucon_umap_init_copy_node. */
#define cucon_umap_cct_copy_node	cucon_umap_init_copy_node

/** @} */
CU_END_DECLARATIONS

#endif
