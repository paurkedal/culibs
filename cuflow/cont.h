/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUFLOW_CONTINUATION_H
#define CUFLOW_CONTINUATION_H

#include <stdlib.h>
#include <setjmp.h>
#include <cu/clos.h>
#include <cuflow/fwd.h>

CU_BEGIN_DECLARATIONS

/* CONTINUATIONS.  Low-level functions
 * ----------------------------------- */

typedef long cuflowP_stack_item_t;

typedef enum {
    cuflowP_cntn_kind_frame,
    cuflowP_cntn_kind_continuation,
    cuflowP_cntn_kind_catch
} cuflowP_cntn_kind_t;

#define cuflowP_cntn_flag_invalid 1
struct cuflow_continuation
{
    unsigned int	kind : 8;  /* Actual type is cuflowP_cntn_kind_t. */
    unsigned int	flags : 8;
    cuflow_continuation_t up;
    cuflow_continuation_t down;
    jmp_buf		door;
    int			level;
    cuflowP_stack_item_t *ptr_stack_item;
    size_t		stack_size;
    char *		stack_data;
    cu_clop0(on_entry, void);
    cu_clop0(on_exit, void);
    cu_clop(on_xc, void, void *);
};

#define cuflow_continuation_invalidate(cont) \
    (CU_MARG(cuflow_continuation_t, (cont))->flags |= cuflowP_cntn_flag_invalid)
#define cuflow_continuation_is_valid(cont) \
    (!(CU_MARG(cuflow_continuation_t, (cont))->flags & cuflowP_cntn_flag_invalid))

#define cuflow_continuation_result_ptr(cont) \
	((void*)(CU_MARG(cuflow_continuation_t, (cont)) + 1))
#define CUFLOW_CONTINUATION_RESULT(cont, T) \
	(*(T*)(CU_MARG(cuflow_continuation_t, (cont)) + 1))
#define CUFLOW_CONTINUATION_APPLY(cont, T, x) \
	(CUFLOW_CONTINUATION_RESULT((cont), T) = x, cuflow_continuation_call(cont))
void cuflow_continuation_call(cuflow_continuation_t cont) CU_ATTR_NORETURN;
/* void cuflow_continuation_throw(cuflow_continuation_t cont); */
void cuflow_continuation_print_stats();


/* CONTINUATION.  Interface
 * ------------------------ */

/* The control flow category.  This is similar to determinism modes,
 * but not quiet the same.  The CF categories only describes the flow
 * of control, and does not care about logic.  Determinism modes
 * requires a deeper analysis of the logic of the language, after
 * which it should be possible to utilize this low-level interface in
 * the generated code. */
typedef enum {		/* _____________________|__allowed_splits__*/
    cuflow_mode_det,	/* deterministic	|        1	   */
    cuflow_mode_semidet,	/* semideterministic	|       0,1	   */
    cuflow_mode_multi,	/* multisolution	|      1,...	   */
    cuflow_mode_nondet,	/* nondeterministic	|      0,...	   */
    /*cuflow_mode_failure,*//* failure		|        0	   */
} cuflow_mode_t;
/* Cf. the determinism categorization of Mercury:
 *
 *   ___________________|__can_fail_|_solutions__
 *   deterministic	|     no    |     1
 *   semideterministic	|    yes    |    0,1
 *   multisolution	|     no    |    1,...
 *   nondeterministic	|    yes    |    0,...
 *   failure		|    yes    |     0
 *   erroneous		|     no    |     0
 *
 * In this implementation, 'can fail' is not interesting.
 *
 * The purpose of the control flow is consistency checking, and to
 * allow programs to probe the current flow and decide how to solve
 * the problem accordingly.
 *
 * 'cuflow_mode_det' means that 'cuflow_split' can be called only with 1 as
 * an argument (which does nothing).  'cuflow_mode_semidet' and
 * 'cuflow_mode_nondet' allows 'cuflow_split(0)', and 'cuflow_mode_multi' and
 * 'cuflow_mode_nondet' allows 'cuflow_split' with arguments higher that 1.
 * Any other combination will result in hard errors.
 *
 * Note that exceptions are part of the control flow and are analogous
 * to 'cuflow_split(0)', so setting the flow to 'cuflow_mode_det' or
 * 'cuflow_mode_multi' implies that exceptions are hard errors unless
 * catched within another sub-flow.  The purpose of this is to help
 * debugging exception safety of code with mandatory cleanup.
 *
 * The usual program flow for a C program would be 'cuflow_mode_det',
 * since C has no exceptions, and 'longjmp' is seldom used.  When
 * libccf exceptions are used 'cuflow_mode_semidet' is the default. */

