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
/** \defgroup cucon_list_h cucon/list.h: Doubly Linked Lists
 ** @{\ingroup cucon_linear_mod */

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

/** Create an empty list. */
void cucon_list_init(cucon_list_t);

/** Return an empty list. */
cucon_list_t cucon_list_new(void);

/** Create a copy of \a src using memcpy assuming slots are \a size bytes. */
void cucon_list_init_copy_mem(cucon_list_t dst, cucon_list_t src, size_t size);

/** \copydoc cucon_list_init_copy_mem */
cucon_list_t cucon_list_new_copy_mem(cucon_list_t src, size_t size);

/** Create a copy of \a src assuming elements are pointers. */
CU_SINLINE void cucon_list_init_copy_ptr(cucon_list_t dst, cucon_list_t src)
{ cucon_list_init_copy_mem(dst, src, sizeof(void *)); }

/** \copydoc cucon_list_init_copy_ptr */
CU_SINLINE cucon_list_t cucon_list_new_copy_ptr(cucon_list_t src)
{ return cucon_list_new_copy_mem(src, sizeof(void *)); }

/** Swap the elements of \a lst0 and \a lst1. */
void cucon_list_swap(cucon_list_t lst0, cucon_list_t lst1);

/** Return an empty list constant.  Don't change it! */
#define cucon_list_empty() (&cuconP_list_empty)

/** Clear the list. */
#define cucon_list_clear cucon_list_init

/** True iff \a list is empty. */
cu_bool_t cucon_list_is_empty(cucon_list_t list);

/** True iff \a list is empty or a singleton. */
cu_bool_t cucon_list_is_empty_or_singleton(cucon_list_t list);

/** True iff \a list has exactly one element. */
cu_bool_t cucon_list_is_singleton(cucon_list_t list);

/** Validate the integrity of the links of \a list. */
void cucon_list_validate(cucon_list_t list);

/** Return an integer less than, equal to, or greater than zero if \a lhs is
 ** less than, equal to, or greater than \a rhs, respectively, considering an
 ** element by element \c memcmp of the slots of \a size bytes.  If the lists
 ** are otherwise equal, the shorter one is considered to be the lesser. */
int cucon_list_cmp_mem(cucon_list_t lhs, cucon_list_t rhs, size_t slot_size);

/** Currently implemented as \ref cucon_list_cmp_mem, but may use pointer
 ** comparison in future versions. */
CU_SINLINE int cucon_list_cmp_ptr(cucon_list_t lhs, cucon_list_t rhs)
{ return cucon_list_cmp_mem(lhs, rhs, sizeof(void *)); }

/** Return the number of elements in \a list.  This takes linear time, use \c
 ** cucon_list_is_empty or \c cucon_list_is_singleton whenever possible. */
size_t cucon_list_count(cucon_list_t list);

/** Return the node containing a pointer \a ptr, or \c NULL if \a ptr is not in
 ** \a list.
 ** \pre All element slots of \a list must start with a pointer.  */
cucon_listnode_t cucon_list_find_ptr(cucon_list_t list, void *ptr);

/** Return an iterator to the first element of \a list. */
CU_SINLINE cucon_listnode_t cucon_list_begin(cucon_list_t list)
{ return list->eol.next; }

/** Return an iterater past the last element of \a list. */
CU_SINLINE cucon_listnode_t cucon_list_end(cucon_list_t list)
{ return &list->eol; }

/** An iterator to the first element of the reversed range of \a list. */
CU_SINLINE cucon_listnode_t cucon_list_rbegin(cucon_list_t list)
{ return list->eol.prev; }

/** An iterator to post the end of the reversed range of \a list. */
CU_SINLINE cucon_listnode_t cucon_list_rend(cucon_list_t list)
{ return &list->eol; }

/** Construct \a new_node, inserting it before \a pos. */
void cucon_list_insert_init(cucon_listnode_t pos, cucon_listnode_t new_node);

