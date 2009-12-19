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

#ifndef CUEX_SEMILATTICE_H
#define CUEX_SEMILATTICE_H

#include <cuex/opn.h>
#include <cu/algo.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cuex_semilattice_h cuex/semilattice.h: Expression Support for Semilattices
 ** @{ \ingroup cuex_mod
 **
 ** This provides expressions in a canonical form according to the three axioms
 ** of semilattices.  Assuming a meet-semilattice, these are
 **
 ** - associative: \e x ∧ (\e y ∧ \e z) = (\e x ∧ \e y) ∧ \e z
 ** - commutative: \e x ∧ \e y = \e y ∧ \e x
 ** - idempotent: \e x ∧ \e x = \e x
 **
 ** We also assume an identity element ⊤ (top) which fulfils \e x ∧ ⊤ = \e x
 ** for any \e x.  For join-semilattices, replace ∧ with ∨ (join) and ⊤ with ⊥
 ** (bottom).
 **
 ** Since the operator is stored in the lattice structures, the same functions
 ** can be used for both meet- or join-semilattices, as well as other
 ** semi-lattice like terms.
 ** To avoid confusion while reading client code, separate naming schemes for
 ** meet and join semi-lattices are implemented by using <code>\#define</code>
 ** aliases for the latter.
 **
 ** Elements are considered equal if they are pointer-equal.
 ** Since expressions are hash-consed, this gives structural equality.
 ** Terms are atomic with respect to a ∧-semilattice if their top-level node is
 ** not a ∧ operation.
 **
 ** There is no special treatment of variables.
 **/

extern cuoo_type_t cuexP_semilattice_type;

CU_SINLINE cuoo_type_t cuex_semilattice_type(void)
{ return cuexP_semilattice_type; }

/** Returns the top element of the ∧-semilattice, where ∧ = \a meet.  This is
 ** also the identity element. */
cuex_t cuex_meetlattice_top(cuex_meta_t meet);

/** For a ∧-semilattice where \a meet = ∧, returns \a x ∧ \a y in a canonical
 ** form with respect top-level ∧-terms of \a x and \a y. */
cuex_t cuex_meetlattice_meet(cuex_meta_t meet, cuex_t x, cuex_t y);

/** This computes \a x ∨ \a y where ∨ is the dual of ∧ = \a meet, under the
 ** assumption that \a a ∧ \a b = ⊤ for any two distinct atomic terms \e a and
 ** \e b. */
cuex_t cuex_meetlattice_semijoin(cuex_meta_t meet, cuex_t x, cuex_t y);

/** True iff \a x = \a x ∧ \e y. */
cu_bool_t cuex_meetlattice_leq(cuex_meta_t meet, cuex_t x, cuex_t y);

/** Returns the most precise ordering predicate for \a x and \a y. */
cu_order_t cuex_meetlattice_order(cuex_meta_t meet, cuex_t x, cuex_t y);

#define cuex_joinlattice_bottom		cuex_meetlattice_top
#define cuex_joinlattice_join		cuex_meetlattice_meet
#define cuex_joinlattice_semimeet	cuex_meetlattice_semijoin
#define cuex_joinlattice_geq		cuex_meetlattice_leq
#define cuex_joinlattice_leq		cuex_meetlattice_geq
#define cuex_joinlattice_order(opr, x, y) cuex_meetlattice_order(opr, y, x)

/** @}*/
CU_END_DECLARATIONS

#endif
