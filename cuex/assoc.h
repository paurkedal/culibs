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

#ifndef CUEX_ASSOC_H
#define CUEX_ASSOC_H

#include <cuex/fwd.h>
#include <cu/algo.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_assoc_h cuex/assoc.h: Association from expression properties to full expression
 *@{\ingroup cuex_mod
 *
 * This is a low-level inferface for implementing semilattices, sets, maps, and
 * other collections of expressions which are keyed on a word computed form the
 * expressions.
 */

extern cuex_t cuexP_assoc_empty;

/*!An empty container. */
CU_SINLINE cuex_t
cuex_assoc_empty()
{ return cuexP_assoc_empty; }

/*!True iff \a assoc is the empty container. */
CU_SINLINE cu_bool_t
cuex_assoc_is_empty(cuex_t assoc)
{
    return assoc == cuexP_assoc_empty;
}

/*!Find \a find_key in \a assoc, assuming \a assoc is keyed by \a get_key.
 * Returns the matching element, i.e. the element \e e such that
 * <tt>cu_call(get_key, e)</tt> equals \a find_key.  Returns \c NULL if no such
 * element exists.
 * \pre \a assoc must be consistently keyed according to \a get_key. */
cuex_t cuex_assoc_find(cu_clop(get_key, cu_word_t, cuex_t),
		       cuex_t assoc, cu_word_t find_key);

/*!If the key of \a insert_value is not already in \a assoc, returns the result
 * of inserting \a insert_value into \a assoc, else returns \a assoc.
 * \pre \a assoc must be consistently keyed according to \a get_key. */
cuex_t cuex_assoc_insert(cu_clop(get_key, cu_word_t, cuex_t),
			 cuex_t assoc, cuex_t insert_value);

/*!If \a erase_key is in \a assoc, returns \a assoc with the value correspoding
 * to \a erase_key erased, otherwise returns \a assoc. */
cuex_t cuex_assoc_erase(cu_clop(get_key, cu_word_t, cuex_t),
			cuex_t assoc, cu_word_t erase_key);

/*!Returns the union of \a assoc0 and \a assoc1, considering two elements equal
 * if the corresponding values returned by \a get_key are equal. */
cuex_t cuex_assoc_union(cu_clop(get_key, cu_word_t, cuex_t),
			cuex_t assoc0, cuex_t assoc1);

/*!True iff \a assoc0 ⊆ \a assoc1 where elements are considered equal iff their
 * \a get_key values are equal. */
cu_bool_t cuex_assoc_subseteq(cu_clop(get_key, cu_word_t, cuex_t),
			      cuex_t assoc0, cuex_t assoc1);

/*!Returns the ordering of \a assoc0 and \a assoc1, where elements are
 * considered equal iff their \a get_key values are equal. */
cu_order_t cuex_assoc_order(cu_clop(get_key, cu_word_t, cuex_t),
			    cuex_t assoc0, cuex_t assoc1);

/*!Returns the intersection of \a assoc0 and \a assoc1, considering two
 * elements equal if the corresponding values returned by \a get_key are
 * equal. */
cuex_t cuex_assoc_isecn(cu_clop(get_key, cu_word_t, cuex_t),
			cuex_t assoc0, cuex_t assoc1);

/*!Call \a fn with each value in \a assoc. */
void cuex_assoc_iter(cuex_t assoc, cu_clop(fn, void, cuex_t));

/*!Returns the conjunction af \a fn applied to all keys in \a assoc.  Stops as
 * soon as \a fn returns false. */
cu_bool_t cuex_assoc_conj(cuex_t assoc, cu_clop(fn, cu_bool_t, cuex_t));

/*!Returns the image of \a assoc under \a fn.  Note that \a get_key is only
 * used for building the result, and may in fact be different from the \a
 * get_key which was used to build \a assoc (thus its order in the argument
 * list).  If \a fn leaves the key of its argument unchanged, see \ref
 * cuex_assoc_isokey_image for a faster alternative. */
cuex_t cuex_assoc_image(cuex_t assoc, cu_clop(fn, cuex_t, cuex_t),
			cu_clop(get_key, cu_word_t, cuex_t));

/*!Returns the image of \a assoc under \a fn assuming \a fn does not change the
 * key of it's argument.  The key referred to is the result of the \e get_key
 * used to build the container \ref cuex_assoc_insert, so the condition is that
 * \e get_key(\a fn(\e e)) = \e get_key(\e e) for all elements \e e in \a
 * assoc.  If this condition is not met see \ref cuex_assoc_image. */
cuex_t cuex_assoc_isokey_image(cuex_t assoc, cu_clop(fn, cuex_t, cuex_t));

/*!@}*/
CU_END_DECLARATIONS

#endif
