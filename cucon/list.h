/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2000--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_LIST_H
#define CUCON_LIST_H

#include <cu/clos.h>
#include <cu/util.h>
#include <cucon/fwd.h>
#include <stddef.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_list_h cucon/list.h: Doubly Linked Lists
 * @{\ingroup cucon_linear_mod */

struct cucon_listnode_s
{
    struct cucon_listnode_s* prev;
    struct cucon_listnode_s* next;
};
struct cucon_list_s
{
    struct cucon_listnode_s eol;
};

extern struct cucon_list_s cuconP_list_empty;

/*!Return an empty list. */
cucon_list_t cucon_list_new(void);

/*!Create an empty list. */
void cucon_list_init(cucon_list_t);

/*!Create a copy of \a src assuming elements are pointers. */
void cucon_list_init_copy_ptr(cucon_list_t dst, cucon_list_t src);

/*!Swap the elements of \a lst0 and \a lst1. */
void cucon_list_swap(cucon_list_t lst0, cucon_list_t lst1);

/*!Return an empty list constant.  Don't change it! */
#define cucon_list_empty() (&cuconP_list_empty)

/*!Clear the list. */
#define cucon_list_clear cucon_list_init

/*!True iff \a lst is empty. */
cu_bool_t cucon_list_is_empty(cucon_list_t lst);
/*!True iff \a lst is empty or a singleton. */
cu_bool_t cucon_list_is_empty_or_singleton(cucon_list_t lst);
/*!True iff \a lst has exactly one element. */
cu_bool_t cucon_list_is_singleton(cucon_list_t lst);

/*!Return the number of elements in \a lst.  NB: linear complexity, use
 * \c cucon_list_is_empty or \c cucon_list_is_singleton whenever possible. */
size_t cucon_list_count(cucon_list_t lst);

/*!Return an iterator to the first element of \a lst. */
CU_SINLINE cucon_listnode_t cucon_list_begin(cucon_list_t lst)
{ return lst->eol.next; }

/*!Return an iterater past the last element of \a lst. */
CU_SINLINE cucon_listnode_t cucon_list_end(cucon_list_t lst)
{ return &lst->eol; }

/*!An iterator to the first element of the reversed range of \a lst. */
CU_SINLINE cucon_listnode_t cucon_list_rbegin(cucon_list_t lst)
{ return lst->eol.prev; }

/*!An iterator to post the end of the reversed range of \a lst. */
CU_SINLINE cucon_listnode_t cucon_list_rend(cucon_list_t lst)
{ return &lst->eol; }

/*!Erase \a it1 from its list and insert it before \a it0. */
cucon_listnode_t
cucon_list_insert_it(cucon_listnode_t it0, cucon_listnode_t it1);

/*!Insert an element with \a size bytes value before \a it. */
cucon_listnode_t
cucon_list_insert_mem(cucon_listnode_t it, size_t size);

/*!Insert a pointer element before \a it. */
cucon_listnode_t
cucon_list_insert_ptr(cucon_listnode_t, void*);

/*!Construct \a node, inserting it before \a it. */
cucon_listnode_t
cucon_list_insert_init_node(cucon_listnode_t it, cucon_listnode_t node);

/*!Return a pointer to the value after unlinking the element from its
 * list. */
void *cucon_list_extract_mem(cucon_listnode_t);

/*!Return the pointer stored as the value of \a it after unlinking the
 * element from its list. */
void *cucon_list_extract_ptr(cucon_listnode_t it);

/*!Erase element pointed to by \a it from its list and return an iterator
 * to the next element. */
cucon_listnode_t cucon_list_erase_node(cucon_listnode_t it);

/*!Return the iterator which points to the pointer \a ptr, or the EOL
 * iterator if \a ptr is not in \a lst.  All elements of \a lst must
 * start with a pointer.  */
cucon_listnode_t cucon_list_find_ptr(cucon_list_t lst, void *ptr);

/*!Erase the first element from \a lst which equals \a ptr and return an
 * iterator to the element after it, or end-of-list if nothing was erased.
 * All elements of \a lst must start with a pointer.  If you need to
 * know if an element was actually erased, use \c cucon_list_find_ptr and
 * \c cucon_list_erase_node. */
cucon_listnode_t cucon_list_erase_ptr(cucon_list_t lst, void *ptr);

/*!Erase all elements of \a lst which equals \a ptr and return the
 * number of elements erased. */
size_t cucon_list_erase_all_ptr(cucon_list_t lst, void *ptr);

/*!Move the first element of the \a lst to the end. */
void cucon_list_rotate_backwards(cucon_list_t lst);

/*!Return a pointer to the value of \a it. */
CU_SINLINE void *cucon_listnode_mem(cucon_listnode_t it)
{ return CU_ALIGNED_MARG_END(cucon_listnode_t, it); }

/*!Return the pointer stored as the value of \a it. */
CU_SINLINE void *cucon_listnode_ptr(cucon_listnode_t it)
{ return (*(void**)CU_ALIGNED_MARG_END(cucon_listnode_t, it)); }

/*!Set the pointer stored at the iterator. */
CU_SINLINE void cucon_listnode_set_ptr(cucon_listnode_t it, void *ptr)
{ *(void**)CU_ALIGNED_MARG_END(cucon_listnode_t, it) = ptr; }

/*!Return an iterator to the element after that referred by \a it. */
CU_SINLINE cucon_listnode_t cucon_listnode_next(cucon_listnode_t it)
{ return CU_MARG(cucon_listnode_t, it)->next; }

/*!Return an iterator to the element before that referred by \a it. */
CU_SINLINE cucon_listnode_t cucon_listnode_prev(cucon_listnode_t it)
{ return CU_MARG(cucon_listnode_t, it)->prev; }

/*!Call \a proc with all values in the iterator range. */
void cucon_list_range_iter_mem(cucon_listnode_t, cucon_listnode_t,
			       cu_clop(proc, void, void *));

/*!A source over values from \a begin up to but not including \a end where
 * value slot pointers are dereferenced as pointers. */
cu_ptr_source_t cucon_listrange_source_ptr(cucon_listnode_t begin,
					   cucon_listnode_t end);

/*!A source over all elements of \a list dereferenced as pointers. */
CU_SINLINE cu_ptr_source_t cucon_list_source_ptr(cucon_list_t list)
{ return cucon_listrange_source_ptr(cucon_list_begin(list),
				    cucon_list_end(list)); }

/*!Insert an element as the first in the list. */
CU_SINLINE cucon_listnode_t
cucon_list_prepend_mem(cucon_list_t list, size_t size)
{ return cucon_list_insert_mem(cucon_list_begin(list), size); }

/*!Insert a pointer as the first in the list. */
CU_SINLINE cucon_listnode_t
cucon_list_prepend_ptr(cucon_list_t list, void *p)
{ return cucon_list_insert_ptr(cucon_list_begin(list), p); }

/*!Insert and construct \a node as first element of \a list. */
CU_SINLINE cucon_listnode_t
cucon_list_prepend_init_node(cucon_list_t list, cucon_listnode_t node)
{ return cucon_list_insert_init_node(cucon_list_begin(list), node); }

/*!Insert an element as the last in the list. */
CU_SINLINE cucon_listnode_t
cucon_list_append_mem(cucon_list_t list, size_t size)
{ return cucon_list_insert_mem(cucon_list_end(list), (size)); }

/*!Insert a pointer as the last in the list. */
CU_SINLINE cucon_listnode_t
cucon_list_append_ptr(cucon_list_t list, void *p)
{ return cucon_list_insert_ptr(cucon_list_end(list), p); }

/*!Insert and construct \a node at end of \a list. */
CU_SINLINE cucon_listnode_t
cucon_list_append_init_node(cucon_list_t list, cucon_listnode_t node)
{ return cucon_list_insert_init_node(cucon_list_end(list), node); }

/*!Append \a src to \a dst, descructing \a src. */
cucon_listnode_t
cucon_list_append_list_dct(cucon_list_t dst, cucon_list_t src);

/*!Prepend \a src to \a dst, descructing \a src. */
cucon_listnode_t
cucon_list_prepend_list_dct(cucon_list_t dst, cucon_list_t src);

/*!A pointer to the slot of the first element of \a list.
 * \pre \a list is non-empty. */
#define cucon_list_front_mem(list) \
	cucon_listnode_mem(cucon_list_begin(list))
/*!A pointer to the slot of the last element of \a list.
 * \pre \a list is non-empty. */
#define cucon_list_back_mem(list) \
	cucon_listnode_mem(cucon_list_end(list))
/*!The value of the first element of \a list, assuming it is a pointer.
 * \pre \a list is non-empty. */
#define cucon_list_front_ptr(list) \
	cucon_listnode_ptr(cucon_list_begin(list))
/*!The value of the last element of \a list, assuming it is a pointer.
 * \pre \a list is non-empty. */
#define cucon_list_back_ptr(list) \
	cucon_listnode_ptr(cucon_list_end(list))

/*!Erase the first element of \a list.
 * \pre \a list is non-empty. */
CU_SINLINE void cucon_list_pop_front(cucon_list_t list)
{ cucon_list_erase_node(cucon_list_begin(list)); }

/*!Erase the last element of \a list.
 * \pre \a list is non-empty. */
CU_SINLINE void cucon_list_pop_back(cucon_list_t list)
{ cucon_list_erase_node(cucon_list_end(list)); }


/*!\deprecated Use cucon_list_erase_node */
#define cucon_list_it_erase cucon_list_erase_node
/*!\deprecated Use cucon_listnode_ptr */
#define cucon_list_it_get_ptr cucon_listnode_ptr
/*!\deprecated Use cucon_listnode_mem */
#define cucon_list_it_get_mem cucon_listnode_mem
/*!\deprecated Use cucon_listnode_set_ptr */
#define cucon_list_it_set_ptr cucon_listnode_set_ptr
/*!\deprecated Use cucon_listnode_next */
#define cucon_list_it_next cucon_listnode_next
/*!\deprecated Use cucon_listnode_prev */
#define cucon_list_it_prev cucon_listnode_prev
/*!\deprecated Use cucon_listnode_prev */
#define cucon_list_it_rnext cucon_listnode_prev
/*!\deprecated Use cucon_listnode_next */
#define cucon_list_it_rprev cucon_listnode_next
/*!\deprecated Use cucon_listnode_s */
#define cucon_list_node_s cucon_listnode_s
/*!\deprecated Use cucon_list_is_singleton */
#define cucon_list_is_singular cucon_list_is_singleton
/*!\deprecated Use cucon_list_is_empty_or_singleton */
#define cucon_list_is_empty_or_singular cucon_list_is_empty_or_singleton
/*!\deprecated Use \ref cucon_list_init. */
#define cucon_list_cct cucon_list_init
/*!\deprecated Use \ref cucon_list_init_copy_ptr. */
#define cucon_list_cct_copy_ptr cucon_list_init_copy_ptr
/*!\deprecated Use \ref cucon_list_insert_init_node. */
#define cucon_list_insert_node_cct cucon_list_insert_init_node
/*!\deprecated Use \ref cucon_list_prepend_init_node. */
#define cucon_list_prepend_node_cct cucon_list_prepend_init_node
/*!\deprecated Use \ref cucon_list_append_init_node. */
#define cucon_list_append_node_cct cucon_list_append_init_node

/*!@}*/
CU_END_DECLARATIONS

#endif
