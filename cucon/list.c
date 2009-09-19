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
#include <string.h>

struct cucon_list_s cuconP_list_empty;

void
cucon_list_init(cucon_list_t list)
{
    list->eol.next = &list->eol;
    list->eol.prev = &list->eol;
}

void
cucon_list_init_copy_mem(cucon_list_t dst, cucon_list_t src, size_t size)
{
    cucon_listnode_t src_node, prev_dst_node;
    cucon_list_init(dst);
    prev_dst_node = &dst->eol;
    for (src_node = cucon_list_begin(src); src_node != cucon_list_end(src);
	 src_node = cucon_listnode_next(src_node)) {
	cucon_listnode_t dst_node;
	dst_node = cu_galloc(sizeof(struct cucon_listnode_s) + size);
	memcpy(cucon_listnode_mem(dst_node),
	       cucon_listnode_mem(src_node), size);
	dst_node->prev = prev_dst_node;
	prev_dst_node->next = dst_node;
	prev_dst_node = dst_node;
    }
    dst->eol.prev = prev_dst_node;
    prev_dst_node->next = &dst->eol;
}

cucon_list_t
cucon_list_new(void)
{
    cucon_list_t lst = cu_gnew(struct cucon_list_s);
    cucon_list_init(lst);
    return lst;
}

cucon_list_t
cucon_list_new_copy_mem(cucon_list_t src, size_t size)
{
    cucon_list_t dst = cu_gnew(struct cucon_list_s);
    cucon_list_init_copy_mem(dst, src, size);
    return dst;
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

void
cucon_list_validate(cucon_list_t L)
{
    cucon_listnode_t eol = &L->eol;
    cucon_listnode_t prev = eol;
    cucon_listnode_t node = eol->next;
    do {
	cu_debug_assert(node->prev == prev);
	prev = node;
	node = node->next;
    } while (prev != eol);
}

int
cucon_list_cmp_mem(cucon_list_t l0, cucon_list_t l1, size_t size)
{
    cucon_listnode_t n0 = cucon_list_begin(l0);
    cucon_listnode_t n1 = cucon_list_begin(l1);
    for (;;) {
	void *p0, *p1;
	int order;
	int cont0 = n0 != cucon_list_end(l0);
	int cont1 = n1 != cucon_list_end(l1);
	if (!cont0 || !cont1)
	    return cont0 - cont1;
	p0 = cucon_listnode_ptr(n0);
	p1 = cucon_listnode_ptr(n1);
	order = memcmp(cucon_listnode_mem(n0), cucon_listnode_mem(n1), size);
	if (order != 0)
	    return order;
	n0 = cucon_listnode_next(n0);
	n1 = cucon_listnode_next(n1);
    }
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
cucon_list_find_ptr(cucon_list_t L, void *ptr)
{
    cucon_listnode_t node;
    for (node = cucon_list_begin(L);
	 node != cucon_list_end(L); node = cucon_listnode_next(node))
	if (cucon_listnode_ptr(node) == ptr)
	    return node;
    return NULL;
}

void
cucon_list_insert_init(cucon_listnode_t pos, cucon_listnode_t node)
{
    node->next = pos;
    node->prev = pos->prev;
    node->prev->next = node;
    pos->prev = node;
}

void
cucon_list_insert_live(cucon_listnode_t pos, cucon_listnode_t node)
{
    /* Erase. */
    node->next->prev = node->prev;
    node->prev->next = node->next;
    /* Re-insert. */
    node->next = pos;
    node->prev = pos->prev;
    node->prev->next = node;
    pos->prev = node;
}

cucon_listnode_t
cucon_list_insert_mem(cucon_listnode_t pos, size_t size)
{
    struct cucon_listnode_s *node
	= cu_galloc(sizeof(struct cucon_listnode_s) + size);
    node->next = pos;
    node->prev = pos->prev;
    pos->prev = node;
    node->prev->next = node;
    return node;
}

cucon_listnode_t
cucon_list_insert_ptr(cucon_listnode_t pos, void *ptr)
{
    cucon_listnode_t node;
    node = cu_galloc(sizeof(struct cucon_listnode_s) + sizeof(void *));
    node->next = pos;
    node->prev = pos->prev;
    pos->prev = node;
    node->prev->next = node;
    cucon_listnode_set_ptr(node, ptr);
    return node;
}

cucon_listnode_t
cucon_list_extract(cucon_listnode_t node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->prev = node->next = NULL;
    return node;
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
cucon_list_rotate(cucon_list_t L, cucon_listnode_t first_node)
{
    if (cucon_list_begin(L) == first_node)
	return;
    cu_debug_assert(first_node != &L->eol);
    L->eol.prev->next = L->eol.next;
    L->eol.next->prev = L->eol.prev;
    L->eol.next = first_node;
    L->eol.prev = first_node->prev;
    first_node->prev = &L->eol;
    L->eol.prev->next = &L->eol;
}

void
cucon_list_rotate_forwards(cucon_list_t L)
{
    if (cucon_list_is_empty(L))
	return;
    cucon_list_rotate(L, cucon_list_rend(L));
}

void
cucon_list_rotate_backwards(cucon_list_t L)
{
    if (cucon_list_is_empty_or_singleton(L))
	return;
    cucon_list_rotate(L, cucon_listnode_next(cucon_list_begin(L)));
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

typedef struct _range_source_s *_range_source_t;
struct _range_source_s
{
    cu_inherit (cu_ptr_source_s);
    cucon_listnode_t cur;
    cucon_listnode_t end;
};

static void *
_range_source_get_ptr(cu_ptr_source_t source)
{
    _range_source_t self = cu_from(_range_source, cu_ptr_source, source);
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
    _range_source_t self = cu_gnew(struct _range_source_s);
    cu_ptr_source_init(cu_to(cu_ptr_source, self), _range_source_get_ptr);
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

