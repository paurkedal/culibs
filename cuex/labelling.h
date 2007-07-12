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

#ifndef CUEX_LABELLING_H
#define CUEX_LABELLING_H

#include <cuex/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_labelling_h cuex/labelling.h:
 *@{\ingroup cuex_mod */

extern cudyn_stdtype_t cuexP_labelling_type;
#define cuex_labelling_meta() cudyn_stdtype_to_meta(cuexP_labelling_type)
#define cuex_labelling_type() cuexP_labelling_type

/*!True iff \a e is a labelling. */
CU_SINLINE cu_bool_t
cuex_is_labelling(cuex_t e)
{ return cuex_meta(e) == cuex_labelling_meta(); }

/*!The empty labelling. */
cuex_t cuex_labelling_empty(void);

/*!A labelling with a single mapping from \a l to \a e. */
cuex_t cuex_labelling_singleton(cuex_t l, cuex_t e);

/*!Given an altering sequence of labels and values terminated by \c NULL,
 * returns the labelling of the corresponding mapping. */
cuex_t cuex_labelling_by_arglist(cuex_t l, cuex_t e, ...);

/*!The labelling \a L with the additional mapping from \a l to \a e.  If \a L
 * already contains the key \a l, then \a L is retured. */
cuex_t cuex_labelling_insert(cuex_t L, cuex_t l, cuex_t e);

/*!Assuming \a va refers to an argument list which contains a altering sequence
 * of pairs of labels and values terminated by \c NULL, returns the result of
 * inserting a mapping for each pair into \a L. */
cuex_t cuex_labelling_insert_valist(cuex_t L, va_list va);

/*!The variable arguments shall be an altering sequence of labels and values
 * terminated by \c NULL, and this function returns the result of inserting a
 * mapping for each (label, value) pair into \a L. */
cuex_t cuex_labelling_insert_arglist(cuex_t L, ...);

/*!Returns \a L with an extra mapping from \a l to \a e.  If \a l is already
 * present in \a L, the existing value \e v is replaced with
 * <tt>merge(\e v, \a e)</tt>. */
cuex_t cuex_labelling_deep_insert(cu_clop(merge, cuex_t, cuex_t, cuex_t),
				  cuex_t L, cuex_t l, cuex_t e);

/*!Returns the mapping of \a l in \a L, or \c NULL if none. */
cuex_t cuex_labelling_find(cuex_t L, cuex_t l);

/*!Forms the union of \a L0 and \a L1 considering elements equal if they have
 * the same label.  For elements present in both labellings, those from |a L0
 * are used in the result. */
cuex_t cuex_labelling_left_union(cuex_t L0, cuex_t L1);

/*!Forms the union of \a L0 and \a L1, merging the value part of common
 * elements with \a merge.  */
cuex_t cuex_labelling_deep_union(cu_clop(merge, cuex_t, cuex_t e0, cuex_t e1),
				 cuex_t L0, cuex_t L1);

/*!Forms the intersection of \a L0 and \a L1 considering elements equal if they
 * have the same label.  The elements of \a L0 are used in the result. */
cuex_t cuex_labelling_left_isecn(cuex_t L0, cuex_t L1);

/*!Forms the intersection of \a L0 and \a L1, merging the value part of the
 * elements with \a merge. */
cuex_t cuex_labelling_deep_isecn(cu_clop(merge, cuex_t, cuex_t e0, cuex_t e1),
				 cuex_t L0, cuex_t L1);

/*!Call \a f with each label and value of \a L as long as it returns true, and
 * return false iff an invocation of \a f returned false. */
cu_bool_t
cuex_labelling_conj_kv(cuex_t L, cu_clop(f, cu_bool_t, cuex_t l, cuex_t e));

/*!Return the labelling where each value of \a L is transformed by \a f, which
 * receives the old value and should return the new value.  If the label is
 * needed for the transform, see \ref cuex_labelling_image_kv. */
cuex_t
cuex_labelling_image(cuex_t L, cu_clop(f, cuex_t, cuex_t e));

/*!Return the labelling where each value of \a L is transformed by \a f, which
 * receives the label and the old value and should return the new value. */
cuex_t
cuex_labelling_image_kv(cuex_t L, cu_clop(f, cuex_t, cuex_t l, cuex_t e));

/*!@}*/
CU_END_DECLARATIONS

#endif
