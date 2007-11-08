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

#ifndef CUEX_ACI_H
#define CUEX_ACI_H

#include <cuex/fwd.h>
#include <cuex/ex.h>
#include <cuex/opn.h>
#include <cu/algo.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_aci_h cuex/aci.h: Acceletation of Associative, Commutative and Idempotent Operations
 * @{\ingroup cuex_mod
 *
 * \deprecated This implementation has an untracked strict-aliasing bug which
 * shows up with -fstrict-aliasing recent GCC versions.  Due to the unnecessary
 * high complexity this pure expression-based implementation, it will be
 * replaced by interfaces based on the much simpler (non-expression) \ref
 * cuex_atree_h "cuex/atree.h" container.
 *
 * The ACI-operators, here denoted by ∨ (join), fulfill
 * <ul>
 *   <li>associative: x ∨ (y ∨ z) = (x ∨ y) ∨ z</li>
 *   <li>commutative: x ∨ y = y ∨ x</li>
 *   <li>idempotent: x ∨ x = x
 * </ul>
 * These axioms define a (join-)semilattice (S, ∨), and may be useful for
 * algebraic structures which share the same axioms.
 * ACI-operators can also be used to define sets and maps directly as
 * expressions.  (By convention we use join instead of meet so that
 * sets can be naturally expressed by using ∨ for ∪ and ≤ for ⊆, but the
 * same implementation covers a meet-semilattice by trivial substitution
 * of terms.)
 *
 * The ACI-operations are implemented on top of the \c cuex_opn_t
 * operations, where the first two operands are used to create a
 * tree-structure, the third operand represents a key, and the
 * remaining operands are optional value fields.  The key and
 * value operands are called the ACI-operands, and the ACI-arity
 * the number of ACI-operands.  Thus, ACI-arity of 1 gives a set
 * (only the key field), and higher arities gives maps.
 *
 * Note that if the ACI-arity is greater than 1 we must restrict
 * the generators, {(key, value...)}, such that there is at most
 * one generator with a given key.  This is dealt with pragmatically
 * by either returning \c NULL for conflict in \ref cuex_aci_insert,
 * or by overriding the conflicting generator in \ref cuex_aci_replace.
 *
 * \todo Algoritms have not been extended to properly handle ACI trees.
 * Currently, the keys of an ACI-tree must not be substituted, or the
 * invariants of the tree will break.  Make sure the keys are inert.
 */

/*!Makes an ACI-operator with elements of arity \a aci_r.
 * \a aci_r must be 1 or larger.  The first ACI-operand is the key, so
 * \a aci_r = 1 gives a set, whereas \a aci_r > 1 gives a map. */
#define cuex_aci_opr(index, aci_r) cuex_opr(index, aci_r + 2)
#define cuex_aci_opr_with_ctor(index, aci_r) \
    cuex_opr_with_ctor(index, aci_r + 2)
#define cuex_o0aci(opr) ((opr) & ~(CUEX_OPR_ARITY_MASK | CUEX_OPRFLAG_CTOR))
#define cuex_opr_is_aci(opr) cuex_og_aci_contains(opr)

/*!The ACI-arity of \a join assuming it is an ACI-operator. */
#define cuex_aci_opr_r(join) (cuex_opr_r(join) - 2)

/*!The left-link of an internal ACI tree node.
 * Normally you don't need this. */
CU_SINLINE cuex_t cuex_aci_left(cuex_opn_t e) { return cuex_opn_at(e, 0); }
/*!The right-link of an internal ACI tree node.
 * Normally you don't need this. */
CU_SINLINE cuex_t cuex_aci_right(cuex_opn_t e) { return cuex_opn_at(e, 1); }
/*!The key-link of an internal ACI tree node. */
CU_SINLINE cuex_t cuex_aci_key(cuex_opn_t e) { return cuex_opn_at(e, 2); }

cuex_opn_t cuex_aci_rebranch(cuex_opn_t e,
			     cuex_opn_t new_left, cuex_opn_t new_right);

/*!Assuming \a tree is an ACI-tree, return the subtree of \a tree where
 * the toplevel node contains \a key in its key position, or NULL if there
 * is no such subtree.  Undefined if \a tree is not an ACI-tree. */
cuex_opn_t cuex_aci_find(cuex_opn_t tree, cuex_t key);

/*!Returns the identity element of \a join, which in set/map terms is
 * the empty set/map. */
CU_SINLINE cuex_opn_t cuex_aci_identity(cuex_meta_t join)
{ return cuex_opn(cuex_o0aci(join)); }

/*!True iff \a e is the identity element of \a join. */
CU_SINLINE cu_bool_t cuex_is_aci_identity(cuex_meta_t join, cuex_t e)
{ return cuex_meta(e) == cuex_o0aci(join); }

/*!True iff \a e is an element of the universe associated with \a join,
 * that is, it is the idenity element of \a join, an generator
 * or an induced element. */
CU_SINLINE cu_bool_t cuex_is_aci(cuex_meta_t join, cuex_t e)
{ return cuex_o0aci(cuex_meta(e)) == cuex_o0aci(join); }

/*!Create a generator for \a join identified by \a key and having
 * <tt>cuex_aci_opr_r(\a join)</tt> - 1 associated values.  In set/map
 * terms, this is a singular set/map. */
cuex_opn_t cuex_aci_generator(cuex_meta_t join, cuex_t key, ...);

/*!Assuming \a x is an ACI-tree of the same \a join operator ∨, returns
 * \a x ∨ \e y, where \e y is a generator created from \a y_key and
 * \c cuex_aci_opr_r(\a join) - 1 vararg arguments.
 * If the ACI-arity is greater than 1, then the additional non-keyed
 * values may conflict with an already present element, in which case
 * \c NULL is returned to indicate that idempotency (x ∨ x = x)
 * can not be fulfilled.
 *
 * In set/map terms, this is the set/map with (\a y_key, ...) inserted, or
 * \c NULL if the values conflict with another mapping for \a y_key. */
cuex_opn_t cuex_aci_insert(cuex_meta_t join, cuex_opn_t x, cuex_t y_key, ...);

/*!Same as \ref cuex_aci_insert, but in case of conflict with a present
 * \a y_key mapping, replace it. */
cuex_opn_t cuex_aci_replace(cuex_meta_t join, cuex_opn_t x, cuex_t y_key, ...);

/*!Returns \a x ∨ \a y where ∨ = \a join.
 * \pre The ACI-arity of \a join must be 1, unless both lhs and rhs are
 * ACI-expressions of the same \a join operator.  Preparing potential
 * operands with \ref cuex_aci_generator may help to fulfill this
 * condition. */
cuex_opn_t cuex_aci_join(cuex_meta_t join, cuex_t x, cuex_t y);

/*!Returns \a x ∧ \a y where ∧ is the dual of ∨ = \a join. It is assumed that
 * the arguments are ∨-expressions over independent elements, including
 * bare expressions which are treated with simple equality. */
cuex_opn_t cuex_aci_meet(cuex_meta_t join, cuex_t x, cuex_t y);

/*!Returns an enum of the ordering relation which applies to \a x and \a y,
 * considering semilattice relations of the \a join operator. */
cu_order_t cuex_aci_order(cuex_meta_t join, cuex_t x, cuex_t y);

/*!True iff \a x ≤ \a y where ≤ corresponds to \a join. */
cu_bool_t cuex_aci_leq(cuex_meta_t join, cuex_t x, cuex_t y);

/*!Assuming \a x is an ACI-tree of ACI-arity at least \a i + 1, returns
 * ACI-operand number \a i.  \a i = 0 gives the key operand. */
CU_SINLINE cuex_t cuex_aci_at(cuex_opn_t x, cu_rank_t i)
{ return cuex_opn_at(x, 2+i); }

/*!Sequential conjunction of \a cb over nodes of an aci tree \a x of
 * the operator \a join. */
cu_bool_t cuex_aci_conj(cuex_meta_t join, cuex_opn_t x,
			cu_clop(cb, cu_bool_t, cuex_opn_t));

/*!Transform generators of \a x with \a cb and return the restructured
 * result.  \a cb may return the identity to remove and whole trees to insert
 * more elements.  If \a cb returns \c NULL, then this function terminates
 * immediately with a \a NULL result. */
cuex_opn_t cuex_aci_tran(cuex_meta_t join, cuex_opn_t x,
			 cu_clop(cb, cuex_opn_t, cuex_opn_t));

typedef cu_clop(cuex_aci_merge_fn_t, cuex_opn_t, cuex_opn_t, cuex_opn_t,
						 cuex_opn_t, cuex_opn_t);


/* Debugging */

typedef struct cuex_aci_stats_s *cuex_aci_stats_t;
struct cuex_aci_stats_s
{
    int min_depth;
    int max_depth;
    size_t node_cnt;
    long node_depth_sum;
    size_t leaf_cnt;
    long leaf_depth_sum;
};

void cuex_aci_stats(cuex_opn_t e, cuex_aci_stats_t stats_out);
size_t cuex_aci_count(cuex_opn_t e);
void cuex_aci_dump(cuex_opn_t e, FILE *os);

/*!@}*/
CU_END_DECLARATIONS

#endif
