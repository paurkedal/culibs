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

#include <cuflow/cont_common.h>


/* Continuation
 * ------------ */

void
cuflow_call_in_root(cu_clop0(trunk, void), cuflow_mode_t trunk_flow)
{
    cuflow_mode_t cur_flow;
    cuflow_tstate_t st = cuflow_tstate();
    struct cuflow_continuation_s root;
    cuflow_continuation_t up = st->onstack_cont;
    root.kind = cuflowP_cntn_kind_frame;
    root.flags = 0;
    root.level = 1;
    root.up = 0;
    root.stack_data = 0;
    root.on_entry = cu_clop_null;
    root.on_exit = cu_clop_null;
    root.on_xc = cu_clop_null;
#if 0
    {
	cuflowP_stack_item_t si;
	/* This is a safe bet, but... */
	root.ptr_stack_item = &si;
    }
#else
    /* ... it may be sufficient to copy from the stack of this
     * function call with one pointer arg, since longjmps will be to
     * within function calls inside the trunk function call. */
    cuflowP_set_stack_mark(&root);
#endif
    assert(setjmp(root.door) == 0); /* initalise door only when debugging */
    st->onstack_cont = &root;
    cur_flow = cuflow_tstate_current_flow(st);

    cuflow_tstate_set_current_flow(st, trunk_flow);
    cu_call0(trunk);
#ifdef CUCONF_ENABLE_FLOW_CHECK
    if (cuflow_tstate_current_flow(st) != trunk_flow)
	cuflowP_mismatched_flow("trunk called from cuflow_call_in_root",
				trunk_flow);
#endif
    cuflow_tstate_set_current_flow(st, cur_flow);
    st->onstack_cont = up;
}

void
cuflow_dynamic_wind(cu_clop0(on_entry, void),
		 cu_clop0(trunk, void),
		 cu_clop0(on_exit, void),
		 cuflow_mode_t trunk_flow)
{
    cuflow_mode_t flow;
    CUFLOW_STACK_BREAK(frame, st, cont);
    cont->on_entry = on_entry;
    cont->on_exit = on_exit;
    cont->on_xc = cu_clop_null;

    /* XXX maybe change CF in 'on_entry' and 'on_exit' to det(?). */
    st->onstack_cont = cont;
    if (!cu_clop_is_null(on_entry))
	cu_call0(on_entry);
    flow = cuflow_tstate_current_flow(st);
    cuflow_tstate_set_current_flow(st, trunk_flow);
    cu_call0(trunk);
#ifdef CUCONF_ENABLE_FLOW_CHECK
    if (cuflow_tstate_current_flow(st) != trunk_flow)
	cuflowP_mismatched_flow("trunk called from cuflow_dynamic_wind", trunk_flow);
#endif
    cuflow_tstate_set_current_flow(st, flow);
    if (!cu_clop_is_null(on_exit))
	cu_call0(on_exit);
    st->onstack_cont = cont->up;
}

void
cuflow_catch(cu_clop0(trunk, void), cu_clop(on_xc, void, cu_ptr_t))
{
    CUFLOW_STACK_BREAK(catch, st, cont);
    cont->on_entry = cu_clop_null;
    cont->on_xc = on_xc;

    st->onstack_cont = cont;
    if (setjmp(cont->door) == 0) {
	cuflow_mode_t cur_flow = cuflow_tstate_current_flow(st);
	if (!cuflow_mode_allows_split_0(cur_flow)) {
	    cuflow_mode_t trunk_flow = cuflow_mode_which_allows_split_0(cur_flow);
	    cuflow_tstate_set_current_flow(st, trunk_flow);
	    cu_call0(trunk);
#ifdef CUCONF_ENABLE_FLOW_CHECK
	    if (cuflow_tstate_current_flow(st) != trunk_flow)
		cuflowP_mismatched_flow("trunk called from cuflow_catch",
				     trunk_flow);
#endif
	    cuflow_tstate_set_current_flow(st, cur_flow);
	}
	else {
	    cu_call0(trunk);
#ifdef CUCONF_ENABLE_FLOW_CHECK
	    if (cuflow_tstate_current_flow(st) != cur_flow)
		cuflowP_mismatched_flow("trunk called from cuflow_catch",
				     cur_flow);
#endif
	}
	st->onstack_cont = cont->up;
    }
    else {
	st->onstack_cont = cont->up;
	cu_call(on_xc, st->exception);
    }
}

int
cuflow_for_catchers(cu_clop(proc, int, cu_clop(, void, cu_ptr_t)))
{
    cuflow_tstate_t st = cuflow_tstate();
    cuflow_continuation_t cont = st->onstack_cont;
    while (cont) {
	if (!cu_clop_is_null(cont->on_xc)) {
	    int i = cu_call(proc, cont->on_xc);
	    if (i >= 0)
		return i;
	}
	cont = cont->up;
    }
    return cuflow_for_catchers_next;
}

