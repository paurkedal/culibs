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

#ifndef CUCON_SLIST_H
#define CUCON_SLIST_H

#include <stddef.h>
#include <cucon/fwd.h>

CU_BEGIN_DECLARATIONS

typedef struct cuconP_slist_node_s cuconP_slist_node_t;
typedef struct cucon_slist_s *cucon_slist_t;
typedef cuconP_slist_node_t *cucon_slist_it_t;

struct cuconP_slist_node_s {
    cuconP_slist_node_t *next;
};
struct cucon_slist_s {
    cuconP_slist_node_t head;
    cuconP_slist_node_t tail;
};

/*
 *  Basis
 */

void			cucon_slist_cct(cucon_slist_t);
void			cucon_slist_cct_tail(cucon_slist_t, cucon_slist_it_t it);
#define			cucon_slist_dct(list)
#define			cucon_slist_clear(list) cucon_slist_cct(list)
#define			cucon_slist_begin(list)	(&(list)->head)
#define			cucon_slist_end(list)	((list)->tail.next)
cucon_slist_it_t		cucon_slist_insert_mem(cucon_slist_it_t, size_t);
cucon_slist_it_t		cucon_slist_insert_ptr(cucon_slist_it_t, void *);
void			cucon_slist_it_erase(cucon_slist_it_t);
#define			cucon_slist_it_get_mem(it) \
			CU_ALIGNED_PTR_END((it)->next)
#define			cucon_slist_it_get_ptr(it) \
			(*(void**)CU_ALIGNED_PTR_END((it)->next))
#define			cucon_slist_it_next(it)	((it)->next)

/*
 *  Derived
 */

#define			cucon_slist_prepend_alloc(list, size) \
	cucon_slist_insert_mem(cucon_slist_begin(list), (size))
#define			cucon_slist_prepend_ptr(list, p) \
	cucon_slist_insert_ptr(cucon_slist_begin(list), (p))
#define			cucon_slist_append_mem(list, size) \
	cucon_slist_insert_mem(cucon_slist_end(list), (size))
#define			cucon_slist_append_ptr(list, p) \
	cucon_slist_insert_ptr(cucon_slist_end(list), (p))

CU_END_DECLARATIONS

#endif
