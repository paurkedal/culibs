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

#include <cuflow/cont_common.h>

/* Diagnostics
 * ----------- */

void
cuflowP_fatal(char const* fmt, ...)
{
    va_list va;
    fprintf(stderr, "\n!! cuflow detected a fatal error:\n!! ");
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
    abort();
}

void
cuflowP_mismatched_flow(char const *where, cuflow_mode_t original_flow)
{
    cuflowP_fatal("In %s, Mismatched flow set/restore, %s changed to %s.\n",
	       where,
	       cuflow_mode_name(original_flow),
	       cuflow_mode_name(cuflow_current_mode()));
}



/* Thread-local state
 * ------------------ */

#ifdef CUCONF_ENABLE_THREADS

static pthread_key_t state_key;

cuflow_tstate_t
cuflow_tstate()
{
    cuflow_tstate_t st = pthread_getspecific(state_key);
    if (!st) {
	st = GC_MALLOC(sizeof(struct cuflow_tstate_s));
	memset(st, 0, sizeof(struct cuflow_tstate_s));
	pthread_setspecific(state_key, st);
    }
    return st;
}

void
cuflow_tstate_init()
{
    static int init = 0;
    if (!init) {
	init = 1;
	pthread_key_create(&state_key, 0);
    }
}

#else /* !CUCONF_ENABLE_THREADS */

struct cuflow_tstate_s _ccf_the_state;

void
cuflow_tstate_init()
{
    memset(&_ccf_the_state, 0, sizeof(struct cuflow_tstate_s));
}

#endif /* CUCONF_ENABLE_THREADS */



/* Flow Types
 * ---------- */

int
(cuflow_mode_allows_split_0)(cuflow_mode_t flow)
{
    return cuflow_mode_allows_split_0(flow);
}
int
(cuflow_mode_allows_split_gt_1)(cuflow_mode_t flow)
{
    return cuflow_mode_allows_split_gt_1(flow);
}
cuflow_mode_t
(cuflow_mode_which_allows_split_0)(cuflow_mode_t flow)
{
    return cuflow_mode_which_allows_split_0(flow);
}
cuflow_mode_t
(cuflow_mode_which_forbids_split_0)(cuflow_mode_t flow)
{
    return cuflow_mode_which_forbids_split_0(flow);
}
cuflow_mode_t
(cuflow_mode_which_allow_split_gt_1)(cuflow_mode_t flow)
{
    return cuflow_mode_which_allows_split_gt_1(flow);
}
cuflow_mode_t
(cuflow_mode_which_forbids_split_gt_1)(cuflow_mode_t flow)
{
    return cuflow_mode_which_forbids_split_gt_1(flow);
}


char const *
cuflow_mode_name(cuflow_mode_t flow)
{
    static char const *name[4] = {
	"deterministic",
	"semi-deterministic",
	"multi-solution",
	"nondeterministic"
    };
    return name[flow];
}

cuflow_mode_t
(cuflow_current_mode)(void)
{
    return cuflow_current_mode();
}

void
(cuflow_set_mode)(cuflow_mode_t newflow)
{
    cuflow_set_mode(newflow);
}

void
(cuflow_set_save_current_flow)(cuflow_mode_t newflow, cuflow_mode_t* oldflow)
{
    cuflow_tstate_t st = cuflow_tstate();
    *oldflow = st->flow;
    st->flow = newflow;
}

void
cuflow_tstate_save_current_flow(cuflow_tstate_t st, cuflow_mode_t newflow,
			    cuflow_mode_t *oldflow)
{
    *oldflow = st->flow;
    st->flow = newflow;
}

/* Continuations
 * ------------- */

void
cuflowP_save_stack(cuflow_continuation_t cont)
{
    D_PRINTF("Saving stack for continuation @ %p.\n", cont);
    assert(cont->up);
    assert(CUFLOW_STACK_DELTA*(cont->ptr_stack_item - cont->up->ptr_stack_item)
	   > 0);
    while (cont->up) {
	if (cont->stack_data != 0)
	    break;
	cont->stack_size
	    = CUFLOW_STACK_DELTA*(cont->ptr_stack_item - cont->up->ptr_stack_item);
	cont->stack_data = GC_MALLOC(cont->stack_size);
#if CUCONF_STACK_DIRECTION > 0
	memcpy(cont->stack_data, cont->up->ptr_stack_item, cont->stack_size);
#else
	memcpy(cont->stack_data, cont->ptr_stack_item, cont->stack_size);
#endif
	cuflowP_size_copied += cont->stack_size;
	cont = cont->up;
    }
}

/* Set the stack mark so that at least the frame of the caller will be
 * saved.  */
void
cuflowP_set_stack_mark(cuflow_continuation_t cont)
{
/*     cuflowP_stack_item_t si; */
    cont->ptr_stack_item = (cuflowP_stack_item_t*)&cont;
}

void
cuflow_continuation_print_stats()
{
    if (cuflowP_size_copied < 1024*20)
	printf("Copied %lu B.  ", (unsigned long)cuflowP_size_copied);
    else if (cuflowP_size_copied < 1024*1024*20)
	printf("Copied %lu kiB.  ", (unsigned long)cuflowP_size_copied >> 10);
    else
	printf("Copied %lu MiB.  ", (unsigned long)cuflowP_size_copied >> 20);
    printf("Investigated %ld possibilities.\n", (long)cuflowP_split_count);
    cuflowP_size_copied = cuflowP_split_count = 0;
}

void
cuflow_uncaught(cu_clop(on_ex, void, void *))
{
    LOCK_GSTATE();
    cuflowP_g_on_uncaught = on_ex;
    UNLOCK_GSTATE();
}

cu_clos_fun(cuflowP_call_cntn,
		cu_prot(void, void *arg_ptr))
{
    cu_clos_self(cuflowP_call_cntn);
    memcpy(self->result_ptr, arg_ptr, self->result_size);
    cuflow_continuation_call(&self->cont);
}

cu_clos_def(cuflowP_set_cc,
	    cu_prot(int, cu_clop(cc, void, void *), void *result_ptr),
	    (cu_clop(*cc_out, void, void *);))
{
    cu_clos_self(cuflowP_set_cc);
    *self->cc_out = cc;
    return 0;
}
int
cuflow_set_cc(cu_clop(*cc_out, void, void *),
	   void *result_ptr, size_t result_size)
{
    cuflowP_set_cc_t set_cc;
    set_cc.cc_out = cc_out;
    return cu_call_with_cc(cuflowP_set_cc_prep(&set_cc),
			   result_ptr, result_size);
}



/* Initialisation
 * -------------- */

size_t cuflowP_size_copied = 0;
size_t cuflowP_split_count = 0;

#ifdef CUCONF_ENABLE_THREADS
pthread_mutex_t cuflowP_g_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void
cuflowP_cntn_common_init()
{
    cuflow_tstate_t st;
    cuflow_tstate_init();
    st = cuflow_tstate();
    cuflowP_g_on_uncaught = cu_clop_null;

    /* continuation.c */
    st->flow_count = 1;
}
