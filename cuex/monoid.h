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
#include <cuex/ltree.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_monoid_h cuex/monoid.h: Monoid Operations
 * @{\ingroup cuex_mod
 *
 * This compound expression type aids in manipulating expressions of
 * associative operators by keeping them in a canonical form.  It implements a
 * free monoid where the generators are arbitrary expressions.  Further
 * degeneracies, including handling of variables is left the client.
 */

typedef struct cuex_monoid_s *cuex_monoid_t;
struct cuex_monoid_s
{
    CUOO_HCOBJ
    cuex_meta_t opr;
    cuex_t ltree;
};

/*!Special value for the end position of slicing operations to indicate slicing
 * to the end. */
#define CUEX_MONOID_END CUEX_LTREE_END

extern cuoo_type_t cuexP_monoid_type;

/*!The object type of elements of monoid compounds. */
CU_SINLINE cuoo_type_t
cuex_monoid_type()
{ return cuexP_monoid_type; }

/*!True iff \a t is the type of a monoid element. */
CU_SINLINE cu_bool_t
cuex_is_monoid_type(cuex_t t)
{ return t == cuexP_monoid_type; }

/*!True iff \a meta is the meta of a monoid element. */
CU_SINLINE cu_bool_t
cuex_is_monoid_meta(cuex_meta_t meta)
{ return meta == cuoo_type_to_meta(cuexP_monoid_type); }

/*!True iff a monoid under any operator contains \a x as a non-generator.
 * \see cuex_check_monoid
 * \see cuex_is_monoid_nongen */
CU_SINLINE cu_bool_t
cuex_is_any_monoid_nongen(cuex_t x)
{ return cuex_meta(x) == cuoo_type_to_meta(cuex_monoid_type()); }

/*!If \a x is a monoid (according to \ref cuex_is_monoid_nongen), then set
 * \c *\a mult_out to the exact operator and return true, else return false.
 * \see cuex_is_monoid_nongen
 * \see cuex_is_any_monoid_nongen */
CU_SINLINE cu_bool_t
cuex_check_monoid(cuex_t x, cuex_meta_t *mult_out)
{
    if (cuex_is_any_monoid_nongen(x)) {
	*mult_out = ((cuex_monoid_t)x)->opr;
	return cu_true;
    } else
	return cu_false;
}

/*!Provided \a x is a monoid, returns it's operator. Do not call this without
 * checking if \a x is a monoid. */
CU_SINLINE cuex_meta_t
cuex_monoid_opr(cuex_t x)
{ return ((cuex_monoid_t)x)->opr; }

/*!True iff \a x is a non-generator element of the monoid under \a mult.  That
 * is, if \a x is either the identity element or a composite element.
 * \see cuex_is_monoid_nongen
 * \see cuex_check_monoid */
CU_SINLINE cu_bool_t
cuex_is_monoid_nongen(cuex_meta_t mult, cuex_t x)
{ return cuex_is_any_monoid_nongen(x) && ((cuex_monoid_t)x)->opr == mult; }

/*!True iff \a x is the identity element of the monoid under \a mult. */
CU_SINLINE cu_bool_t
cuex_is_monoid_identity(cuex_meta_t mult, cuex_t x)
{
    return cuex_is_monoid_nongen(mult, x)
	&& cuex_ltree_is_empty(((cuex_monoid_t)x)->ltree);
}

/*!True iff \a x is a composite element of the monoid under \A{mult}. */
CU_SINLINE cu_bool_t
cuex_is_monoid_composite(cuex_meta_t mult, cuex_t x)
{ return cuex_is_monoid_nongen(mult, x)
     && !cuex_ltree_is_empty(((cuex_monoid_t)x)->ltree); }

/*!The identity element of the monoid induced by \a mult.  This is the same for
 * both unnested or nested cases.*/
cuex_t cuex_monoid_identity(cuex_meta_t mult);

/*!Returns the \A{mult}-product of factors drawn from \a source in order. */
cuex_t cuex_monoid_from_source(cuex_meta_t mult, cu_ptr_source_t source);

/*!Returns a \a mult monoid element of \a factor_count factors from the array
 * \a factor_arr. */
cuex_t cuex_monoid_from_array(cuex_meta_t mult,
			      cuex_t *factor_arr, size_t factor_count);

/*!Returns \a x * \a y, where * = \a mult. */
cuex_t cuex_monoid_product(cuex_meta_t mult, cuex_t x, cuex_t y);

/*!Returns \a x * \a y, where * = \a mult and \a y is treated as a single
 * factor.  If \a y is itself a monoid, the result is a nested monoid. */
cuex_t cuex_monoid_rightmult(cuex_meta_t mult, cuex_t x, cuex_t y);

/*!The result of repeated application of \ref cuex_monoid_rightmult, starting
 * with \a x, for each expression drawn from \a source as the last argument. */
cuex_t cuex_monoid_rightmult_source(cuex_meta_t mult, cuex_t x,
				    cu_ptr_source_t source);

/*!The result of repeated application of \ref cuex_monoid_rightmult, starting
 * with \a x, for of the \a count expressions of the \a array as the last
 * argument. */
cuex_t cuex_monoid_rightmult_array(cuex_meta_t mult, cuex_t x,
				   cuex_t *array, size_t count);

/*!The number of factors in the factorisation of \a x with respect to
 * \a mult. */
size_t cuex_monoid_length(cuex_meta_t mult, cuex_t x);

/*!Factor number \a i (counting from 0) of the factorisation of \a x
 * with respect to \a mult.  If \a i is negative it's taken to be relative to
 * just past the last factor of \a x.
 * \pre \e N ≤ \a i < \e N where \e N = \ref cuex_monoid_length(\a mult, \a x) */
cuex_t cuex_monoid_factor_at(cuex_meta_t mult, cuex_t x, ptrdiff_t i);

/*!The monoid of factors \a i inclusive to \a j exclusive of the
 * factorisation of \a x, considering the operator \a mult.
 * \pre \a i ≤ \a j ≤ \ref cuex_monoid_length(\a mult, \a x) */
cuex_t cuex_monoid_factor_slice(cuex_meta_t mult, cuex_t x,
				ptrdiff_t i, ptrdiff_t j);

/*!Returns a source which provides the factors \a i inclusive to \a j exclusive
 * in the factorisation of \a x with respect to \a mult. */
cu_ptr_source_t cuex_any_monoid_factor_source(cuex_t e,
					      ptrdiff_t i, ptrdiff_t j);

struct cuex_monoid_itr_s { struct cuex_ltree_itr_s sub; };
typedef struct cuex_monoid_itr_s cuex_monoid_itr_t;

/*!Initialises \a itr for iterating over all \A{mult}-factors of \a x. */
void cuex_monoid_itr_init_full(cuex_meta_t mult,
			       cuex_monoid_itr_t *itr, cuex_t x);

/*!Initialises \a itr for iterating over \A{mult}-factors starting with number
 * \a i and stopping at number \a j. */
void cuex_monoid_itr_init_slice(cuex_meta_t mult,
				cuex_monoid_itr_t *itr, cuex_t x,
				ptrdiff_t i, ptrdiff_t j);

/*!Pops the next expression off \a itr or returns \c NULL if \a itr is
 * empty. */
CU_SINLINE cuex_t cuex_monoid_itr_get(cuex_monoid_itr_t *itr)
{ return cuex_ltree_itr_get(&itr->sub); }

/*!Checks if \a itr is empty without altering it. */
CU_SINLINE cu_bool_t cuex_monoid_itr_is_end(cuex_monoid_itr_t *itr)
{ return cuex_ltree_itr_is_end(&itr->sub); }

/*!@}*/
CU_END_DECLARATIONS

#endif
