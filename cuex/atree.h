/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_ATREE_H
#define CUEX_ATREE_H

#include <cuex/fwd.h>
#include <cu/algo.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_atree_h cuex/tree.h: Association from expression properties to full expression
 *@{\ingroup cuex_mod
 *
 * This is a low-level inferface for implementing semilattices, sets, maps, and
 * other collections of expressions which are keyed on a word computed form the
 * expressions.
 */

/*!An empty container. */
CU_SINLINE cuex_t cuex_atree_empty() { return NULL; }

/*!True iff \a tree is the empty container. */
CU_SINLINE cu_bool_t cuex_atree_is_empty(cuex_t tree) { return tree == NULL; }

/*!Find \a find_key in \a tree, assuming \a tree is keyed by \a get_key.
 * Returns the matching element, i.e. the element \e e such that
 * <tt>cu_call(get_key, e)</tt> equals \a find_key.  Returns \c NULL if no such
 * element exists.
 * \pre \a tree must be consistently keyed according to \a get_key. */
cuex_t cuex_atree_find(cu_clop(get_key, cu_word_t, cuex_t),
		       cuex_t tree, cu_word_t find_key);

/*!If the key of \a insert_value is not already in \a tree, returns the result
 * of inserting \a insert_value into \a tree, else returns \a tree.
 * \pre \a tree must be consistently keyed according to \a get_key. */
cuex_t cuex_atree_insert(cu_clop(get_key, cu_word_t, cuex_t),
			 cuex_t tree, cuex_t insert_value);

/*!If \a erase_key is in \a tree, returns \a tree with the value correspoding
 * to \a erase_key erased, otherwise returns \a tree. */
cuex_t cuex_atree_erase(cu_clop(get_key, cu_word_t, cuex_t),
			cuex_t tree, cu_word_t erase_key);

/*!Returns the union of \a tree0 and \a tree1, considering two elements equal
 * if the corresponding values returned by \a get_key are equal. */
cuex_t cuex_atree_union(cu_clop(get_key, cu_word_t, cuex_t),
			cuex_t tree0, cuex_t tree1);

/*!True iff \a tree0 ⊆ \a tree1 where elements are considered equal iff their
 * \a get_key values are equal. */
cu_bool_t cuex_atree_subseteq(cu_clop(get_key, cu_word_t, cuex_t),
			      cuex_t tree0, cuex_t tree1);

/*!Returns the ordering of \a tree0 and \a tree1, where elements are
 * considered equal iff their \a get_key values are equal. */
cu_order_t cuex_atree_order(cu_clop(get_key, cu_word_t, cuex_t),
			    cuex_t tree0, cuex_t tree1);

/*!Returns the intersection of \a tree0 and \a tree1, considering two
 * elements equal if the corresponding values returned by \a get_key are
 * equal. */
cuex_t cuex_atree_isecn(cu_clop(get_key, cu_word_t, cuex_t),
			cuex_t tree0, cuex_t tree1);

/*!Call \a fn with each value in \a tree. */
void cuex_atree_iter(cuex_t tree, cu_clop(fn, void, cuex_t));

/*!Returns the conjunction af \a fn applied to all keys in \a tree.  Stops as
 * soon as \a fn returns false. */
cu_bool_t cuex_atree_conj(cuex_t tree, cu_clop(fn, cu_bool_t, cuex_t));

/*!Returns the image of \a tree under \a fn.  Note that \a get_key is only
 * used for building the result, and may in fact be different from the \a
 * get_key which was used to build \a tree (thus its order in the argument
 * list).  If \a fn leaves the key of its argument unchanged, see \ref
 * cuex_atree_isokey_image for a faster alternative. */
cuex_t cuex_atree_image(cuex_t tree, cu_clop(fn, cuex_t, cuex_t),
			cu_clop(get_key, cu_word_t, cuex_t));

/*!Returns the image of \a tree under \a fn assuming \a fn does not change the
 * key of it's argument.  The key referred to is the result of the \e get_key
 * used to build the container \ref cuex_atree_insert, so the condition is that
 * \e get_key(\a fn(\e e)) = \e get_key(\e e) for all elements \e e in \a
 * tree.  If this condition is not met see \ref cuex_atree_image. */
cuex_t cuex_atree_isokey_image(cuex_t tree, cu_clop(fn, cuex_t, cuex_t));

/*!@}*/
CU_END_DECLARATIONS

#endif