int cuflow_mode_allows_split_0(cuflow_mode_t);
#define cuflow_mode_allows_split_0(flow) ((flow) & 1)
int cuflow_mode_allows_split_gt_1(cuflow_mode_t);
#define cuflow_mode_allows_split_gt_1(flow) ((flow) & 2)

cuflow_mode_t cuflow_mode_which_allows_split_0(cuflow_mode_t);
#define cuflow_mode_which_allows_split_0(flow) ((cuflow_mode_t)((flow) | 1))
cuflow_mode_t cuflow_mode_which_forbids_split_0(cuflow_mode_t);
#define cuflow_mode_which_forbids_split_0(flow) ((cuflow_mode_t)((flow) & ~1))

cuflow_mode_t cuflow_mode_which_allows_split_gt_1(cuflow_mode_t);
#define cuflow_mode_which_allows_split_gt_1(flow) ((cuflow_mode_t)((flow | 2)))
cuflow_mode_t cuflow_mode_which_forbids_split_gt_1(cuflow_mode_t);
#define cuflow_mode_which_forbids_split_gt_1(flow) ((cuflow_mode_t)((flow & ~2)))

char const *cuflow_mode_name(cuflow_mode_t);

/* Return the current control flow kind. */
cuflow_mode_t cuflow_current_mode(void);
#define cuflow_current_mode() ((cuflow_mode_t const)(cuflow_tstate())->flow)

/* Set the kind of control flow.  This may be slightly faster than
 * 'cuflow_set_save_current_flow' if old flow is not required. */
void	cuflow_set_current_mode(cuflow_mode_t newflow);
#define	cuflow_set_current_mode(newflow) ((void)(cuflow_tstate()->flow = (newflow)))

/* Assign old flow to '*oldflow' and switch to 'newflow'.  This may be
 * faster (for a multithreaded application) than calling both
 * 'cuflow_current_mode' and 'cuflow_set_current_mode'.  */
void	cuflow_set_save_current_flow(cuflow_mode_t newflow, cuflow_mode_t *oldflow);
#ifndef CUCONF_ENABLE_THREADS
#define	cuflow_set_save_current_flow(newflow, oldflow) \
	(*(oldflow) = cuflow_current_mode(), cuflow_set_current_mode(newflow))
#endif

/* Call
 *
 *     void (__closure trunk)(void)
 *
 * in a root frame, i.e. with a guarantee that no continuation created
 * therein is used outside this call.  This also disallows multireturn
 * of 'trunk', so flow is degraded to 'cuflow_mode_det' or
 * 'cuflow_mode_semidet' while evaluating 'trunk'. */
void cuflow_call_in_root(cu_clop0(trunk, void), cuflow_mode_t trunk_flow);

/* Call 'trunk' with a stack break at the call.  This means that the
 * stack above this call is only copied once even if the stack below
 * must be copied several times.  */
#define cu_call_in_frame(trunk, flow) \
    cuflow_dynamic_wind(cu_clop_null, (trunk), cu_clop_null, (flow))

/* Same semantics as Scheme's 'dynamic-wind'. */
void cuflow_dynamic_wind(cu_clop0(on_entry, void),
		      cu_clop0(trunk, void),
		      cu_clop0(on_exit, void),
		      cuflow_mode_t trunk_flow);

