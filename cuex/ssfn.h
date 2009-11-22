/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_SSFN_H
#define CUEX_SSFN_H

#include <cuex/fwd.h>
#include <cucon/pmap.h>
#include <cucon/rbtree.h>
#include <cucon/stack.h>
#include <cuex/pvar.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_ssfn_h cuex/ssfn.h: Syntactic/Semantic Functions
 * @{ \ingroup cuex_mod */

/* Structural Semantic Functions
 * ============================= */

typedef struct cuex_ssfn_node_s *cuex_ssfn_node_t;
typedef struct cuex_ssfn_s *cuex_ssfn_t;
typedef unsigned long cuex_ssfn_seqno_t;

struct cuex_ssfn_node_s
{
    /* Match on current operand. */
    struct cucon_pmap match_exaddr;	/* cuex_t → cuex_ssfn_node_s */
    struct cucon_umap match_opr;	/* opr → cuex_ssfn_node_s */

    /* For patterns with a universal match on the current operand,
     * link to the match for the next operand.  When there are no more
     * operands to match, this links to the value slot. */
    cuex_qcode_t var_qcode;
    cuex_ssfn_node_t var_cntn;

    cuex_ssfn_seqno_t seqno;
};

struct cuex_ssfn_s
{
    cu_inherit (cuex_ssfn_node_s);
};


/* Control and returned status of some of the search algorithms. */
typedef enum {
    cuex_ssfn_ctrl_unmatched	= 0,	/* no match; continue with any */
    cuex_ssfn_ctrl_cont_delete	= 1,	/* delete;   continue with any */
    cuex_ssfn_ctrl_continue	= 2,	/* match;    continue with any */
    cuex_ssfn_ctrl_cut_mg	= 4,	/* match;    continue with lger */
    cuex_ssfn_ctrl_commit	= 6,	/* match;    committed choice */
    cuex_ssfn_ctrl_fail		= 8,	/* error;    committed choice */
} cuex_ssfn_ctrl_t;
#define cuex_ssfn_ctrl_do_delete(ctrl)	((ctrl) & 1)
#define cuex_ssfn_ctrl_do_cont(ctrl)	((ctrl) <= cuex_ssfn_ctrl_cut_mg)

/* Create a structural semantic function which matches nothing. */
void cuex_ssfn_cct(cuex_ssfn_t ssfn);
cuex_ssfn_t cuex_ssfn_new(void);

/* Make memory allocated by 'ssfn' available for GC. */
void cuex_ssfn_dct(cuex_ssfn_t ssfn);

void cuex_ssfn_cct_copy_mem(cuex_ssfn_t dst, cuex_ssfn_t src,
			    size_t slot_size,
			    cu_clop(slot_cct_copy, void, void *, void *));
cuex_ssfn_t
cuex_ssfn_new_copy_mem(cuex_ssfn_t src,
		       size_t slot_size,
		       cu_clop(slot_cct_copy, void, void *, void *));

/* Insert 'patn' into 'ssfn', where variables in 'patn' are
 * interpreted as wildcards.  'size' is the size to allocate for
 * '*slot_o', including the 'cuex_ssfn_slot_s' base struct.
 * 'var_cnt_o' and 'var_arr_o' is the size and contents of an array of
 * the variables appearing in 'patn' with 'extra_var_cnt' extra
 * uninitialised slots at the end (also counted).  */
cu_bool_t cuex_ssfn_insert_mem(cuex_ssfn_t ssfn, cuex_t patn,
			       cuex_ssfn_seqno_t seqno,
			       size_t slot_size, cu_ptr_ptr_t slot_o,
			       cu_count_t *var_cnt_o, cuex_var_t **var_arr_o);

/* Find the most specific matches of 'key' in 'ssfn'.  The algorithm
 * will call 'out' for each match until 'out' returns true.
 * Thereafter, it will call 'out' for matches which are structurally
 * at least as specific as that for which 'out' returned true for the
 * first time.  */
void cuex_ssfn_find(cuex_ssfn_t ssfn, cuex_t key,
		    cu_clop(out, cu_bool_t, cu_count_t arg_cnt,
					    cuex_t *arg_arr, void *slot));

void
cuex_ssfn_intersection_find_aux(
    cuex_ssfn_t ssfn_first,
    cucon_list_t ssfn_list,
    cuex_t key,
    cu_clop(out, cu_bool_t, cu_count_t arg_cnt, cuex_t *arg_arr, void *slot));

/* For each pettern in 'ssfn' which compares to 'key' as given next,
 * call 'cb' with the unifier in 'subst' and '(pat_arg_cnt,
 * pat_arg_arr)'.  The comparions are
 *
 *     _mgu	match patterns which unifies with key
 *     _wpmgu	weak-preserving match of patterns which unifies with key
 *     _lgr	less general or (equal within a) renaming
 *     _mgr	more general or (equal within a) renaming
 *     _wpmgr	weak-preserving more general or (equal within) a renaming
 *
 * Weak-preserving means that no weak variables in the ssfn patterns
 * are substituted.  */
typedef struct cuex_ssfn_matchinfo_s *cuex_ssfn_matchinfo_t;
struct cuex_ssfn_matchinfo_s
{
    cu_bool_t is_wp;
};

/*!Closure pointer type used for callbacks for various search functions. */
typedef cu_clop(cuex_ssfn_find_cb_t,
		cuex_ssfn_ctrl_t,
		    void *slot, cuex_subst_t subst,
		    cu_rank_t pat_arg_cnt, cuex_t *pat_arg_arr,
		    cuex_ssfn_matchinfo_t mi);
cuex_ssfn_ctrl_t cuex_ssfn_find_mgu(cuex_ssfn_t ssfn, cuex_subst_t subst,
				    cuex_t key, cuex_ssfn_seqno_t seqno,
				    cuex_ssfn_find_cb_t cb);
cuex_ssfn_ctrl_t cuex_ssfn_find_wpmgu(cuex_ssfn_t ssfn, cuex_subst_t subst,
				      cuex_t key, cuex_ssfn_seqno_t seqno,
				      cuex_ssfn_find_cb_t cb);
cuex_ssfn_ctrl_t cuex_ssfn_find_lgr(cuex_ssfn_t ssfn, cuex_subst_t subst,
				    cuex_t key, cuex_ssfn_seqno_t seqno,
				    cuex_ssfn_find_cb_t cb);
cuex_ssfn_ctrl_t cuex_ssfn_find_mgr(cuex_ssfn_t ssfn, cuex_subst_t subst,
				    cuex_t key, cuex_ssfn_seqno_t seqno,
				    cuex_ssfn_find_cb_t cb);
cuex_ssfn_ctrl_t cuex_ssfn_find_wpmgr(cuex_ssfn_t ssfn, cuex_subst_t subst,
				      cuex_t key, cuex_ssfn_seqno_t seqno,
				      cuex_ssfn_find_cb_t cb);
cuex_ssfn_ctrl_t cuex_ssfn_find_eqr(cuex_ssfn_t ssfn, cuex_t key,
				    cu_clop(cb, cuex_ssfn_ctrl_t, void *slot));


/* Convenience
 * ----------- */

cu_bool_t cuex_ssfn_have_wpmgu(cuex_ssfn_t ssfn, cuex_subst_t subst,
			       cuex_t key, cuex_ssfn_seqno_t seqno);
cu_bool_t cuex_ssfn_have_wpmgr(cuex_ssfn_t ssfn, cuex_subst_t subst,
			       cuex_t key, cuex_ssfn_seqno_t seqno);

void cuex_ssfn_erase_lgr(cuex_ssfn_t ssfn, cuex_t key);
void cuex_ssfn_erase_mgr(cuex_ssfn_t ssfn, cuex_t key);
void cuex_ssfn_erase_wpmgr(cuex_ssfn_t ssfn, cuex_t key);

void *cuex_ssfn_find_single_wpmgr(cuex_ssfn_t ssfn, cuex_subst_t subst,
				  cuex_t key);

void cuex_ssfn_dump_keys(cuex_ssfn_t ssfn, FILE *out);


/* cuex_ssfn_find_it_t
 * ------------------ */

typedef struct cuex_ssfn_find_it_s *cuex_ssfn_find_it_t;
typedef struct cuexP_ssfn_find_jargs_s {
    struct cucon_stack input;
    struct cucon_stack output;
    struct cucon_stack sctr;
    int *sctr_most_specific;
    cu_clop(receiver, cu_bool_t, cu_count_t arg_cnt, cuex_t *arg_arr, void *);
    cu_count_t arg_cnt;
} *cuexP_ssfn_find_jargs_t;
struct cuex_ssfn_find_it_s
{
    struct cucon_stack stack;
    struct cuexP_ssfn_find_jargs_s jargs;
    void *slot;
    cu_count_t arg_cnt;
    cuex_t *arg_arr;
    int *sctr_this;
};

/* Create an iterator starting at the first match of 'key' in 'ssfn'. */
void cuex_ssfn_find_it_cct(cuex_ssfn_find_it_t it, cuex_ssfn_t ssfn, cuex_t);
cuex_ssfn_find_it_t cuex_ssfn_find_it_new(cuex_ssfn_t ssfn, cuex_t key);

/* Advance 'it' to the next match. */
void cuex_ssfn_find_it_advance(cuex_ssfn_find_it_t it);

/* True iff 'it' is the end-iterator. */
#define cuex_ssfn_find_it_is_end(it) \
	(CU_MARG(cuex_ssfn_find_it_t, it)->slot == NULL)

/* Inform that 'it' matches, which narrows to remaining search to
 * matches at least as specific as the match referred to by 'it'. */
void cuex_ssfn_find_it_report_match(cuex_ssfn_find_it_t it);

/* Return the arguments of the match referred to by 'it'. */
#define cuex_ssfn_find_it_arg_cnt(it) \
	(CU_MARG(cuex_ssfn_find_it_t, it)->arg_cnt)
#define cuex_ssfn_find_it_arg_arr(it) \
	(CU_MARG(cuex_ssfn_find_it_t, it)->arg_arr)

/* Return the slot of the match referred to by 'it'. */
#define cuex_ssfn_find_it_slot(it) \
	(CU_MARG(cuex_ssfn_find_it_t, it)->slot)

/* True iff the match referred to by 'it0' is more specific or equal
 * within a renaming as that referred to by 'it1'.  Pre:
 * 'cuex_ssfn_find_it_report_match' must have been called on both
 * iterators and they must be matches of the same concrete expression,
 * but they may be from different structural semantic functions. */
cu_bool_t cuex_ssfn_find_it_match_lgr(cuex_ssfn_find_it_t it0,
				      cuex_ssfn_find_it_t it1);

/*!@}*/
CU_END_DECLARATIONS

#endif
