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

#ifndef CUEX_ALGO_H
#define CUEX_ALGO_H

#include <cuex/fwd.h>
#include <cuex/var.h>
#include <cu/idr.h>
#include <cucon/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_algo cuex/algo.h: Algorithms on Expressions and Substitutions
 * @{ \ingroup cuex_mod */

/*!The maximum arity of \a e and any of it's subexpressions.  Compounds are
 * checked for subexpressions, but count only as 1 themselves. */
cu_rank_t cuex_max_arity(cuex_t e);

/*!The depth of the deepest branch of \a e.  The depth of non-operations are
 * considered to be 0, and μ-recursions are ignored. */
int cuex_max_depth(cuex_t e);

cu_bool_t cuex_match_pmap(cuex_t patn, cuex_t ex, cucon_pmap_t pmap);

cuex_t cuex_unify(cuex_t x, cuex_t y);

/*!Returns <tt>cu_call(\a cb, \a e)</tt> if non-\c NULL, else if \a e is an
 * operation, returns it with each operand transformed by a recursive call to
 * this function, else (if \a e is not and operation) returns \a e.  This
 * function handles transformations of ACI trees.
 *
 * This is quite generally applicable for doing deep transformations.  It is
 * based on the idea that \a cb will either know how to handle a certain
 * operator, in which case it can pre-process, invoke cuex_fallback_tran
 * recursively, re-construct, and post-process, or it will not know the form,
 * in which case it typically has no need for any of the processing except for
 * the fall-back recursion and re-construction. */
cuex_t cuex_fallback_tran(cuex_t e, cu_clop(cb, cuex_t, cuex_t));

/*!Sequentially conjunct \a cb over variables in \a ex in depth-first order. */
cu_bool_t cuex_depth_conj_vars(cuex_t ex, cu_clop(cb, cu_bool_t, cuex_var_t));

/*!Sequentially conjunct \a cb over all nodes in \a ex in depth-first
 * leaf-to-root order. */
cu_bool_t cuex_depthout_conj(cuex_t ex, cu_clop(cb, cu_bool_t, cuex_t));

/*!Transform each node of \a ex with \a cb in depth-first leaf-to-root
 * order. */
cuex_t cuex_depthout_tran(cuex_t ex, cu_clop(cb, cuex_t, cuex_t));

/*!Transform the leaves of \a ex with \a cb in depth-first order. */
cuex_t cuex_depth_tran_leaves(cuex_t ex, cu_clop(cb, cuex_t, cuex_t));

/*!Transform the variables of \a ex with \a cb in depth-first order. */
cuex_t cuex_depth_tran_vars(cuex_t ex, cu_clop(cb, cuex_t, cuex_var_t));

/*!Call \a out with the maximum subtrees of \a ex for which \a pred holds
 * true on all leaves. */
cu_bool_t cuex_maxtrees_of_leaftest_iter(cuex_t ex,
					 cu_clop(pred, cu_bool_t, cuex_t leaf),
					 cu_clop(out, void, cuex_t subex));

/*!Return the result of transforming with \a tran all maximum subtrees
 * of \a ex for which \a pred holds true on all leaves.
 * E.g. if \a pred checks if a leaf is constant, then this function
 * transforms (\e x + 9) ⋅ (2 + (1 + 1)) to
 * (\e x + \a tran(9)) ⋅ \a tran(2 + (1 + 1)) with \a tran evaluated. */
cuex_t cuex_maxtrees_of_leaftest_tran(cuex_t ex,
				      cu_clop(pred, cu_bool_t, cuex_t leaf),
				      cu_clop(tran, cuex_t, cuex_t subex));


cu_bool_t cuex_has_weak_var(cuex_t ex);

cu_bool_t cuex_lgr(cuex_t ex0, cuex_t ex1, cuex_subst_t subst);

/*!Computes the most specific structural generalisation of \a e0 and \a e1.
 * Subexpressions of \a e0 and \a e1 which are different are replaced by
 * <tt>unify(e0sub, e1sub)</tt>.  Typical use is for \a unify to return a new
 * variable, and possibly record the mismatching sub-expressions. */
cuex_t cuex_msg_unify(cuex_t e0, cuex_t e1,
		      cu_clop(unify, cuex_t, cuex_t e0sub, cuex_t e1sub));

/*!A variant of \ref cuex_msg_unify which compares any number of terms
 * simultaneously. */
cuex_t cuex_msg_unify_by_arr(size_t cnt, cuex_t *arr,
			     cu_clop(unify, cuex_t, cuex_t *arr));

/*!The number of subexpressions, variables, and objects in \a e, counting
 * duplicates only once. */
size_t cuex_count_unique_nodes(cuex_t e);

/*!The number of subexpressions, variables, and objects in \a e, counting
 * duplicates only once, excluding \a exclude, and adding all the counted
 * items to \a exclude. */
size_t cuex_count_unique_nodes_except(cuex_t ex, cucon_pmap_t exclude);

cu_bool_t cuex_contains_var_in_pmap(cuex_t ex, cucon_pmap_t pmap);

cu_bool_t cuex_contains_ex(cuex_t ex, cuex_t sub);

/*!Return the result of substituting \a var with \a value in \a ex. */
cuex_t cuex_substitute_ex(cuex_t ex, cuex_t var, cuex_t value);

/*!Return the result of substituting in \a ex all keys of \a pmap with
 * their values.
 * \arg pmap A \c cucon_pmap_t mapping from \c cuex_t to \c cuex_t */
cuex_t cuex_substitute_pmap(cuex_t ex, cucon_pmap_t pmap);

/*!Return the leftmost leaf of \a ex with the meta \a meta. */
cuex_t cuex_leftmost_with_meta(cuex_t ex, cuex_meta_t meta);
/*!Return the rightmost leaf of \a ex with the meta \a meta. */
cuex_t cuex_rightmost_with_meta(cuex_t ex, cuex_meta_t meta);

/*!The leftmost identifier in \a ex.
 * \pre \a ex must not contain ACI operators. */
CU_SINLINE cu_idr_t cuex_leftmost_idr(cuex_t ex)
{ return (cu_idr_t)cuex_leftmost_with_meta(ex,
				cuoo_type_to_meta(cu_idr_type())); }

/*!The rightmost identifier in \a ex.
 * \pre \a ex must not contain ACI operators. */
CU_SINLINE cu_idr_t cuex_rightmost_idr(cuex_t ex)
{ return (cu_idr_t)cuex_rightmost_with_meta(ex,
				cuoo_type_to_meta(cu_idr_type())); }

/*!The leftmost variable in \a ex having quantification in \a qcset.
 * \pre \a ex must not contain ACI operators. */
cuex_var_t cuex_leftmost_var(cuex_t ex, cuex_qcset_t qcset);

/*!The rightmost variable in \a ex having quantification in \a qcset.
 * \pre \a ex must not contain ACI operators. */
cuex_var_t cuex_rightmost_var(cuex_t ex, cuex_qcset_t qcset);

/*!If the toplevel operator of \a ex is not \a opr then 0, else 1
 * plus the recursive call on the leftmost operand.  */
int cuex_binary_left_depth(cuex_meta_t opr, cuex_t ex);

/*!Given \a opr is a binary operator.
 * If the toplevel operator of \a ex is not \a opr then 0, else 1
 * plus the recursive call on the second operand. */
int cuex_binary_right_depth(cuex_meta_t opr, cuex_t ex);

cuex_t cuex_binary_left_subex(cuex_meta_t opr, cuex_t ex, int depth);
cuex_t cuex_binary_right_subex(cuex_meta_t opr, cuex_t ex, int depth);

/*!Given \a opr is a binary operator.
 * If the toplevel operator of \a ex is not \a opr then
 * to \a opr (\a lhs, \a ex), else \a ex with its left operand transformed
 * by recursive application of this function. */
cuex_t cuex_binary_inject_left(cuex_meta_t opr, cuex_t ex, cuex_t lhs);

/*!Do a sequential conjunction of \a fn over the free variables in \a ex
 * quantified as one of \a qcset, except for those in \a excl.  \a excl may be
 * \c NULL.  The second argument to \a fn is \a excl augmented with locally
 * bound variables at the current spot.
 *
 * A variable which is the first operand of a scoping group, or which is a
 * label in a labelling at said position, is considered to be bound in the
 * remaining operands and in the right-hand sides of the labelling.  This
 * algorithm supports compounds.
 *
 * \note \a excl must be thread local, as it is modified and restored by the
 * algorithm. */
cu_bool_t
cuex_free_vars_conj(cuex_t ex, cuex_qcset_t qcset, cucon_pset_t excl,
		    cu_clop(fn, cu_bool_t, cuex_var_t, cucon_pset_t));

/*!Calls \a ref cuex_free_vars_conj with a callback that inserts elements into
 * \a accu. */
void cuex_free_vars_insert(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl,
			   cucon_pset_t accu);

/*!Calls \a ref cuex_free_vars_conj with a callback that erases elements from
 * \a accu. */
void cuex_free_vars_erase(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl,
			  cucon_pset_t accu);

/*!The number of free variables in \a e with quantification in \a qcset,
 * excluding \a excl if non-\c NULL. */
int cuex_free_vars_count(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl);

/*!Transforms \a e by replacing each free variable of quantification among \a
 * qcset with their mapping under \a f.  \a excl is the current set of variable
 * considered bound; it's modified in-place and restored. */
cuex_t
cuex_free_vars_tran(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl,
		    cu_clop(f, cuex_t, cuex_t, cucon_pset_t));

/*!Returns the result of substituting each free variable in \a e which has
 * quantification among \a qcset and occurs in the domain of \a subst, with
 * their respective mapping from \a subst.  \a excl is the current set of
 * variables considered bound; it's modified in-place and restored. */
cuex_t
cuex_free_vars_tran_pmap(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl,
			 cucon_pmap_t subst);

/*!Returns \a e if \a vars is empty, else return
 * <tt>cuex_opn(\a opr, \e v, vars ∖ * {\e v})</tt> for some \e v in
 * \a vars.  \pre \a opr must be binary. */
cuex_t cuex_outmost_quantify_vars(cuex_meta_t opr, cucon_pset_t vars, cuex_t e);

/* Insert ex into accum and return true. */
cu_clos_edec(cuex_pset_curried_insert_ex,
	     cu_prot(cu_bool_t, cuex_t ex),
    ( cucon_pset_t accu; ));

/* Erase ex from accum and return true. */
cu_clos_edec(cuex_pset_curried_erase_ex,
	     cu_prot(cu_bool_t, cuex_t ex),
    ( cucon_pset_t accu; ));


/*!The statistics returned by \ref cuex_stats. */
typedef struct cuex_stats
{
    size_t node_cnt;		/*!< total number of nodes */
    size_t   var_cnt;		/*!< number of variables of any kind */
    size_t     strong_var_cnt;
    size_t     weak_var_cnt;
    size_t     passive_var_cnt;
    size_t     other_var_cnt;
    size_t   opn_cnt;		/*!< number of operations */
    size_t   obj_cnt;		/*!< number of objects (constants) */
    size_t   other_node_cnt;
} cuex_stats_t;
/*!Make some statistics of \a ex. */
void cuex_stats(cuex_t ex, cuex_stats_t *stats);

cuex_t cuex_autoquantify_uvw_xyz(cuex_t ex, cucon_pmap_t env);

#ifdef CUCONF_DEBUG_CLIENT
cuex_t cuex_binary_inject_left_D(cuex_meta_t, cuex_t, cuex_t);
#define cuex_binary_inject_left cuex_binary_inject_left_D
#endif

/*!@}*/
CU_END_DECLARATIONS

#endif
