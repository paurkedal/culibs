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
_tstate_init(cuP_tstate_t tls)
{
    memset(tls, 0, sizeof(struct cuP_tstate));

    tls->priority = 7; /* normal priority, cf cuflow/gworkq.h */
    tls->atexit_chain = NULL;

    /* Link state to global chain. */
    cu_mutex_lock(&cuP_global_mutex);
    tls->next = cuP_tstate_chain;
    cuP_tstate_chain = tls;
    cu_mutex_unlock(&cuP_global_mutex);
}

static void
_tstate_destruct(cuP_tstate_t tls)
{
    cuP_tstate_t *stp;

    /* Unlink state from global chain. */
    cu_mutex_lock(&cuP_global_mutex);
    for (stp = &cuP_tstate_chain;; stp = &(*stp)->next) {
	cu_debug_assert(*stp);
	if (*stp == tls) {
	    *stp = tls->next;
	    break;
	}
    }
    cu_mutex_unlock(&cuP_global_mutex);

    /* Free up resources. */
    if (tls->iconv_utf8_to_ucs4)
	iconv_close(tls->iconv_utf8_to_ucs4);
    if (tls->iconv_ucs4_to_utf8)
	iconv_close(tls->iconv_ucs4_to_utf8);
}

CU_THREADLOCAL_DEF(cuP_tstate, cuP_tstate, _tstate);

void
cuP_tstate_init(void)
{
    CU_THREADLOCAL_INIT(cuP_tstate, cuP_tstate, _tstate);
}
