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

/*!\file
 * \deprecated Use cucon/hmap.h */

#ifndef CUCON_HSET_H
#define CUCON_HSET_H

#include <cu/clos.h>
#include <cucon/fwd.h>

CU_BEGIN_DECLARATIONS

typedef struct cucon_hset_s *cucon_hset_t;
typedef struct cucon_hset_it_s cucon_hset_it_t;
typedef struct cucon_hset_node_s *cucon_hset_node_t;

struct cucon_hset_node_s
{
    cucon_hset_node_t next;
    void *key;
};

struct cucon_hset_s
{
    cu_clop(equals, cu_bool_t, void *, void *);
    cu_clop(hash, cu_hash_t, void *);
    cucon_hset_node_t *table;
    size_t count;
    cu_hash_t mask;
    struct cucon_hset_node_s tail;
};

struct cucon_hset_it_s
{
    cucon_hset_node_t *node_head;
    cucon_hset_node_t node;
};

/* Create a new hash set over elements with equality predicate 'equal'
 * and hash function 'hash'. */
void
cucon_hset_cct(cucon_hset_t,
	     cu_clop(equal, cu_bool_t, void *, void *),
	     cu_clop(hash, cu_hash_t, void *));
cucon_hset_t
cucon_hset_new(cu_clop(equal, cu_bool_t, void *, void *),
	     cu_clop(hash, cu_hash_t, void *));


/* Redundant. */
void		cucon_hset_dct_free(cucon_hset_t);

/* Erase all entries in the hash set. */
void		cucon_hset_clear(cucon_hset_t);

/* If an key equal to 'key' is in 'hs', return it, else return NULL. */
void *		cucon_hset_find(cucon_hset_t hs, void *key);

/* If an key equal to 'key' is in 'hs', return it, else insert
 * 'key' and return NULL. */
void *		cucon_hset_insert(cucon_hset_t hs, void *key);

/* Insert 'key' into 'hs' possibly replacing an key which compares
 * equal. The old key is returned or NULL if none. */
void *		cucon_hset_replace(cucon_hset_t hs, void *key);

/* If an key equal to 'key' is in 'hs', erase it from 'hs' and return it,
 * else return NULL. */
void *		cucon_hset_erase(cucon_hset_t hs, void *key);
void *		cucon_hset_erase_keep_capacity(cucon_hset_t hs, void *key);

void		cucon_hset_set_capacity(cucon_hset_t, int);

#define		cucon_hset_size(hs) ((size_t const)(hs)->count)
#define		cucon_hset_is_empty(hs) (cucon_hset_size(hs) == 0)

/* Evaluate the sequential conjunction of 'cb' over 'hs'. */
cu_bool_t	cucon_hset_conj(cucon_hset_t hs,
			      cu_clop(cb, cu_bool_t, void *));

/* Comparison of whole hash sets, assuming they have the same callbacks. */
cu_bool_t	cucon_hset_eq(cucon_hset_t hs0, cucon_hset_t hs1);	/* = */
cu_bool_t	cucon_hset_sub(cucon_hset_t hs0, cucon_hset_t hs1);	/* ⊂ */
cu_bool_t	cucon_hset_subeq(cucon_hset_t hs0, cucon_hset_t hs1);	/* ⊆ */

/* Iterators
 * --------- */

cucon_hset_it_t	cucon_hset_begin(cucon_hset_t);
cucon_hset_it_t	cucon_hset_end(cucon_hset_t);
#define		cucon_hset_it_eq(it1, it2) ((it1)->node == (it2)->node)
cucon_hset_it_t	cucon_hset_it_next(cucon_hset_it_t it);

void		cucon_hset_check_integrity(cucon_hset_t);

#endif
