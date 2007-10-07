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
#include <cuex/ltree.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_monoid_h cuex/monoid.h: Expressions of Associative Operators with Identity
 * @{\ingroup cuex_mod
 * Create and manipulate expressions trees of associative operators.
 * These corresponds to the operators of monoids, though since we
 * are working with raw expressions, the set of the monoid is not explicit.
 */

typedef struct cuex_monoid_s *cuex_monoid_t;
struct cuex_monoid_s
{
    CUOO_HCOBJ
    cuex_meta_t opr;
    cuex_t ltree;
};

extern cuoo_stdtype_t cuexP_monoid_type;

CU_SINLINE cuoo_type_t
cuex_monoid_type()
{ return cuoo_stdtype_to_type(cuexP_monoid_type); }

/*!True iff \a x is a monoid product or a monoid identity element of any
 * operator.
 * \see cuex_check_monoid
 * \see cuex_is_monoid */
CU_SINLINE cu_bool_t
cuex_is_any_monoid(cuex_t x)
{ return cuex_meta(x) == cuoo_type_to_meta(cuex_monoid_type()); }

/*!If \a x is a monoid (according to \ref cuex_is_monoid), then set \c *\a
 * mult_out to the exact operator and return true, else return false.
 * \see cuex_is_monoid
 * \see cuex_is_monoid */
CU_SINLINE cu_bool_t
cuex_check_monoid(cuex_t x, cuex_meta_t *mult_out)
{
    if (cuex_is_any_monoid(x)) {
	*mult_out = ((cuex_monoid_t)x)->opr;
	return cu_true;
    } else
	return cu_false;
}

/*!True iff \a x is a non-generator element of the monoid induced by
 * \a mult, including the identity of \a mult.
 * \see cuex_is_monoid
 * \see cuex_check_monoid */
CU_SINLINE cu_bool_t
cuex_is_monoid(cuex_meta_t mult, cuex_t x)
{ return cuex_is_any_monoid(x) && ((cuex_monoid_t)x)->opr == mult; }

/*!True iff \a x is a monoid product, i.e. having at least two factors. */
CU_SINLINE cu_bool_t
cuex_is_monoid_product(cuex_meta_t mult, cuex_t x)
{ return cuex_is_monoid(mult, x)
     && !cuex_ltree_is_empty(((cuex_monoid_t)x)->ltree); }

/*!The identity element of the monoid induced by \a mult. */
cuex_t cuex_monoid_identity(cuex_meta_t mult);

/*!True iff \a x is the identity of the monoid operator \a mult. */
CU_SINLINE cu_bool_t
cuex_is_monoid_identity(cuex_meta_t mult, cuex_t x)
{
    return cuex_is_monoid(mult, x)
	&& cuex_ltree_is_empty(((cuex_monoid_t)x)->ltree);
}

/*!Returns \a x * \a y, where * = \a mult. */
cuex_t cuex_monoid_product(cuex_meta_t mult, cuex_t x, cuex_t y);

cuex_t cuex_monoid_product_over_source(cuex_meta_t mult,
				       cu_ptr_source_t source);

cuex_t cuex_monoid_right_multiply_source(cuex_meta_t mult,
					 cuex_t x, cu_ptr_source_t source);

/*!The number of generators in the factorisation of \a x with respect to
 * the operator \a mult. */
size_t cuex_monoid_factor_count(cuex_meta_t mult, cuex_t x);

/*!Factor number \a i (counting from 0) of the factorisation of \a x
 * with respect to \a mult.
 * \pre \a i ≤ \ref cuex_monoid_factor_count(\a mult, \a x) */
cuex_t cuex_monoid_factor_at(cuex_meta_t mult, cuex_t x, size_t i);

/*!The monoid of factors \a i inclusive to \a j exclusive of the
 * factorisation of \a x, considering the operator \a mult.
 * \pre \a i ≤ \a j ≤ \ref cuex_monoid_factor_count(\a mult, \a x) */
cuex_t cuex_monoid_factor_slice(cuex_meta_t mult, cuex_t x, size_t i, size_t j);

/* Construct a source for generators \a i inclusive to \a j
 * exclusive in the factorisation of \a x with respect to \a mult. */
cu_ptr_source_t cuex_any_monoid_factor_source(cuex_t e, size_t i, size_t j);

struct cuex_monoid_itr_s { struct cuex_ltree_itr_s sub; };
typedef struct cuex_monoid_itr_s cuex_monoid_itr_t;

void cuex_monoid_itr_init_full(cuex_meta_t mult,
			       cuex_monoid_itr_t *itr, cuex_t x);

void cuex_monoid_itr_init_slice(cuex_meta_t mult,
				cuex_monoid_itr_t *itr, cuex_t x,
				size_t i, size_t j);

CU_SINLINE cuex_t cuex_monoid_itr_get(cuex_monoid_itr_t *itr)
{ return cuex_ltree_itr_get(&itr->sub); }

CU_SINLINE cu_bool_t cuex_monoid_itr_is_end(cuex_monoid_itr_t *itr)
{ return cuex_ltree_itr_is_end(&itr->sub); }

/*!@}*/
CU_END_DECLARATIONS

#endif
