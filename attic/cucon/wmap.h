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

/* !!! DON'T USE THIS. IT IS BORKEN. !!! */

#ifndef CUCON_WMAP_H
#define CUCON_WMAP_H

#include <stdlib.h>
#include <cucon/fwd.h>
#include <cu/memory.h>
#include <cu/clos.h>

CU_BEGIN_DECLARATIONS
/* \defgroup cucon_wmap cucon/wmap.h: Weak-keyed Hash Maps
 * @{ \ingroup cucon_mod
 * \warning The cucon_wmap is not completely debugged. */

struct cucon_wmap_s
{
    size_t size; /* the number of elements in the map. */
    size_t mask; /* = capacity - 1 */
    struct cuconP_wmap_node_s **arr;
};

typedef struct cuconP_wmap_node_s *cuconP_wmap_node_t;
struct cuconP_wmap_node_s
{
    cu_uintptr_t key;
    cuconP_wmap_node_t next;
    /* variable size data */
};

typedef struct
{
    cuconP_wmap_node_t *node_head;
    cuconP_wmap_node_t node;
} cucon_wmap_it_t;

/* Construct 'wmap' as an empty property map.  See the corresponding
 * 'new' functions. */
void cucon_wmap_cct(cucon_wmap_t wmap);

/* Return an empty property map where keys are weak pointers.  The
 * latter means that when a key is no longer accessible elsewhere in
 * the program, its mapping can be garbage collected.  Note that
 * 'cucon_wmap_size' is unreliable and 'cucon_wmap_for' may map over more
 * keys that those actually accessible in the program.  This
 * constructor is suitable when the map is expected to have a longer
 * life-time that the keys. */
cucon_wmap_t cucon_wmap_new(void);

/* If 'key' has a mapping in 'wmap', set '*slot' to a pointer to the
 * value and return 0, else create a new mapping to 'slot_size' bytes
 * of assigned to '*slot' and return 1.  The object at 'key' is not
 * considered reachable by the collector as a consequence of its
 * presence in 'wmap'.  Moveover, when 'key' is collected, its entry
 * in 'wmap' will be erased.  */
int cucon_wmap_insert_mem(cucon_wmap_t wmap, void const *key,
			size_t slot_size, cu_ptr_ptr_t slot);

/* Same as above, but with empty slot. */
int cucon_wmap_insert_void(cucon_wmap_t wmap, void const *key);
#define cucon_wmap_insert_void(wmap, key) cucon_wmap_insert_mem(wmap, key, 0,NULL)

/* If 'key' has a mapping in 'wmap', assume its value contains a
 * pointer, return the pointer in '*ptr' and return 0, else allocate a
 * pointer, set it to '*ptr' and return 1. */
int cucon_wmap_insert_ptr(cucon_wmap_t wmap, void const *key, void **ptr);

/* If 'key' is bound in 'wmap', assume it is bound to a pointer, and
 * replace the poiter with 'ptr' and return the old pointer, else bind
 * 'key' to 'ptr' and return NULL. */
void *cucon_wmap_replace_ptr(cucon_wmap_t wmap, void const *key, void *ptr);

/* If 'key' has a mapping in 'wmap', erase it and return 1, else
 * return 0. */
int cucon_wmap_erase(cucon_wmap_t wmap, void const *key);
#define cucon_wmap_erase(wmap, key) cuconP_wmap_erase(wmap, key, 0)
int cuconP_wmap_erase(cucon_wmap_t wmap, void const *key, int is_in_finaliser);

/* If 'key' has a mapping in 'wmap', retrun a pointer to the value,
 * else return NULL. */
void *cucon_wmap_find_mem(cucon_wmap_t wmap, void const *key);

/* If 'key' has a mapping in 'tmap', return assume the value is a pointer,
 * and return it, else return NULL. */
void *cucon_wmap_find_ptr(cucon_wmap_t wmap, void const *key);

/* Evaluate
 *     void (__closure fnc)(void const *key, void *value) // for cucon_wmap_for
 * for each 'key' to 'value' mapping in the property map, where
 * 'value' points the immediate value memory. */
void cucon_wmap_for(cucon_wmap_t wmap, cu_clop(cb, void, void const *, void *));

/* Call
 *     void (__closure cb)(void const *key) // for cucon_wmap_for_keys
 * for each 'key' in 'wmap'. */
void cucon_wmap_for_keys(cucon_wmap_t wmap, cu_clop(cb, void, void const *));

/* For profiling use. */
void cucon_wmap_show_stats(cucon_wmap_t wmap);

/* Return the number of elements in 'wmap'. */
#define cucon_wmap_size(wmap) ((wmap)->size)

#if 0 /* Not safe */
/* A range over entries in 'tmap'. 'cucon_wmap_end_eq' is faster than
 * comparing with the result of 'cucon_wmap_end', even if the result of
 * 'cucon_wmap_end' is stored. */
cucon_wmap_it_t cucon_wmap_begin(cucon_wmap_t tmap);
cucon_wmap_it_t cucon_wmap_end(cucon_wmap_t tmap);
#ifdef CUCONF_AVOID_WARNINGS
#  define cucon_wmap_end_eq(tmap, it) ((tmap)? (it).node == (void*)-1 : 0)
#else
#  define cucon_wmap_end_eq(tmap, it) ((tmap), (it).node == (void*)-1)
#endif
#define cucon_wmap_it_eq(it0, it1) ((it0.node) == (it1.node))
cucon_wmap_it_t cucon_wmap_it_next(cucon_wmap_it_t it);

#define cucon_wmap_it_key(it) cu_reveal_ptr((it).node->key)

/* Return the value part at 'it'. */
#define cucon_wmap_it_value_mem(it) ((void*)CU_ALIGNED_PTR_END((it).node))
#define cucon_wmap_it_value_ptr(it) (*(void**)CU_ALIGNED_PTR_END((it).node))
#endif

/*!@}*/
CU_END_DECLARATIONS

#endif