/** Extract the live \a node from its list and insert it before \a pos. */
void cucon_list_insert_live(cucon_listnode_t pos, cucon_listnode_t node);

/** Insert a new node with \a size bytes value before \a node. */
cucon_listnode_t cucon_list_insert_mem(cucon_listnode_t node, size_t size);

/** Insert a new node before \a node, which holds the pointer \a ptr. */
cucon_listnode_t cucon_list_insert_ptr(cucon_listnode_t node, void *ptr);

/** Remove \a node from its list and return it as an isolated node. */
cucon_listnode_t cucon_list_extract(cucon_listnode_t node);

/** Return a pointer to the value slot of \a node after unlinking it from its
 ** list.  */
void *cucon_list_extract_mem(cucon_listnode_t node);

/** Return the pointer stored as the value of \a node after unlinking it from
 ** its list. */
void *cucon_list_extract_ptr(cucon_listnode_t node);

/** Erase element pointed to by \a node from its list and return an iterator to
 ** the next element. */
cucon_listnode_t cucon_list_erase_node(cucon_listnode_t node);

/** Erase the first element from \a list which equals \a ptr and return an
 ** iterator to the element after it, or end-of-list if nothing was erased.
 ** All elements of \a list must start with a pointer.  If you need to know if
 ** an element was actually erased, use \c cucon_list_find_ptr and \c
 ** cucon_list_erase_node. */
cucon_listnode_t cucon_list_erase_ptr(cucon_list_t list, void *ptr);

/** Erase all elements of \a list which equals \a ptr and return the number of
 ** elements erased. */
size_t cucon_list_erase_all_ptr(cucon_list_t list, void *ptr);

/** Rotate elements of \a list so that \a first_node becomes the first node.
 ** \pre \a first_node must be an element of \a list other than EOL. */
void cucon_list_rotate(cucon_list_t list, cucon_listnode_t first_node);

/** Move the first element of the \a list to the end. */
void cucon_list_rotate_backwards(cucon_list_t list);

/** Return a pointer to the value of \a node. */
CU_SINLINE void *cucon_listnode_mem(cucon_listnode_t node)
{ return CU_ALIGNED_MARG_END(cucon_listnode_t, node); }

/** Return the pointer stored as the value of \a node. */
CU_SINLINE void *cucon_listnode_ptr(cucon_listnode_t node)
{ return (*(void**)CU_ALIGNED_MARG_END(cucon_listnode_t, node)); }

/** Set the value of \a node to the pointer \a ptr.
 ** \pre The value slot of \a node must be large enough to hold the pointer. */
CU_SINLINE void cucon_listnode_set_ptr(cucon_listnode_t node, void *ptr)
{ *(void**)CU_ALIGNED_MARG_END(cucon_listnode_t, node) = ptr; }

/** Return an iterator to the element after that referred by \a node. */
CU_SINLINE cucon_listnode_t cucon_listnode_next(cucon_listnode_t node)
{ return CU_MARG(cucon_listnode_t, node)->next; }

/** Return an iterator to the element before that referred by \a node. */
CU_SINLINE cucon_listnode_t cucon_listnode_prev(cucon_listnode_t node)
{ return CU_MARG(cucon_listnode_t, node)->prev; }

/** Call \a proc with all values in the iterator range. */
void cucon_list_range_iter_mem(cucon_listnode_t, cucon_listnode_t,
			       cu_clop(proc, void, void *));

/** A source over values from \a begin up to but not including \a end where
 ** value slot pointers are dereferenced as pointers. */
cu_ptr_source_t cucon_listrange_source_ptr(cucon_listnode_t begin,
					   cucon_listnode_t end);

/** A source over all elements of \a list dereferenced as pointers. */
CU_SINLINE cu_ptr_source_t cucon_list_source_ptr(cucon_list_t list)
{ return cucon_listrange_source_ptr(cucon_list_begin(list),
				    cucon_list_end(list)); }

