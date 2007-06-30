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

#ifndef CUCON_HMAP_H
#define CUCON_HMAP_H

#include <cucon/fwd.h>
#include <cu/clos.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_hmap cucon/hmap.h: General Hash Maps
 * @{\ingroup cucon_maps_and_sets_mod */

typedef struct cucon_hmap_it_s cucon_hmap_it_t;
typedef struct cucon_hmap_node_s *cucon_hmap_node_t;

struct cucon_hmap_node_s
{
    cucon_hmap_node_t next;
    void *key;
    /* value data */
};

struct cucon_hmap_s
{
    cu_clop(eq, cu_bool_t, void *, void *);
    cu_clop(hash, cu_hash_t, void *);
    cucon_hmap_node_t *table;
    int size;
    cu_hash_t mask;
    struct cucon_hmap_node_s tail;
};

struct cucon_hmap_it_s
{
    cucon_hmap_node_t *node_head;
    cucon_hmap_node_t node;
};

void		cucon_hmap_cct(cucon_hmap_t hm,
			     cu_clop(eq, cu_bool_t, void *, void *),
			     cu_clop(hash, cu_hash_t, void *));
void		cucon_hmap_dct_free(cucon_hmap_t);

/* Return a hash set over objects with equality defined by 'equal' and
 * hash codes created by 'hash'. */
cucon_hmap_t	cucon_hmap_new(cu_clop(eq, cu_bool_t, void *, void *),
			     cu_clop(hash, cu_hash_t, void *));

/* Erase all entries in the hash set. */
void		cucon_hmap_clear(cucon_hmap_t);

/* If an object equal to 'key' is in 'hs', return its value slot, else
 * retrun NULL. */
void *		cucon_hmap_find_mem(cucon_hmap_t hs, void *key);

void *		cucon_hmap_find_ptr(cucon_hmap_t hs, void *key);

/* If 'key' is in 'hs', return false and set '*slot' to its value
 * slot, else return true and associate 'key' with 'slot_size' bytes
 * of value slot assigned to '*slot'. */
cu_bool_t	cucon_hmap_insert_mem(cucon_hmap_t hs, void *key,
				    size_t slot_size, cu_ptr_ptr_t slot);

void *		cucon_hmap_erase(cucon_hmap_t hs, void *key);

void		cucon_hmap_set_capacity(cucon_hmap_t, int);

#define		cucon_hmap_size(hs) ((size_t const)(hs)->size)
#define		cucon_hmap_is_empty(hs) (cucon_hmap_size(hs) == 0)

cu_bool_t cucon_hmap_conj_mem(cucon_hmap_t map,
			     cu_clop(cb, cu_bool_t, void const *, void *));

cucon_hmap_it_t	cucon_hmap_begin(cucon_hmap_t);
cucon_hmap_it_t	cucon_hmap_end(cucon_hmap_t);
#define		cucon_hmap_it_eq(it1, it2) ((it1)->node == (it2)->node)
cucon_hmap_it_t	cucon_hmap_it_next(cucon_hmap_it_t it);

/*typedef struct cucon_hmap_it_t {
    cucon_hmap_ths;
    int idx;
    void *entry;
};

hash_set_iterator* cucon_hmap_begin(cucon_hmap_t hs);
hash_set_iterator* cucon_hmap_end(cucon_hmap_t hs);
int hsi_equals(cucon_hmap_it_t *hi, cucon_hmap_it_t *hi1);
void cucon_hmap_next(hash_set_iterator *hi);*/

/*!@}*/
CU_END_DECLARATIONS
#endif
