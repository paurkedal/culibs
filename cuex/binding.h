/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_BINDING_H
#define CUEX_BINDING_H

#include <cuex/fwd.h>
#include <cuex/opn.h>
#include <cuex/oprdefs.h>
#include <cucon/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_binding_h cuex/binding.h: Variable Binding using de Bruijn Indices
 *@{\ingroup cuex_mod
 *
 * This provides variables, scoping constructs, and algorithms for expressions
 * using de Bruijn indexes.  This includes the built-in μ-recursion and
 * λ-expressions.
 *
 * \see cuex_recursion_h
 */

/*!An expression which is bound to the surrounding binding site after skipping
 * \a l levels.  That is, with \a l = 0 binds to the immediate surronding
 * binding site. */
CU_SINLINE cuex_t
cuex_hole(int l)
{
    cu_debug_assert(0 <= l && l < (1 << CUEX_OA_HOLE_INDEX_WIDTH));
    return cuex_opn(CUEX_O0_HOLE | CUEX_OA_HOLE_INDEX(l));
}

CU_SINLINE cu_bool_t
cuex_is_hole(cuex_t e)
{
    cuex_meta_t meta = cuex_meta(e);
    return cuex_meta_is_opr(meta) && cuex_og_hole_contains(meta);
}

CU_SINLINE cuex_t cuex_mupath_null() { return cuex_null(); }

CU_SINLINE cuex_t
cuex_mupath_pair(int level_diff, cuex_t head_seq, cuex_t tail_comp)
{
    if (head_seq == cuex_mupath_null())
	return tail_comp;
    else
	return cuex_opn(CUEX_O2_MUPATH | CUEX_OA_HOLE_INDEX(level_diff),
			head_seq, tail_comp);
}

/*!If this returns true, then \a opr binds exactly one de Bruijn indexed
 * variable, otherwise \a opr binds no such a variable. */
CU_SINLINE cu_bool_t
cuex_opr_is_binder(cuex_meta_t opr)
{ return cuex_og_binder_contains(opr); }

/*!For all free b-variables in \a e, increment their de Bruijn index by \a
 * l_diff.  This prepares the subexpression \a e to be substituted down
 * accross \a l_diff bind-sites, or for negative, up accross -\a l_diff
 * bind-sites. */
cuex_t cuex_bfree_adjusted(cuex_t e, int l_diff);

/*!Calls \a f(\e lr, \e lr_top) for free hole in \a e, where \e lr is the hole
 * index and \e lr_top is \a l_top plus the binding depth of the hole.  \a
 * l_top is the initial top level to consider free, typically 0. */
void cuex_bfree_iter(cuex_t e, cu_clop(f, void, int, int), int l_top);

/*!Transforms \a e by replacing each free hole with \a f (\e lr, \e lr_top),
 * where \e lr is the hole index and \e lr_top is \a l_top plus the binding
 * depth of the hole.  \a l_top is the top level relative to \a e to consider
 * free, typically 0. */
cuex_t cuex_bfree_tran(cuex_t e, cu_clop(f, cuex_t, int, int), int l_top);

cu_bool_t cuex_bfree_match(cu_clop(f, cu_bool_t, int, cuex_t, int),
			   cuex_t p, cuex_t e, int l_top);

/*!Inserts free variables of \a e into \a set, where \a l_top (usually 0) is
 * considered to be the first unbound level.  The index of the outermost
 * variable is returned, or INT_MIN of there were no free variables. */
int cuex_bfree_into_uset(cuex_t e, int l_top, cucon_uset_t set);

/*!Inserts free variables of \a e into \a set limited by it's initialised size,
 * and returns the number of unique free variables within the range. */
int cuex_bfree_into_bitvect(cuex_t e, int l_top, cucon_bitvect_t set);

cuex_t cuex_reindex_by_int_stack(cuex_t e, int stack_top_level,
				 int stack_span, cucon_stack_t stack);

/*!The maximum number of binding sites, according to \ref
 * cuex_og_binder_contains, which needs to be crossed to reach a leaf of
 * \a e. */
int cuex_max_binding_depth(cuex_t e);

/*!Returns a map {\e x → \e S}, where \e x corresponds to a μ-bind
 * subexpression of \a e, and \e S is a \ref cucon_uset of the de Bruijn
 * indices of the free λ-variables of the body of the μ-bind.  The keys \e x is
 * a list of left-associated \ref CUEX_O2_METAPAIR of all μ-bind expressions
 * down to and including the one \e S applies to.
 *
 * An an optimisation, a known result of \a cuex_max_binding_depth may be
 * passed for \a max_binding_depth, otherwise pass -1.  */
cucon_pmap_t cuex_unfolded_fv_sets(cuex_t e, int max_binding_depth);

#define CUEX_BI_SIFI_FLAG_PRUNE 1

void cuex_bi_sifi_indexing_accu(cuex_t e, unsigned int flags,
				cucon_pmap_t accu);

cucon_pmap_t cuex_bi_sifi_indexing(cuex_t e, unsigned int flags);

/*!@}*/
CU_END_DECLARATIONS

#endif