int
cuflow_call_with_cc(cu_clop(trunk, int, cu_clop(, void, void *), void *),
		    void *result_ptr, size_t result_size)
{
    int r;
    cuflow_tstate_t st = cuflow_tstate();
    cuflowP_call_cntn_t *cntn_clos = GC_malloc(sizeof(cuflowP_call_cntn_t));
    cuflow_mode_t cur_flow = cuflow_tstate_current_flow(st);
#ifdef CUCONF_ENABLE_FLOW_CHECK
    if (!cuflow_mode_allows_split_gt_1(cur_flow))
	cuflowP_fatal("Attemted to create a continuation of point with "
		   "%s flow.\n", cuflow_mode_name(cur_flow));
#endif
    D_PRINTF("Created continuation closure %p.\n", cntn_clos);
    assert(st->onstack_cont);
    cntn_clos->cont.kind = cuflowP_cntn_kind_continuation;
    cntn_clos->cont.flags = 0;
    cntn_clos->cont.up = st->onstack_cont;
    cntn_clos->cont.level = st->onstack_cont->level + 1;
    cntn_clos->cont.stack_data = 0;
    cntn_clos->result_ptr = result_ptr;
    cntn_clos->result_size = result_size;
    cuflowP_set_stack_mark(&cntn_clos->cont);

    if (setjmp(cntn_clos->cont.door) == 0) {
	st->onstack_cont = &cntn_clos->cont;
	r = cu_call(trunk, cuflowP_call_cntn_prep(cntn_clos), result_ptr);
#ifdef CUCONF_ENABLE_FLOW_CHECK
	if (cuflow_tstate_current_flow(st) != cur_flow)
	    cuflowP_mismatched_flow("trunk called from cuflow_call_with_cc",
				    cur_flow);
#endif
    }
    else
	r = INT_MIN;
    if (cuflow_continuation_is_valid(&cntn_clos->cont))
	cuflowP_save_stack(&cntn_clos->cont);
    st->onstack_cont = cntn_clos->cont.up;
    return r;
}

int
cuflow_split(int n)
{
    cuflow_tstate_t st;
    if (n == 1)
	return 0;

    st = cuflow_tstate();
    if (n == 0) {
#ifdef CUCONF_ENABLE_FLOW_CHECK
	if (!cuflow_mode_allows_split_0(cuflow_tstate_current_flow(st)))
	    cuflowP_fatal("chi_split(0) called from %s flow.\n",
		       cuflow_mode_name(cuflow_tstate_current_flow(st)));
#endif
	if (st->split_cont) {
	    ++cuflowP_split_count;
	    cuflow_continuation_call(st->split_cont);
	}
	else {
	    assert(!"XXX not implemented.");
	}
	cu_debug_unreachable();
    }
    else {
	cuflow_continuation_t cont, prev_split;
#ifdef CUCONF_ENABLE_FLOW_CHECK
	if (!cuflow_mode_allows_split_gt_1(cuflow_tstate_current_flow(st)))
	    cuflowP_fatal("chi_split(%d) called from %s flow.\n",
		       n, cuflow_mode_name(cuflow_tstate_current_flow(st)));
#endif
	D_PRINTF("cuflow_split(%d), st = %p\n", n, st);
	assert(st->onstack_cont);
	cont = GC_MALLOC(sizeof(struct cuflow_continuation_s) + sizeof(int));
	D_PRINTF("cont = %p, st = %p\n", cont, st);
	cont->kind = cuflowP_cntn_kind_continuation;
	cont->flags = 0;
	cont->up = st->onstack_cont;
	cont->level = st->onstack_cont->level + 1;
	cont->stack_data = 0;
	cont->on_entry = cu_clop_null;
	cont->on_exit = cu_clop_null;
	cont->on_xc = cu_clop_null;
	CUFLOW_CONTINUATION_RESULT(cont, int) = 0;
	prev_split = st->split_cont;
	st->split_cont = cont;
	++st->flow_count;

	D_PRINTF("cuflow_split: cont = %p, st = %p\n", cont, st);
	if (setjmp(cont->door) == 0) {
	    cuflowP_set_stack_mark(cont);
	    cuflowP_save_stack(cont);
	    D_PRINTF("cont = %p, st = %p\n", cont, st);
	    return 0;
	}
	else {
	    int i = ++CUFLOW_CONTINUATION_RESULT(cont, int);
	    if (i == n - 1) {
		st->split_cont = prev_split;
		--st->flow_count;
	    }
	    st->onstack_cont = cont->up;
	    return i;
	}
    }
}

static void recreate_frame_and_call(cuflow_continuation_t cont) CU_ATTR_NORETURN;

cuflowP_stack_item_t* cuflowP_continuation_dummy;

static void
recreate_frame_and_call_0(cuflow_continuation_t cont)
{
    cuflowP_stack_item_t items[1000];
    cuflowP_continuation_dummy = items;
    recreate_frame_and_call(cont);
}

