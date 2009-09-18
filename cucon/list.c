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


#include <cucon/list.h>
#include <cu/memory.h>
#include <cu/ptr_seq.h>

struct cucon_list_s cuconP_list_empty;

void
cucon_list_init(cucon_list_t list)
{
    list->eol.next = &list->eol;
    list->eol.prev = &list->eol;
}

void
cucon_list_init_copy_ptr(cucon_list_t dst, cucon_list_t src)
{
    cucon_listnode_t it;
    cucon_list_init(dst);
    for (it = cucon_list_begin(src); it != cucon_list_end(src);
	 it = cucon_listnode_next(it))
	cucon_list_append_ptr(dst, cucon_listnode_ptr(it));
}

cucon_list_t
cucon_list_new(void)
{
    cucon_list_t lst = cu_gnew(struct cucon_list_s);
    cucon_list_init(lst);
    return lst;
}

void
cucon_list_swap(cucon_list_t lst0, cucon_list_t lst1)
{
    if (cucon_list_is_empty(lst1)) {
	if (cucon_list_is_empty(lst0))
	    return;
	CU_SWAP(cucon_list_t, lst0, lst1);
    }
    if (cucon_list_is_empty(lst0)) {
	lst0->eol.next = lst1->eol.next;
	lst0->eol.prev = lst1->eol.prev;
	cucon_list_init(lst1);
    }
    else {
	CU_SWAP(cucon_listnode_t, lst0->eol.next, lst1->eol.next);
	CU_SWAP(cucon_listnode_t, lst0->eol.prev, lst1->eol.prev);
	lst1->eol.next->prev = &lst1->eol;
	lst1->eol.prev->next = &lst1->eol;
    }
    lst0->eol.next->prev = &lst0->eol;
    lst0->eol.prev->next = &lst0->eol;
}

cu_bool_fast_t
cucon_list_is_empty(cucon_list_t list)
{
    return list->eol.next == &list->eol;
}

cu_bool_fast_t
cucon_list_is_empty_or_singleton(cucon_list_t list)
{
    return list->eol.next == list->eol.prev;
}

cu_bool_fast_t
cucon_list_is_singleton(cucon_list_t list)
{
    return list->eol.next != &list->eol
	&& list->eol.next == list->eol.prev;
}

size_t
cucon_list_count(cucon_list_t list)
{
    cucon_listnode_t it;
    size_t i = 0;
    for (it = cucon_list_begin(list); it != cucon_list_end(list);
	 it = cucon_listnode_next(it))
	++i;
    return i;
}

cucon_listnode_t
cucon_list_insert_it(cucon_listnode_t it, cucon_listnode_t node)
{
    /* Erase. */
    node->next->prev = node->prev;
    node->prev->next = node->next;
    /* Re-insert. */
    node->next = it;
    node->prev = it->prev;
    node->prev->next = node;
    it->prev = node;
    return node;
}

cucon_listnode_t
cucon_list_insert_mem(cucon_listnode_t it, size_t size)
{
    struct cucon_listnode_s *it_new
	= cu_galloc(sizeof(struct cucon_listnode_s) + size);
    it_new->next = it;
    it_new->prev = it->prev;
    it->prev = it_new;
    it_new->prev->next = it_new;
    return it_new;
}

cucon_listnode_t
cucon_list_insert_ptr(cucon_listnode_t it, void *ptr)
{
    struct cucon_listnode_s *it_new
	= cu_galloc(sizeof(struct cucon_listnode_s)+sizeof(void*));
    it_new->next = it;
    it_new->prev = it->prev;
    it->prev = it_new;
    it_new->prev->next = it_new;
    cucon_listnode_set_ptr(it_new, ptr);
    return it_new;
}

cucon_listnode_t
cucon_list_insert_init_node(cucon_listnode_t it, cucon_listnode_t newnode)
{
    newnode->next = it;
    newnode->prev = it->prev;
    newnode->prev->next = newnode;
    it->prev = newnode;
    return newnode;
}

void *
cucon_list_extract_mem(cucon_listnode_t it)
{
    it->next->prev = it->prev;
    it->prev->next = it->next;
    it->next = it->prev = NULL;
    return cucon_listnode_mem(it);
}

