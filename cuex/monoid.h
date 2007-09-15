/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_MONOID_H
#define CUEX_MONOID_H

#include <cuex/fwd.h>
#include <cuex/opn.h>
#include <cuex/oprdefs.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_monoid_h cuex/monoid.h: Expressions of Associative Operators with Identity
 * @{\ingroup cuex_mod
 * Create and manipulate expressions trees of associative operators.
 * These corresponds to the operators of monoids, though since we
 * are working with raw expressions, the set of the monoid is not explicit.
 */

/* Representation
 *     operators must be defined with aux = 0 and r = 0
 *     identity element:	aux = 1 and r = 0
 *     non-trivial nodes:	aux ≥ 1 and r = FANOUT
 */

#define CUEX_MONOID_OPR_MASK (CUEX_METAKIND_MASK | CUEX_OPR_SELECT_MASK)
#define CUEX_MONOID_MAXPL_DEPTH (1 << CUEX_OA_MONOID_DEPTH_WIDTH)

#define cuexP_monoid_opr(opr, depth, arity) \
    ((opr) | CUEX_OA_MONOID_DEPTH(depth) | ((arity) << CUEX_OPR_ARITY_SHIFT))

/*!Internal use. The depth part of \a meta. */
#define cuexP_monoid_meta_depth(meta) cuex_oa_monoid_depth(meta)

/*!The operator part of \a meta which should me the meta of a monoid
 * operation. */
#define cuex_monoid_meta_opr(meta) \
    cuex_opr_sans_arity(cuex_og_monoid_strip(meta))

/*!True iff \a x is a non-generator element of the monoid induced by
 * \a mult, including the identity of \a mult. */
CU_SINLINE cu_bool_t cuex_is_monoid_product(cuex_meta_t mult, cuex_t x)
{ return cuex_monoid_meta_opr(cuex_meta(x)) == mult; }

/*!The identity element of the monoid induced by \a mult. */
CU_SINLINE cuex_opn_t cuex_monoid_identity(cuex_meta_t mult)
{ return cuex_opn(mult | CUEX_OA_MONOID_DEPTH(1)); }

/*!True iff \a x is the identity of the monoid operator \a mult. */
CU_SINLINE cu_bool_t cuex_is_monoid_identity(cuex_meta_t mult, cuex_t x)
{ return cuex_meta(x) == (mult | CUEX_OA_MONOID_DEPTH(1)); }

/*!Returns \a x * \a y, where * = \a mult. */
cuex_opn_t cuex_monoid_product(cuex_meta_t mult, cuex_t x, cuex_t y);

/*!True iff \a meta is the meta of a monoid operation.  Note that this
 * is false for monoid operators as they are defined, but true for the
 * meta of operations which are created with \ref cuex_monoid_identity
 * and \ref cuex_monoid_product and which do not reduce to generators. */
CU_SINLINE cu_bool_t cuex_is_monoid_meta(cuex_meta_t meta)
{ return cuex_og_monoid_contains(meta); }

/*!The number of generators in the factorisation of \a x with respect to
 * the operator \a mult. */
size_t cuex_monoid_factor_cnt(cuex_meta_t mult, cuex_t x);

/*!Factor number \a i (counting from 0) of the factorisation of \a x
 * with respect to \a mult.
 * \pre \a i ≤ \ref cuex_monoid_factor_cnt(\a mult, \a x) */
cuex_t cuex_monoid_factor_at(cuex_meta_t mult, cuex_t x, size_t i);

/*!The monoid of the first \a n factors of the factorisation of \a x,
 * considering the operator \a mult.
 * \pre \a n ≤ \ref cuex_monoid_factor_cnt(\a mult, \a x) */
cuex_t cuex_monoid_factor_prefix(cuex_meta_t mult, cuex_t x, size_t n);

/*!The monoid of factors \a i inclusive to \a j exclusive of the
 * factorisation of \a x, considering the operator \a mult.
 * \pre \a i ≤ \a j ≤ \ref cuex_monoid_factor_cnt(\a mult, \a x) */
cuex_t cuex_monoid_factor_range(cuex_meta_t mult, cuex_t x, size_t i, size_t j);


struct cuex_monoid_it_s
{
    size_t index;
    size_t size;
    cuex_meta_t mult;
    cuex_t stack[CUEX_MONOID_MAXPL_DEPTH];
};
typedef struct cuex_monoid_it_s cuex_monoid_it_t;

/*!Construct \a it as an iterator pointing to the first generator in
 * in the factorisation of \a x with respect to \a mult, and iterating
 * over successive generators. */
void cuex_monoid_it_cct(cuex_monoid_it_t *it, cuex_meta_t mult, cuex_t x);

void cuex_monoid_it_cct_at(cuex_monoid_it_t *it,
			   cuex_meta_t mult, cuex_t x, size_t i);

void cuex_monoid_it_cct_range(cuex_monoid_it_t *it,
			      cuex_meta_t mult, cuex_t x, size_t i, size_t j);

/*!True iff \a it is at the end of its range. */
CU_SINLINE cu_bool_t cuex_monoid_it_is_end(cuex_monoid_it_t *it)
{ return it->index == it->size; }

/*!If \a it is at its end, returns \c NULL, else returns the next
 * generator of the monoid factorisation, and advances \a it to point
 * to the next. */
cuex_t cuex_monoid_it_read(cuex_monoid_it_t *it);

cu_clos_edec(cuex_monoid_factor_src,
	     cu_prot(size_t, cuex_t *buf_arr, size_t buf_size),
    ( struct cuex_monoid_it_s it; ));

#if 0
/* Construct a buffering source for generators \a i inclusive to \a j
 * exclusive in the factorisation of \a x with respect to \a mult.
 * The constructed object \a src is a \c cu_clos.  After costruction
 * \ref cu_clos_ref(\a src) gives a closure pointer of type
 * <pre>size_t cu_clptr(cuex_t *\e buf_arr, size_t \e buf_size)</pre> which
 * when called fills the array at \e buf_arr with up to \e buf_size
 * factors, and returns the actual number filled in.  It returns a
 * short count only when it reaches the end.
 * \pre \a i ≤ j ≤ \ref cuex_monoid_factor_cnt(\a x) */
CU_SINLINE void cuex_monoid_factor_src_cct(cuex_monoid_factor_src_t *src,
					   cuex_meta_t mult, cuex_t x,
					   size_t i, size_t j)
{
    cuex_monoid_it_cct_range(&src->it, mult, x, i, j);
}
#endif

/*!@}*/
CU_END_DECLARATIONS

#endif
