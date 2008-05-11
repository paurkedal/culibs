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
#include <cuoo/meta.h>
#include <cuoo/type.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_labelling_h cuex/labelling.h: Association from Constants to Expressions
 *@{\ingroup cuex_mod */

extern cuoo_stdtype_t cuexP_labelling_type;
#define cuex_labelling_meta() cuoo_stdtype_to_meta(cuexP_labelling_type)
#define cuex_labelling_type() cuoo_stdtype_to_type(cuexP_labelling_type)

/*!True iff \a e is a labelling. */
CU_SINLINE cu_bool_t
cuex_is_labelling(cuex_t e)
{ return cuex_meta(e) == cuex_labelling_meta(); }

extern cuex_t cuexP_labelling_empty;

/*!The empty labelling. */
CU_SINLINE cuex_t cuex_labelling_empty(void) { return cuexP_labelling_empty; }

/*!True iff \a e is the empty labelling. */
CU_SINLINE cu_bool_t cuex_is_labelling_empty(cuex_t e)
{ return e == cuexP_labelling_empty; }

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

/*!Returns the result of erasing the mapping from \a l in \a L if present,
 * otherwise returs \a L. */
cuex_t cuex_labelling_erase(cuex_t L, cuex_t l);

/*!If \a l has a mapping if \c *\a L, updates \c *\a L by erasing it, and
 * returns the value of the mapping, otherwise returns \c NULL. */
cuex_t cuex_labelling_find_erase(cuex_t *L, cuex_t l);

/*!Forms the union of \a L0 and \a L1 considering elements equal if they have
 * the same label.  For elements present in both labellings, those from |a L0
 * are used in the result. */
cuex_t cuex_labelling_left_union(cuex_t L0, cuex_t L1);

/*!Forms the union of \a L0 and \a L1, merging the value part of common
 * elements with \a merge.  */
cuex_t cuex_labelling_deep_union(cu_clop(merge, cuex_t, cuex_t e0, cuex_t e1),
				 cuex_t L0, cuex_t L1);

/*!Returns the union of \a L0 and \a L1 if they are disjoint, \c NULL
 * otherwise.  ("Disjoint union" is not used in the sense of "discriminated
 * union" here.) */
cuex_t cuex_labelling_disjoint_union(cuex_t L0, cuex_t L1);

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

cuex_t cuex_labelling_expand_all(cuex_t L);
cuex_t cuex_labelling_contract_all(cuex_t L);

/*!Returns an iteration source for the unordered sequence.  The implied
 * sequence are \c CUEX_O2_METAPAIR nodes.
 * \pre L is a labelling; check with \ref cuex_is_labelling */
cu_ptr_source_t cuex_labelling_comm_iter_source(cuex_t L);

/*!Returns an iteration source for an ordered sequence of all values mapped by
 * \a L.  The ordering is given by arbitrary fixing the ordering of the
 * labels.  The labels are not exposed.
 * \pre L is a labelling; check with \ref cuex_is_labelling
 * \see cuex_labelling_comm_iter_source */
cu_ptr_source_t cuex_labelling_ncomm_iter_source(cuex_t L);

/*!A junctor for constructing the image of \a L, using the ordered view
 * corresponding to \a cuex_labelling_comm_iter_source.  For each element
 * popped from the junctor, at most one must be put back.  The labels of the
 * resulting labelling is taken from the original. */
cu_ptr_junctor_t cuex_labelling_ncomm_image_junctor(cuex_t L);

/*!A sinktor for construction a new labelling by unordered insertion of \c
 * CUEX_O2_METAPAIR nodes and labellings. */
cu_ptr_sinktor_t cuex_labelling_comm_build_sinktor(void);

/*!A sinktor for extending a labelling by unordered insertion of \c
 * CUEX_O2_METAPAIR nodes and labellings. */
cu_ptr_sinktor_t cuex_labelling_comm_union_sinktor(cuex_t L);

/*!@}*/
CU_END_DECLARATIONS

#endif
