/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_HZMAP_H
#define CUCON_HZMAP_H

#include <cucon/fwd.h>
#include <cu/conf.h>
#include <cu/clos.h>
#include <stdio.h>

#define CUCON_HZMAP_COMPACT

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_hzmap_h cucon/hzmap.h: Hash Map of Flat Fixed-Sized Keys
 *@{\ingroup cucon_maps_and_sets_mod
 * \see cucon_hzset_h
 * \see cucon_umap_h
 * \see cucon_pmap_h
 */

/*!Node base struct for \ref cucon_hzmap. */
struct cucon_hzmap_node
{
    cucon_hzmap_node_t next;
};

#define CUCON_HZMAP_NODE_INIT {NULL}

/*!A hash map of fixed-size keys. */
struct cucon_hzmap
{
#ifdef CUCON_HZMAP_COMPACT
#  if CUCONF_SIZEOF_SIZE_T <= 4
    unsigned short key_size_w;
    unsigned short cap_expt;
#  else
    cu_shortsize_t key_size_w;
    cu_shortsize_t cap_expt;
#  endif
#else
    cu_shortsize_t key_size_w;
    size_t mask;
#endif
    size_t size;
    cucon_hzmap_node_t *arr;
};

/*!Number of elements in \a map. */
CU_SINLINE size_t cucon_hzmap_size(cucon_hzmap_t map)
{ return map->size; }

/*!The current size of the underlying array. */
CU_SINLINE size_t cucon_hzmap_capacity(cucon_hzmap_t map)
#ifdef CUCON_HZMAP_COMPACT
{ return (size_t)1 << map->cap_expt; }
#else
{ return map->mask + 1; }
#endif

/*!Initialize \a map as an empty map to be used with elements of key size \a
 * key_size_w words. */
void cucon_hzmap_init(cucon_hzmap_t map, cu_shortsize_t key_size_w);

/*!Return an empty map to be used for elements with key size \a key_size_w
 * words. */
cucon_hzmap_t cucon_hzmap_new(cu_shortsize_t key_size_w);

/*!As an optimisation, this function may be called prior to a sequence of
 * inserts to inform how many keys will be inserted.  This will cause the
 * underlying array to be resized to it's final size right away. */
void cucon_hzmap_prepare_insert(cucon_hzmap_t, size_t count);

/*!Given that \a node is initialised with a key of suitable size for \a map,
 * if they key exists in \a map, returns false, else links \a node to \a map
 * and returns true.  The \ref cucon_hzmap_node base struct of \a node is
 * initialised by this call if the insert takes place. */
cu_bool_t cucon_hzmap_insert_node(cucon_hzmap_t map, cucon_hzmap_node_t node);

/*!If a node with key \a key exists in \a map, assign it to \c *\a node_out and
 * return false, else insert a new node of size \a node_size, set it's key to
 * \a key, assign it to \c *\a node_out, and return true. */
cu_bool_t cucon_hzmap_insert(cucon_hzmap_t map, void const *key,
			     size_t node_size, cucon_hzmap_node_t *node_out);

/*!If a node of key \a key exists in \a map, return false, else insert a node
 * of key \a key an no othe data. */
cu_bool_t cucon_hzmap_insert_void(cucon_hzmap_t, void const *key);

/*!If \a map contains a node with key \a key, erase it and return true, else
 * return false. */
cu_bool_t cucon_hzmap_erase(cucon_hzmap_t, void const *key);

/*!Same as \ref cucon_hzmap_erase, except the underlying capacity of \a map
 * will be left unchanged.  This is used for optimising code where many keys
 * are erased in a row.  Call \ref cucon_hzmap_finish_erase at the end to
 * adjust the capacity. */
cu_bool_t cucon_hzmap_step_erase(cucon_hzmap_t map, void const *key);

/*!Adjusts the capacity after a sequence of \ref cucon_hzmap_erase. */
void cucon_hzmap_finish_erase(cucon_hzmap_t);

/*!Returns the node in \a map with key \a key, or \c NULL if no such node
 * exists. */
cucon_hzmap_node_t cucon_hzmap_find(cucon_hzmap_t map, void const *key);

/*!A version of \a cucon_hzmap_find_node optimised for the case when it is
 * known that \a map has 1 word keys. */
cucon_hzmap_node_t cucon_hzmap_1w_find(cucon_hzmap_t map, void const *key);

/*!A version of \a cucon_hzmap_find_node optimised for the case when it is
 * known that \a map has 2 word keys. */
cucon_hzmap_node_t cucon_hzmap_2w_find(cucon_hzmap_t map, void const *key);

/*!True if \a f maps all elements of \a map to true, otherwise exits with false
 * on the first element which maps to false. */
cu_bool_t cucon_hzmap_forall(cu_clop(f, cu_bool_t, cucon_hzmap_node_t),
			     cucon_hzmap_t map);

/*!As \ref cucon_hzmap_forall, but pass only keys to \a f. */
cu_bool_t cucon_hzmap_forall_keys(cu_clop(f, cu_bool_t, void const *key),
				  cucon_hzmap_t map);

/*!Removes all elements from \a map which \a f maps to false. */
void cucon_hzmap_filter(cu_clop(f, cu_bool_t, cucon_hzmap_node_t),
			cucon_hzmap_t map);

/*!As \ref cucon_hzmap_filter, but pass only keys to \a f. */
void cucon_hzmap_filter_keys(cu_clop(f, cu_bool_t, void const *key),
			     cucon_hzmap_t map);

struct cucon_hzmap_itr
{
    cucon_hzmap_node_t *arr_cur;
    cucon_hzmap_node_t *arr_end;
    cucon_hzmap_node_t node;
};

/*!Initialise an iterator over elements of \a map. */
void cucon_hzmap_itr_init(cucon_hzmap_itr_t itr, cucon_hzmap_t map);

/*!The text element of the sequence initialised with \ref cucon_hzmap_itr_init,
 * or \c NULL if there are no more elements. */
cucon_hzmap_node_t cucon_hzmap_itr_get(cucon_hzmap_itr_t itr);

void const *cucon_hzmap_itr_get_key(cucon_hzmap_itr_t itr);

void cucon_hzmap_dump_stats(cucon_hzmap_t map, FILE *out);

/*!@}*/
CU_END_DECLARATIONS

#endif
