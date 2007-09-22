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


#include <cucon/slist.h>
#include <cu/memory.h>
#include <stdlib.h>

void
cucon_slist_cct(cucon_slist_t list)
{
    list->head.next = &list->tail;
    list->tail.next = &list->head;
}

void
cucon_slist_cct_tail(cucon_slist_t list, cucon_slist_t src, cucon_slist_it_t src_it)
{
    list->head = src_it->next;
    list->tail = src->tail;
}

void
cucon_slist_it_erase(cucon_slist_it_t it)
{
    cuconP_slist_node_t *nd = it->next;
    it->next = it->next->next;
    if (nd->next->next == nd)
	nd->next->next = it;
}

cucon_slist_it_t
cucon_slist_insert_mem(cucon_slist_it_t it, size_t size)
{
    cuconP_slist_node_t *p
	= cu_galloc(CU_ALIGNED_SIZEOF(cuconP_slist_node_t) + size);
    p->next = it->next;
    it->next = p;
    if (p->next->next == it)
	p->next->next = p;
    return it;
}

cucon_slist_it_t
cucon_slist_insert_ptr(cucon_slist_it_t it, void *ptr)
{
    cuconP_slist_node_t *p
	= cu_galloc(CU_ALIGNED_SIZEOF(cuconP_slist_node_t) + sizeof(void*));
    p->next = it->next;
    it->next = p;
    if (p->next->next == it)
	p->next->next = p;
    cucon_slist_it_get_ptr(it) = ptr;
    return it;
}