void *
cucon_list_extract_ptr(cucon_listnode_t it)
{
    void *ptr = cucon_listnode_ptr(it);
    it->next->prev = it->prev;
    it->prev->next = it->next;
    cu_gfree(it);
    return ptr;
}

cucon_listnode_t
cucon_list_erase_node(cucon_listnode_t it)
{
    it->next->prev = it->prev;
    it->prev->next = it->next;
    return it->next;
}

cucon_listnode_t
cucon_list_find_ptr(cucon_list_t lst, void *ptr)
{
    cucon_listnode_t it;
    for (it = cucon_list_begin(lst);
	 it != cucon_list_end(lst); it = cucon_listnode_next(it))
	if (cucon_listnode_ptr(it) == ptr)
	    break;
    return it;
}

cucon_listnode_t
cucon_list_erase_ptr(cucon_list_t lst, void *ptr)
{
    cucon_listnode_t it;
    for (it = cucon_list_begin(lst);
	 it != cucon_list_end(lst); it = cucon_listnode_next(it))
	if (cucon_listnode_ptr(it) == ptr)
	    return cucon_list_erase_node(it);
    return it;
}

size_t
cucon_list_erase_all_ptr(cucon_list_t lst, void *ptr)
{
    size_t count = 0;
    cucon_listnode_t it = cucon_list_begin(lst);
    while (it != cucon_list_end(lst)) {
	if (cucon_listnode_ptr(it) == ptr) {
	    ++count;
	    it = cucon_list_erase_node(it);
	}
	else
	    it = cucon_listnode_next(it);
    }
    return count;
}

void
cucon_list_rotate_backwards(cucon_list_t lst)
{
    if (cucon_list_is_empty_or_singleton(lst))
	return;
    lst->eol.prev->next = lst->eol.next;
    lst->eol.next->prev = lst->eol.prev;
    lst->eol.prev = lst->eol.next;
    lst->eol.next = lst->eol.next->next;
    lst->eol.prev->next = &lst->eol;
    lst->eol.next->prev = &lst->eol;
}

void
cucon_list_range_iter_mem(cucon_listnode_t first, cucon_listnode_t last,
			  cu_clop(proc, void, void *))
{
    while (first != last) {
	cu_call(proc, cucon_listnode_mem(first));
	first = cucon_listnode_next(first);
    }
}

typedef struct range_source_s *range_source_t;
struct range_source_s
{
    cu_inherit (cu_ptr_source_s);
    cucon_listnode_t cur;
    cucon_listnode_t end;
};

static void *
range_source_get_ptr(cu_ptr_source_t source)
{
    range_source_t self = cu_from(range_source, cu_ptr_source, source);
    cucon_listnode_t cur = self->cur;
    if (cur == self->end)
	return NULL;
    else {
	self->cur = cucon_listnode_next(cur);
	return cucon_listnode_ptr(cur);
    }
}

cu_ptr_source_t
cucon_listrange_source_ptr(cucon_listnode_t begin, cucon_listnode_t end)
{
    range_source_t self = cu_gnew(struct range_source_s);
    cu_ptr_source_init(cu_to(cu_ptr_source, self), range_source_get_ptr);
    self->cur = begin;
    self->end = end;
    return cu_to(cu_ptr_source, self);
}

cucon_listnode_t
cucon_list_append_list_dct(cucon_list_t lst, cucon_list_t dlst)
{
    cucon_listnode_t mid = dlst->eol.next;
    mid->prev = lst->eol.prev;
    mid->prev->next = mid;
    lst->eol.prev = dlst->eol.prev;
    lst->eol.prev->next = &lst->eol;
#ifndef CU_NDEBUG
    dlst->eol.next = NULL;
    dlst->eol.prev = NULL;
#endif
    return mid;
}

cucon_listnode_t
cucon_list_prepend_list_dct(cucon_list_t lst, cucon_list_t dlst)
{
    cucon_listnode_t mid = lst->eol.prev;
    mid->prev = dlst->eol.prev;
    mid->prev->next = mid;
    lst->eol.next = dlst->eol.next;
    lst->eol.next->prev = &lst->eol;
#ifndef CU_NDEBUG
    dlst->eol.next = NULL;
    dlst->eol.prev = NULL;
#endif
    return mid;
}

void
cuconP_list_init(void)
{
    cucon_list_init(&cuconP_list_empty);
}