/* Call 'trunk' with the current continuation 'cc'.  This function can
 * return normally or multiple times due to calls to 'cc':
 *
 * (1) If it returns normally, 'cuflow_call_with_cc' returns the same
 * integer.  'res' is the same pointer as 'result_ptr'.
 *
 * (2) When 'cc' is called inside any stack frame with a common root
 * as the caller of 'cuflow_call_with_cc', 'cc' will copy the object
 * pointed to by 'arg', which is assumed to have size 'result_size',
 * to the location 'result_ptr', and cause a return from
 * 'cuflow_call_with_cc' with value 'INT_MIN'.  */
int
cuflow_call_with_cc(
    cu_clop(trunk, int, cu_clop(cc, void, void *arg), void *res),
    void *result_ptr, size_t result_size);

/* Set the '*cc_out' to the continuation of this function call.  On
 * immediate return, the result is 0.  When '*cc_out' is called with a
 * pointer, 'result_size' bytes from this pointer is copied to
 * 'result_ptr' and 'INT_MIN' is returned.  This is an extension of
 * setjmp that allows jumping back to the caller from any stack frame
 * which a common root as the caller of 'cuflow_set_cc'.  */
int cuflow_set_cc(cu_clop(*cc_out, void, void *),
	       void *result_ptr, size_t result_size);

/* Low-level exception handling.  You'll probably want to implement
 * some kind of polymorphism for the 'xc' objects.  Inside 'trunk',
 * the current control flow category is changed to allow
 * exceptions.  */
void cuflow_catch(cu_clop0(trunk, void),
		  cu_clop(on_xc, void, cu_ptr_t xc));
void cuflow_throw(cu_ptr_t xc) CU_ATTR_NORETURN;
void cuflow_uncaught(cu_clop(on_xc, void, void *));

/* Call 'proc' with each exception handler starting with most recent
 * and continuing as long as 'proc' returns 'cuflow_for_catchers_next'.
 * If 'proc' returns any non-negative value, than 'cuflow_for_catchers'
 * exits immediately with that value.  Otherwise if the catchers are
 * emptied, 'cuflow_for_catchers_next' is returned. */
#define cuflow_for_catchers_next -1
int cuflow_for_catchers(cu_clop(proc, int, cu_clop(, void, cu_ptr_t)));

/* Split control flow into n braches, return all values 0, 1, ..., n -
 * 1.  'cuflow_split(0)' is equivalent to 'cuflow_absorb()'.  'cuflow_split(1)'
 * just returns 0.  It is unspecified if changes of automatic
 * variables will take effect in parallel flows.  If needed, use
 * dynamic storage allocated before the split. */
int cuflow_split(int n);

/* Cut all possibilities leading to this branch.  This function does
 * not return; it leaves control to other possibilities or exits the
 * current frame if none (XXX). */
#define cuflow_absorb() ((void)cuflow_split(0))

/* Return the current number of nondeterministic control flows
 * including the current CF. */
#define cuflow_mode_count() (cuflow_tstate()->flow_count)



/* Thread Local State
 * ================== */

struct cuflow_tstate
{
    /* continuation.c */
    cuflow_continuation_t onstack_cont;
    cuflow_continuation_t split_cont;
    int flow_count;
    void *exception;
    cu_clop(uncaught_backtrace, void, void *);
    int opt_uncaught_backtrace;
    cuflow_mode_t flow;
};

#ifdef CUCONF_ENABLE_THREADS
cuflow_tstate_t cuflow_tstate();
#else
extern struct cuflow_tstate _ccf_the_state;
#define cuflow_tstate() (&_ccf_the_state)
#endif /* !CUCONF_ENABLE_THREADS */

#define cuflow_tstate_current_flow(st) (CU_MARG(cuflow_tstate_t, (st))->flow)
#define cuflow_tstate_set_current_flow(st, newflow) \
	((void)(CU_MARG(cuflow_tstate_t, (st))->flow = (newflow)))
void cuflow_tstate_save_current_flow(cuflow_tstate_t st, cuflow_mode_t newflow,
				 cuflow_mode_t *oldflow);

CU_END_DECLARATIONS

#endif
