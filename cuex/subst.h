/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CHI_LO_SUBST_H
#define CHI_LO_SUBST_H

#include <cuex/fwd.h>
#include <cuex/pvar.h>
#include <cucon/pmap.h>
#include <cucon/slink.h>
#include <cufo/fwd.h>
#include <cu/conf.h>
#include <stdio.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_subst_h cuex/subst.h: Substitutions
 * @{ \ingroup cuex_mod
 * Substitutions are here implemented as maps from variables to equivalence
 * sets of variables and value-expressions.  A \ref cuex_subst
 * "cuex_subst_t" has a specified set of active quantification kinds.  \ref
 * cuex_subst_unify will attempt to extend the substitution so as to unify two
 * expressions by adding bindings for variables with active quantification.
 * Unification can produce variable equivalences of the form \e x = \e y where
 * \e x and \e y are variables, or bindings of the form \e x = \e e where \e e
 * is an expression.  This is handled by mapping each variable \e x to (\e S,
 * \e e), where \e S is the set of \e x and all variables equivalent to it and
 * \e e is an optional binding for all variables in \e S.  (\e S, \e e) is
 * represented by \ref cuex_veqv and is shared between variables in \e S.
 */

/*!An equivalence set of variables with an optional binding. This is only
 * used as part of a \ref cuex_subst. */
struct cuex_veqv
{
    unsigned int qcode : 8;	/* Actual type is cuex_qcode_t. */
    cu_bool_t is_feedback_var : 1;
    cucon_slink_t var_link;
    cuex_t value;
};

typedef cucon_slink_t cuex_veqv_it_t;

/** The quantification of variables in \a vq. */
CU_SINLINE cuex_qcode_t
cuex_veqv_qcode(cuex_veqv_t vq) { return (cuex_qcode_t)vq->qcode; }

/*!The value of variables in \a vq or \c NULL if unknown. */
CU_SINLINE cuex_t cuex_veqv_value(cuex_veqv_t vq) { return vq->value; }

/*!The primary variable of \a vq. */
CU_SINLINE cuex_var_t cuex_veqv_primary_var(cuex_veqv_t vq)
{ return (cuex_var_t)cucon_slink_get_ptr(vq->var_link); }

/*!After calling \ref cuex_subst_mark_min_feedback or \ref
 * cuex_subst_mark_all_feedback, and before any further change to the
 * substitution, this will return true iff the variables of \a vq are part of
 * a minimum feedback set or which are strongly connected, rsp.  Useful only
 * for non-idempotent substitutions. */
CU_SINLINE cu_bool_t cuex_veqv_is_feedback(cuex_veqv_t vq)
{ return vq->is_feedback_var; }

/*!An iterator to the first variable in \a vq. */
CU_SINLINE cuex_veqv_it_t cuex_veqv_begin(cuex_veqv_t vq)
{ return vq->var_link; }

/*!An iterator beyond the range of variables in \a vq. */
CU_SINLINE cuex_veqv_it_t cuex_veqv_end(cuex_veqv_t vq) { return NULL; }

/*!The iterator next after \a it in a range of variables. */
CU_SINLINE cuex_veqv_it_t cuex_veqv_it_next(cuex_veqv_it_t it)
{ return cucon_slink_next(it); }

/*!The variable at \a it. */
CU_SINLINE cuex_var_t cuex_veqv_it_get(cuex_veqv_it_t it)
{ return (cuex_var_t)cucon_slink_get_ptr(it); }

/*!The representation of a substitution. */
struct cuex_subst
{
    size_t clone_cnt;
    size_t shadow_access_cnt;
    cuex_subst_t shadowed;
    struct cucon_pmap var_to_veqv;
    unsigned int qcset : 8;	/* Actual type is cuex_qcset_t. */
    cu_bool_least_t is_idem;
};

/*!Construct \a subst where variables of quantisation in \a qcset are
 * unifiable (substitutable). */
void cuex_subst_init(cuex_subst_t subst, cuex_qcset_t qcset);

void cuex_subst_init_nonidem(cuex_subst_t subst, cuex_qcset_t qcset);

CU_SINLINE void cuex_subst_init_uw(cuex_subst_t subst)
{ cuex_subst_init(subst, cuex_qcset_uw); }
CU_SINLINE void cuex_subst_init_e(cuex_subst_t subst)
{ cuex_subst_init(subst, cuex_qcset_e); }
CU_SINLINE void cuex_subst_init_n(cuex_subst_t subst)
{ cuex_subst_init(subst, cuex_qcset_n); }

/*!Return a substitution with \a qcset as the quantisation of
 * variables which are substitutable.  */
cuex_subst_t cuex_subst_new(cuex_qcset_t qcset);

cuex_subst_t cuex_subst_new_nonidem(cuex_qcset_t qcset);

CU_SINLINE cuex_subst_t cuex_subst_new_uw(void)
{ return cuex_subst_new(cuex_qcset_uw); }
CU_SINLINE cuex_subst_t cuex_subst_new_e(void)
{ return cuex_subst_new(cuex_qcset_e); }
CU_SINLINE cuex_subst_t cuex_subst_new_n(void)
{ return cuex_subst_new(cuex_qcset_n); }

/*!Return a shallow copy of \a src_subst, which may be NULL. */
cuex_subst_t cuex_subst_new_clone(cuex_subst_t src_subst, cuex_qcset_t qcset);
CU_SINLINE cuex_subst_t cuex_subst_new_uw_clone(cuex_subst_t src_subst)
{ return cuex_subst_new_clone(src_subst, cuex_qcset_uw); }
CU_SINLINE cuex_subst_t cuex_subst_new_e_clone(cuex_subst_t src_subst)
{ return cuex_subst_new_clone(src_subst, cuex_qcset_e); }
CU_SINLINE cuex_subst_t cuex_subst_new_n_clone(cuex_subst_t src_subst)
{ return cuex_subst_new_clone(src_subst, cuex_qcset_n); }

/*!Return a deep copy of \a subst. */
cuex_subst_t cuex_subst_new_copy(cuex_subst_t src_subst);

CU_SINLINE cuex_qcset_t cuex_subst_active_qcset(cuex_subst_t subst)
{ return (cuex_qcset_t)subst->qcset; }

CU_SINLINE cu_bool_t
cuex_subst_is_active_varmeta(cuex_subst_t subst, cuex_meta_t varmeta)
{ return cuex_qcset_contains(cuex_subst_active_qcset(subst),
			     cuex_varmeta_qcode(varmeta)); }

/*!Merge all relevant bindings from inherited substitutions of \a subst into
 * \a subst, and drop the inheritance. */
void cuex_subst_flatten(cuex_subst_t subst);

/*!Indicate that \a subst is no longer in use.  This is just an optimisation,
 * you never need to call this. */
void cuex_subst_delete(cuex_subst_t subst);

/*!The number of variables in \a subst, possible counting variables which
 * have been shadowed after cloning and modifying a substitution.  For
 * cloned substitutions this is just for estimation, since
 * treatment of shadowing is inherently unpredictable. */
size_t cuex_subst_size(cuex_subst_t subst);

cuex_veqv_t cuex_subst_cref(cuex_subst_t subst, cuex_var_t var);
cuex_veqv_t cuex_subst_mref(cuex_subst_t subst, cuex_var_t var);

/*!True iff \a subst is the identity substitution. */
cu_bool_t cuex_subst_is_identity(cuex_subst_t subst);

/*!Attempt to unify \a ex0 and \a ex1 while respecting and adding new variable
 * unifications in \a subst.  If both \a ex0 and \a ex1 are valueless
 * variables, the primary variable of \a ex1 is preserved. */
cu_bool_t cuex_subst_unify(cuex_subst_t subst, cuex_t ex0, cuex_t ex1);

/*!\copydoc cuex_subst_unify
 * Whenever unification fails for pair of subexpressions, the
 * result of \a aux_unify is used instead. */
cuex_t
cuex_subst_unify_aux(cuex_subst_t subst, cuex_t ex0, cuex_t ex1,
		     cu_clop(aux_unify, cuex_t, cuex_subst_t, cuex_t, cuex_t));

/*!Block \a v from being unified with non-variables. */
void cuex_subst_block(cuex_subst_t subst, cuex_var_t v);

/*!Remove a unification-block of \a v. */
void cuex_subst_unblock(cuex_subst_t subst, cuex_var_t v);

/*!Remove all unification-blocks for variables in \a subst. */
void cuex_subst_unblock_all(cuex_subst_t  subst);

/*!For each variable in \a ex which does not have a mapping, insert a
 * substitution from it to a fresh blocked variable. */
void cuex_subst_freshen_and_block_vars_in(cuex_subst_t subst, cuex_t ex);

/* Insert all veqvs in \a ex into \a veqvset. */
void cuex_subst_collect_veqvset(cuex_subst_t subst, cuex_t ex,
				cucon_pmap_t veqvset);

/*!Return the mapping of \a var in \a subst.  If there is no mapping to
 * non-variables, return the primary variable. */
cuex_t cuex_subst_lookup(cuex_subst_t subst, cuex_var_t var);

/*!Return the application of \a subst to \a ex.
 * \pre \a subst is idempotent. */
cuex_t cuex_subst_apply(cuex_subst_t subst, cuex_t ex);

/*!Update each <tt>cuex_tvar_t</tt> type in \a ex to the result of applying
 * \a subst to it. */
void cuex_subst_update_tvar_types(cuex_subst_t subst, cuex_t ex);

/*!Return \a ex with a single expansion of variabels from \a subst.  This
 * also works for non-idempotent substitutions. */
cuex_t cuex_subst_expand(cuex_subst_t subst, cuex_t ex);

/*!Call \a cb for each \a veqv in \a subst. */
void cuex_subst_iter_veqv(cuex_subst_t subst, cu_clop(cb, void, cuex_veqv_t));

/*!Call \a f for each \a veqv in \a subst.  To each \c cuex_veqv_t a unique
 * cache slot of \a cache_size bytes is passed to \a f.  \a f may request the
 * cache data of another variable to be computed before returing, but calling
 * \ref cuex_subst_iter_veqv_cache_sub on the provided \a sub_data.
 * \see cuex_subst_tran_cache */
void cuex_subst_iter_veqv_cache(cuex_subst_t subst, size_t cache_size,
				cu_clop(f, void, cuex_veqv_t veqv, void *cache,
						 void *sub_data));
/*!This is used only inside the callback of \ref cuex_subst_iter_veqv_cache.
 * It forces a computation of cache data for \a var before finishing the
 * current callback.  It is unsafe to use this on non-idempotent
 * substitutions. */
void *cuex_subst_iter_veqv_cache_sub(void *sub_data, cuex_t var);

/*!Transform all bindings in \a subst with \a f.  \a f is passed a cache of \a
 * cache_size bytes, where it can store auxiliary data used during the
 * computation.  The cache associated with a set of equivalent variables can
 * be queried with \ref cuex_subst_tran_cache_sub, which will force the given
 * variable to be processed if necessary.
 * \see cuex_subst_iter_veqv_cache */
cuex_subst_t
cuex_subst_tran_cache(cuex_subst_t subst, size_t cache_size,
		      cu_clop(f, cuex_t, cuex_t value, void *cache,
					 void *sub_data));

/*!This is used only inside the callback of \a cuex_subst_tran_cache.  It
 * forces a computation of cache data for \a var.  It is unsafe to use this on
 * non-idempotent substitutions. */
void *cuex_subst_tran_cache_sub(void *sub_data, cuex_t var);

/*!Sequentially conjunct \a cb over variables that would be left after
 * applying \a subst to \a ex.  For variables in \a ex which are part of an
 * equavalence, \a cb is called with the primary variable.
 * \see cuex_subst_free_vars_insert, cuex_subst_free_vars_erase */
cu_bool_t cuex_subst_free_vars_conj(cuex_subst_t subst, cuex_t ex,
				    cu_clop(cb, cu_bool_t, cuex_var_t));

/*!Insert each variable that would be left after applying \a subst to \a ex
 * into \a accu.
 * \see cuex_subst_free_vars_conj */
void cuex_subst_free_vars_insert(cuex_subst_t subst, cuex_t ex,
				 cucon_pset_t accu);

/*!Erase each variable that would be left after applying \a subst to \a ex
 * from \a accu.
 * \see cuex_subst_free_vars_conj */
void cuex_subst_free_vars_erase(cuex_subst_t subst, cuex_t ex,
				cucon_pset_t accu);

/*!Print \a subst to \a file, using \a sep to separate elements. */
void cuex_subst_print(cuex_subst_t subst, cufo_stream_t fos, char const *sep);

/*!Debug dump of \a subst. */
void cuex_subst_dump(cuex_subst_t subst, FILE *file);


/* Algorithms
 * ---------- */

/* For each node of 'ex', create a mapping to a node which is
 * equivalent under 'dst' where its subexpressions are replaced by
 * appropriate variables.  Returns the variable which is equivalent to
 * 'ex' under 'dst'.  'ex_to_var' shall be a mapping from expressions
 * to variables, and may be empty or contain accumulation from
 * previous calls.  Variabels in 'ex' are resolved using 'src'. */
cuex_var_t cuex_subst_insert_fragment_accum(cuex_subst_t dst,
					    cucon_pmap_t ex_to_var,
					    cuex_t ex, cuex_subst_t src);

/* Calls the above with an empty 'ex_to_var'. */
cuex_var_t cuex_subst_insert_expand(cuex_subst_t dst,
				    cuex_t ex, cuex_subst_t src);

/* Return a substitution which defines the cuex_var_t keys of
 * 'var_to_var' equivalently, but maximally expanded using
 * 'cuex_subst_insert_fragment_accum', and set the cuex_var_t slots of
 * 'var_to_var' to the variables corresponding to the keys. */
cuex_subst_t cuex_subst_fragment_project(cuex_subst_t subst,
					 cucon_pmap_t var_to_var);

#ifdef CUCONF_HAVE_BUDDY
/*!Return an idempotent rewrite of the possibly non-idempotent \a subst.
 * Mutually recursive bindings are replaced by bindings to a top-level \ref
 * CUEX_O2_RBIND where the LHS is a \a CUEX_OR_TUPLE of the old
 * bindings with back-refereces replaced with appropriate \a cuex_rvar_t
 * variables, and the RHS is the \a cuex_rvar_t selecting the appropriate
 * component.
 * \note Only avaliable if built with BuDDY.
 * \see cuex_subst_mark_min_feedback */
void cuex_subst_render_idempotent(cuex_subst_t subst);

/*!Mark all \ref cuex_veqv "cuex_veqv_t" nodes of \a subst which are part of
 * a minimum feedback variable set.  The marks can be inspected with \ref
 * cuex_veqv_is_feedback.
 * \note Only available if built with BuDDY.
 * \see cuex_subst_mark_all_feedback
 * \see cuex_subst_render_idempotent */
cu_bool_t cuex_subst_mark_min_feedback(cuex_subst_t subst);

/*!Mark all \ref cuex_veqv "cuex_veqv_t" nodes of \a subst which are part of
 * a circular dependency.  Considering \a subst to be a directed graph, this
 * marks all strongly connected nodes.  The marks can be inspected with \ref
 * cuex_veqv_is_feedback.
 * \note Only available if built with BuDDY.
 * \see cuex_subst_mark_min_feedback
 * \see cuex_subst_render_idempotent */
cu_bool_t cuex_subst_mark_all_feedback(cuex_subst_t subst);
#endif

/*!@}*/
#define cuex_subst_cct		cuex_subst_init
#define cuex_subst_cct_nonidem	cuex_subst_init_nonidem
#define cuex_subst_cct_uw	cuex_subst_init_uw
#define cuex_subst_cct_e	cuex_subst_init_e
#define cuex_subst_cct_n	cuex_subst_init_n
CU_END_DECLARATIONS

#endif
