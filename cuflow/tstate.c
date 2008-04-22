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

#include <cuflow/tstate.h>
#include <cu/thread.h>
#include <cu/memory.h>

static pthread_mutex_t cuflowP_tstate_chain_mutex = CU_MUTEX_INITIALISER;
static cu_dlink_t cuflowP_tstate_chain = NULL;

void cuflowP_exeq_init_tstate(cuflow_tstate_t);

static void
tstate_dct(cuflow_tstate_t tstate)
{
    cu_mutex_lock(&cuflowP_tstate_chain_mutex);
    cu_dlink_erase(cu_to(cu_dlink, tstate));
    cu_mutex_unlock(&cuflowP_tstate_chain_mutex);
}

static void
tstate_cct(cuflow_tstate_t tstate)
{
    cuflowP_exeq_init_tstate(tstate);
    cu_mutex_lock(&cuflowP_tstate_chain_mutex);
    if (cuflowP_tstate_chain)
	cu_dlink_insert_before(cuflowP_tstate_chain, cu_to(cu_dlink, tstate));
    else {
	cuflowP_tstate_chain = cu_to(cu_dlink, tstate);
	cu_dlink_init_singleton(cu_to(cu_dlink, tstate));
    }
    cu_mutex_unlock(&cuflowP_tstate_chain_mutex);
}


#ifdef CUCONF_HAVE_THREAD_KEYWORD

__thread cu_bool_t cuflowP_tstate_initialised = cu_false;
__thread struct cuflow_tstate_s cuflowP_tstate;

cu_clop_def0(tstate_cleanup, void)
{
    tstate_dct(&cuflowP_tstate);
}

void
cuflowP_tstate_initialise(void)
{
    tstate_cct(&cuflowP_tstate);
    cu_thread_atexit(tstate_cleanup);
    cuflowP_tstate_initialised = cu_true;
}

#else

pthread_key_t cuflowP_tstate_key;

cuflow_tstate_t
cuflowP_tstate_initialise(void)
{
    cuflow_tstate_t tstate = cu_gnew_u(struct cuflow_tstate_s);
    tstate_cct(tstate);
    pthread_setspecific(cuflowP_tstate_key, tstate);
    return tstate;
}

#endif

void
cuflowP_tstate_init()
{
#ifndef CUCONF_HAVE_THREAD_KEYWORD
    switch (pthread_key_create(&cuflowP_tstate_key,
			       (void (*)(void *))tstate_dct)) {
	case EAGAIN:
	    cu_errf("Out of thread-local keys.");
	    exit(2);
	case 0:
	    break;
	default:
	    cu_debug_unreachable();
    }
#endif
}