static void
recreate_frame_and_call(cuflow_continuation_t cont)
{
    cuflow_tstate_t st = cuflow_tstate();
    cuflowP_stack_item_t stack_item;
    cuflow_continuation_t cont1, onstack;
    ptrdiff_t need = CUFLOW_STACK_DELTA*(cont->ptr_stack_item - &stack_item);
    if (need > 0)
	recreate_frame_and_call_0(cont);
    D_PRINTF("Restoring stack for continuation @ %p.\n", cont);

    /* Fix the 'down' links for the new stack state. */
    onstack = st->onstack_cont;
    cont->down = 0;
    for (cont1 = cont; cont1->up; cont1 = cont1->up) {
	if (cont1 == onstack)
	    break;
	cont1->up->down = cont1;
    }

    /* Restore the stack pieces, 'on_entry' code, and 'longjmp'. */
    for (cont1 = onstack->down; cont1; cont1 = cont1->down) {
	assert(cont1->stack_data);
#if CUCONF_STACK_DIRECTION > 0
	memcpy(cont1->up->ptr_stack_item, cont1->stack_data, cont1->stack_size);
#else
	memcpy(cont1->ptr_stack_item, cont1->stack_data, cont1->stack_size);
#endif
	cuflowP_size_copied += cont1->stack_size;
	st->onstack_cont = cont1;
	if (!cu_clop_is_null(cont1->on_entry))
	    cu_call0(cont1->on_entry);
    }
    longjmp(cont->door, cont->level);
}

void
cuflow_continuation_call(cuflow_continuation_t cont)
{
    cuflow_tstate_t st = cuflow_tstate();
    cuflow_continuation_t cont0, cont1;
    D_PRINTF("Calling continuation @ %p [%d]\n", cont, cont->level);

#ifdef CUCONF_ENABLE_FLOW_CHECK
    if (!cuflow_mode_allows_split_0(cuflow_tstate_current_flow(st)))
	cuflowP_fatal("Continuation called in %s flow.\n",
		   cuflow_mode_name(cuflow_tstate_current_flow(st)));
#endif

    if (!cuflow_continuation_is_valid(cont))
	cuflowP_fatal("Called invalidated continuation.\n"); /* XXX or throw */

    /* Spin up to the first common stack frame. */
    for (cont1 = st->onstack_cont; cont1->level > cont->level;
	 cont1 = cont1->up)
	if (!cu_clop_is_null(cont1->on_exit))
	    cu_call0(cont1->on_exit);
    for (cont0 = cont; cont0->level > cont1->level; cont0 = cont0->up)
	(void)0;
    for (; cont0 != cont1; cont0 = cont0->up, cont1 = cont1->up) {
	if (cont0 == NULL || cont1 == NULL)
	    cuflowP_fatal("Called continuation which does not have a common "
		       "root with the current stack.");
	if (!cu_clop_is_null(cont1->on_exit))
	    cu_call0(cont1->on_exit);
    }

    /* Then 'longjmp' if on-stack, or spin down to the target stack
     * frame and 'longjmp'. */
    if (cont1 == cont)
	longjmp(cont->door, cont->level);
    recreate_frame_and_call(cont);
}

void
cuflow_throw(cu_ptr_t exception)
{
    cuflow_continuation_t cont1;
    cuflow_tstate_t st = cuflow_tstate();

#ifdef CUCONF_ENABLE_FLOW_CHECK
    if (!cuflow_mode_allows_split_0(cuflow_tstate_current_flow(st)))
	cuflowP_fatal("Exception thrown while in %s flow.\n",
		   cuflow_mode_name(cuflow_tstate_current_flow(st)));
#endif

    /* Save the stack frame for backtrace if requested.  We could have
     * waited with unwinding the stack until a catcher exits without
     * rethrowing, thought this will complicate handling of the much
     * used (due to rethrows) throws inside catchers. */
    if (st->opt_uncaught_backtrace) {
	D_PRINTF("Saving exception stack.\n");
	if (cuflow_set_cc(&st->uncaught_backtrace, NULL, 0)) {
	    fprintf(stderr,
		    "Uncaught exception.  "
		    "The stack is restored for backtrace.\n");
	    if (!cu_clop_is_null(cuflowP_g_on_uncaught))
		cu_call(cuflowP_g_on_uncaught, exception);
	    abort();
	}
    }

    st->exception = exception;

    /* Spin up to the next catcher and jump there. */
    cont1 = st->onstack_cont;
    while (cont1) {
	cuflow_continuation_invalidate(cont1);
	if (!cu_clop_is_null(cont1->on_exit))
	    cu_call0(cont1->on_exit);
	if (!cu_clop_is_null(cont1->on_xc))
	    longjmp(cont1->door, cont1->level);
	cont1 = cont1->up;
    }

    /* Then the exception is uncaught. */
    if (!cu_clop_is_null(st->uncaught_backtrace)) {
	D_PRINTF("Uncaught exception.  "
		 "Trying to restore stack for backtrace.\n");
	cu_call(st->uncaught_backtrace, NULL);
	abort();
    }
    else {
	fprintf(stderr,
		"Uncaught exception.  Too see the full backtrace in a "
		"debugger, enable\nexception stack restore.\n");
	if (!cu_clop_is_null(cuflowP_g_on_uncaught))
	    cu_call(cuflowP_g_on_uncaught, exception);
	abort();
    }
}
