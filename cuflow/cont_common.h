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

#ifndef CUFLOW_CNTN_COMMON_H
#define CUFLOW_CNTN_COMMON_H

#include <cuflow/fwd.h>

CU_BEGIN_DECLARATIONS

#include <cu/clos.h>
#include <cuflow/cont.h>
#include <cu/conf.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#if 1
#include <cu/conf.h>
#ifdef CUCONF_HAVE_GC_GC_H
#  include <gc/gc.h>
#else
#  include <gc.h>
#endif
#else
#  define GC_MALLOC malloc
#  define GC_NEW(T) malloc(sizeof(T))
#endif

/* #define CUFLOW_CONTINUATION_DEBUG */

#define CUCONF_ENABLE_FLOW_CHECK 1

#define CUFLOW_STACK_DELTA (CUCONF_STACK_DIRECTION*sizeof(cuflowP_stack_item_t))

#ifdef CUFLOW_CONTINUATION_DEBUG
#  define D_PRINTF(args...) fprintf(stderr, args)
#else
#  define D_PRINTF(args...) ((void)0)
#endif

void cuflowP_fatal(char const* fmt, ...) CU_ATTR_NORETURN;

void cuflowP_mismatched_flow(char const *where, cuflow_mode_t org_flow)
    CU_ATTR_NORETURN;

#define cu_debug_unreachable()					\
    do {								\
	fprintf(stderr,							\
		"%s:%d: This line should not have been reached.\n",	\
		__FILE__, __LINE__);					\
	abort();							\
    } while (0)

/*
 *  Global state
 */
#ifdef CUCONF_ENABLE_THREADS
#include <pthread.h>
extern pthread_mutex_t cuflowP_g_mutex;
#  define LOCK_GSTATE() pthread_mutex_lock(&cuflowP_g_mutex)
#  define UNLOCK_GSTATE() pthread_mutex_unlock(&cuflowP_g_mutex);
#else
#  define LOCK_GSTATE() ((void)0)
#  define UNLOCK_GSTATE() ((void)0)
#endif

#define CUFLOW_STACK_BREAK(PURPOSE, STATE, CONT)				\
    cuflow_tstate_t STATE = cuflow_tstate();					\
    cuflow_continuation_t CONT = GC_NEW(struct cuflow_continuation_s);	\
    cuflowP_set_stack_mark(CONT);						\
    CONT->kind = cuflowP_cntn_kind_##PURPOSE;				\
    CONT->flags = 0;							\
    CONT->level = st->onstack_cont->level + 1;				\
    CONT->up = st->onstack_cont;					\
    CONT->stack_data = 0;

cu_clos_edcl(cuflowP_call_cntn,
	     cu_prot(void, void *arg_ptr),
    ( struct cuflow_continuation_s cont;
      void *result_ptr;
      size_t result_size; ));

void cuflowP_save_stack(cuflow_continuation_t cont);
void cuflowP_set_stack_mark(cuflow_continuation_t cont);

cu_clop(cuflowP_g_on_uncaught, void, void *);
extern size_t cuflowP_size_copied;
extern size_t cuflowP_split_count;

CU_END_DECLARATIONS

#endif