/** Insert and construct \a node as first element of \a list. */
CU_SINLINE void
cucon_list_prepend_init(cucon_list_t list, cucon_listnode_t node)
{ cucon_list_insert_init(cucon_list_begin(list), node); }

/** Move \a node from its list to the front of \a list. */
CU_SINLINE void
cucon_list_prepend_live(cucon_list_t list, cucon_listnode_t node)
{ cucon_list_insert_live(cucon_list_begin(list), node); }

/** Insert an element as the first in the list. */
CU_SINLINE cucon_listnode_t
cucon_list_prepend_mem(cucon_list_t list, size_t size)
{ return cucon_list_insert_mem(cucon_list_begin(list), size); }

/** Insert a pointer as the first in the list. */
CU_SINLINE cucon_listnode_t
cucon_list_prepend_ptr(cucon_list_t list, void *p)
{ return cucon_list_insert_ptr(cucon_list_begin(list), p); }

/** Insert and construct \a node at end of \a list. */
CU_SINLINE void
cucon_list_append_init(cucon_list_t list, cucon_listnode_t node)
{ cucon_list_insert_init(cucon_list_end(list), node); }

/** Move a live \a node to the back of \a list. */
CU_SINLINE void
cucon_list_append_live(cucon_list_t list, cucon_listnode_t node)
{ cucon_list_insert_live(cucon_list_end(list), node); }

/** Insert an element as the last in the list. */
CU_SINLINE cucon_listnode_t
cucon_list_append_mem(cucon_list_t list, size_t size)
{ return cucon_list_insert_mem(cucon_list_end(list), size); }

/** Insert a pointer as the last in the list. */
CU_SINLINE cucon_listnode_t
cucon_list_append_ptr(cucon_list_t list, void *p)
{ return cucon_list_insert_ptr(cucon_list_end(list), p); }

/** Append \a src to \a dst, descructing \a src. */
cucon_listnode_t
cucon_list_append_list_dct(cucon_list_t dst, cucon_list_t src);

/** Prepend \a src to \a dst, descructing \a src. */
cucon_listnode_t
cucon_list_prepend_list_dct(cucon_list_t dst, cucon_list_t src);

/** A pointer to the slot of the first element of \a list.
 ** \pre \a list is non-empty. */
CU_SINLINE void *cucon_list_front_mem(cucon_list_t list)
{ return cucon_listnode_mem(cucon_list_begin(list)); }

/** A pointer to the slot of the last element of \a list.
 ** \pre \a list is non-empty. */
CU_SINLINE void *cucon_list_back_mem(cucon_list_t list)
{ return cucon_listnode_mem(cucon_list_rbegin(list)); }

/** The value of the first element of \a list, assuming it is a pointer.
 ** \pre \a list is non-empty. */
CU_SINLINE void *cucon_list_front_ptr(cucon_list_t list)
{ return *(void **)cucon_list_front_mem(list); }

/** The value of the last element of \a list, assuming it is a pointer.
 ** \pre \a list is non-empty. */
CU_SINLINE void *cucon_list_back_ptr(cucon_list_t list)
{ return *(void **)cucon_list_back_mem(list); }

/** Erase the first element of \a list.
 ** \pre \a list is non-empty. */
CU_SINLINE void cucon_list_pop_front(cucon_list_t list)
{ cucon_list_erase_node(cucon_list_begin(list)); }

/** Erase the last element of \a list.
 ** \pre \a list is non-empty. */
CU_SINLINE void cucon_list_pop_back(cucon_list_t list)
{ cucon_list_erase_node(cucon_list_end(list)); }

/** @} */

/* Deprecated 2009-09-19. */
#define cucon_list_insert_it		cucon_list_insert_live
#define cucon_list_insert_init_node	cucon_list_insert_init
#define cucon_list_prepend_init_node	cucon_list_prepend_init
#define cucon_list_append_init_node	cucon_list_append_init

CU_END_DECLARATIONS

#endif
