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

#include <cu/tstate.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cu/thread.h>
#include <string.h>
#include <errno.h>

pthread_mutex_t cuP_global_mutex = CU_MUTEX_INITIALISER;
cuP_tstate_t cuP_tstate_chain = NULL;

static void
cuP_tstate_init_glck(cuP_tstate_t st)
{
    memset(st, 0, sizeof(struct cuP_tstate_s));

    st->priority = 7; /* normal priority, cf cuflow/gworkq.h */
    st->atexit_chain = NULL;

    /* Link state to global chain. */
    st->next = cuP_tstate_chain;
    cuP_tstate_chain = st;
}

void cuP_thread_run_atexit(cuP_tstate_t);

static void
cuP_tstate_dct(cuP_tstate_t st)
{
    cuP_tstate_t *stp;

    cuP_thread_run_atexit(st);

    /* Unlink state from global chain. */
    cu_mutex_lock(&cuP_global_mutex);
    for (stp = &cuP_tstate_chain;; stp = &(*stp)->next) {
	cu_debug_assert(*stp);
	if (*stp == st) {
	    *stp = st->next;
	    break;
	}
    }
    cu_mutex_unlock(&cuP_global_mutex);

    /* Free up resources. */
    if (st->iconv_utf8_to_ucs4)
	iconv_close(st->iconv_utf8_to_ucs4);
    if (st->iconv_ucs4_to_utf8)
	iconv_close(st->iconv_ucs4_to_utf8);
#ifndef CUCONF_HAVE_THREAD_KEYWORD
    cu_gfree_u(st);
#endif
}

#ifdef CUCONF_ENABLE_THREADS

pthread_key_t cuP_tstate_key;

#ifdef CUCONF_HAVE_THREAD_KEYWORD

__thread cu_bool_t cuP_tstate_initialised = 0;
__thread struct cuP_tstate_s cuP_tstate_var;

void
cuP_tstate_initialise_glck(void)
{
    cuP_tstate_init_glck(&cuP_tstate_var);
    cuP_tstate_initialised = 1;
    pthread_setspecific(cuP_tstate_key, &cuP_tstate_var);
	/* because we need the destructor */
}
void
cuP_tstate_initialise(void)
{
    cu_mutex_lock(&cuP_global_mutex);
    cuP_tstate_initialise_glck();
    cu_mutex_unlock(&cuP_global_mutex);
}

#else /* !CUCONF_HAVE_THREAD_KEYWORD */

cuP_tstate_t
cuP_tstate_new_glck()
{
    /* OBS, GC does not scan thread-specific data, so allocate it
     * uncollectable and free it from the pthread key destructor. */
    cuP_tstate_t tstate;
    tstate = cu_gnew_u(struct cuP_tstate_s);
    cuP_tstate_init_glck(tstate);
    switch (pthread_setspecific(cuP_tstate_key, tstate)) {
    case 0:
	break;
    case ENOMEM:
	cu_errf("Not enough memory to initialise thread-local data.");
	break;
    case EINVAL:
	cu_bugf("The culibs are not initialised.");
	abort();
    default:
	cu_debug_unreachable();
    }
    return tstate;
}

cuP_tstate_t
cuP_tstate_new()
{
    cuP_tstate_t tstate;
    cu_mutex_lock(&cuP_global_mutex);
    tstate = cuP_tstate_new_glck();
    cu_mutex_unlock(&cuP_global_mutex);
    return tstate;
}

#endif /* !CUCONF_HAVE_THREAD_KEYWORD */

void
cuP_tstate_init(void)
{
    switch (pthread_key_create(&cuP_tstate_key,
	    (void (*)(void *))cuP_tstate_dct)) {
    case EAGAIN:
	cu_errf("There are no pthread keys available for culibs, need one.");
	abort();
    case 0:
	break;
    default:
	cu_debug_unreachable();
	abort();
    }
}

#else /* !CUCONF_ENABLE_THREADS */

struct cuP_tstate_s cuP_tstate_global;

void
cuP_tstate_init(void)
{
    cuP_tstate_init_glck(&cuP_tstate_global);
}

#endif /* !CUCONF_ENABLE_THREADS */

