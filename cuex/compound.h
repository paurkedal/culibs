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

#ifndef CUEX_COMPOUND_H
#define CUEX_COMPOUND_H

#include <cuex/fwd.h>
#include <cu/clos.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_compound_h cuex/compound.h: Compound Expressions
 *@{\ingroup cuex_mod
 *
 * Compound expressions, or compounds for short, are containers of expressions
 * (dynamic objects and operations).
 * Compounds are also themselves expressions.
 * One thing you can do an compounds without knowing more about their structure
 * is to iterate over their subexpressions.
 * Further, compounds may support creation of their image under a function or
 * iterable construction.
 *
 * The following terminology will reflect that compounds are geared towards
 * representing associative binary operations, though it is just as useful for
 * containers.
 * We distinguish between non-commutative and commutative views.  A compound
 * may bind one or both views.
 *
 * The non-commutative view has one \ref cuex_intf_iterable_s interface for
 * iteration, and an optional \ref cuex_intf_imageable_s interface for
 * construction images.
 * The two non-commutative interfaces must yield the exact same sequence of
 * elements.
 * These interfaces are most useful for non-commutative operators and for
 * list-like containers.
 *
 * The commutative view also has one \ref cuex_intf_iterable_s interface for
 * iteration, and an optional \ref cuex_intf_growable_s interface for
 * construction.
 * These interfaces are most useful for commutative operators and for set-like
 * or map-like containers.
 *
 * They same type may bind both non-commutative and commutative interfaces, in
 * which case the set of elements exposed by iteration may be different but
 * must still have a one-to-one correspondence.
 * For example, a map may be an commutative compound of key-value pairs, or a
 * non-commutative compound of just the values where the keys are treated
 * inertly.
 * Conversely, a vector may have a commutative iterator where the values are
 * index-value pairs.
 */

typedef struct cuex_intf_compound_s *cuex_intf_compound_t;
typedef struct cuex_intf_iterable_s *cuex_intf_iterable_t;
typedef struct cuex_intf_imageable_s *cuex_intf_imageable_t;
typedef struct cuex_intf_growable_s *cuex_intf_growable_t;

/*!Subinterface of \ref cuex_intf_compound_s for iterating over a compound. */
struct cuex_intf_iterable_s
{
    size_t (*itr_size)(cuex_t compound);
    void (*itr_init)(void *itr, cuex_t compound);
    cuex_t (*itr_get)(void *itr);
};

/*!Subinterface of \ref cuex_intf_compound_s for iteratively constructing the
 * image of a componud. */
struct cuex_intf_imageable_s
{
    unsigned int flags;
    size_t (*itr_size)(cuex_t compound);
    void (*itr_init)(void *itr, cuex_t compound);
    cuex_t (*itr_get)(void *itr);
    void (*itr_put)(void *itr, cuex_t member);
    cuex_t (*itr_finish)(void *itr);
};

/*!Subinterface of \ref cuex_intf_growable_s for iteratively constructing a
 * compound. */
struct cuex_intf_growable_s
{
    size_t itr_size;
    void (*itr_init_empty)(void *itr, cuex_t template_compound);
    void (*itr_init_copy)(void *itr, cuex_t compound);
    void (*itr_put)(void *itr, cuex_t member);
    cuex_t (*itr_finish)(void *itr);
};

/*!Set if the non-commutative interface is preferable when an algorithm
 * supports both.
 * This flag \e must be set if the commutative interface is unbound. */
#define CUEX_COMPOUNDFLAG_PREFER_NCOMM 1

/*!Set if the commutative interface is preferable when an algorithm supports
 * both.
 * This flag \e must be set if the non-commutative interface is unbound. */
#define CUEX_COMPOUNDFLAG_PREFER_COMM 2

/*!Set iff successive elements which are equal reduces to a single occurrence
 * for the non-commutative interface. */
#define CUEX_COMPOUNDFLAG_NCOMM_IDEMPOTENT 4

/*!Set iff equal elements reduces to a single occurrence for the commutative
 * interface.  */
#define CUEX_COMPOUNDFLAG_COMM_IDEMPOTENT 8

/*!Set iff the compound is imageable and it is valid to drop elements during
 * the iteration by omitting some calls to \c itr_put. */
#define CUEX_COMPOUNDFLAG_FILTERABLE_IMAGE 16

/*!Set iff the compound is imageable and it is valid to add elements during the
 * iteration by calling \c itr_put several more than once between \c itr_get
 * calls. */
#define CUEX_COMPOUNDFLAG_EXPANSIVE_IMAGE 32

/*!Interface for compound expressions. */
struct cuex_intf_compound_s
{
    unsigned int flags;
    cuex_intf_iterable_t ncomm_iterable;
    cuex_intf_imageable_t ncomm_imageable;
    cuex_intf_iterable_t comm_iterable;
    cuex_intf_growable_t comm_growable;
    cuex_t (*comm_find)(cuex_t compound, cuex_t member);
};

/*!Verifies that \a impl has been correctly initialised.  May also provide
 * backwards-compatibility patching and/or signal such cases. */
void cuex_intf_compound_verify(cuex_intf_compound_t impl);

cu_bool_t cuex_compound_conj(cuex_intf_compound_t impl, cuex_t compound,
			     cu_clop(f, cu_bool_t, cuex_t));

cuex_t cuex_compound_image(cuex_intf_compound_t impl, cuex_t compound,
			   cu_clop(f, cuex_t, cuex_t));

/*!@}*/
CU_END_DECLARATIONS

#endif
