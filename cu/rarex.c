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

#include <cu/rarex.h>
#include <cu/tstate.h>
#include <cu/debug.h>
#include <cu/thread.h>
#include <pthread.h>

static pthread_cond_t rarex_reader_dorm = PTHREAD_COND_INITIALIZER;
static pthread_cond_t rarex_writer_dorm = PTHREAD_COND_INITIALIZER;


/* Return the number of threads which either
 *     (1) are accessing the the protected data
 *     (2) are about to jam while requesting access (possibly incl self)
 */
static unsigned int
_unaccounted_count(cu_rarex_t *rarex, unsigned int rarex_snap)
{
    cuP_tstate_t tstate;
    int jammed_cnt = 0;
    unsigned int total_cnt;
    total_cnt = rarex_snap / cuP_RAREX_WRITE_MULTIPLIER
	      + rarex_snap % cuP_RAREX_WRITE_MULTIPLIER;
    for (tstate = cuP_tstate_chain; tstate; tstate = tstate->next)
	if (tstate->jammed_on_rarex == rarex)
	    ++jammed_cnt;
    cu_debug_assert(jammed_cnt <= total_cnt);
    return total_cnt - jammed_cnt;
}

static void
_release_a_writer(cu_rarex_t *rarex)
{
    cuP_tstate_t tstate;
    for (tstate = cuP_tstate_chain;; tstate = tstate->next) {
	cu_debug_assert(tstate);
	if (tstate->jammed_on_rarex == rarex && tstate->jammed_on_write) {
	    tstate->jammed_on_rarex = NULL;
	    break;
	}
    }
    pthread_cond_broadcast(&rarex_writer_dorm);
}

static void
_release_all_readers(cu_rarex_t *rarex)
{
    cuP_tstate_t tstate;
    for (tstate = cuP_tstate_chain; tstate; tstate = tstate->next) {
	if (tstate->jammed_on_rarex == rarex) {
	    cu_debug_assert(!tstate->jammed_on_write);
	    tstate->jammed_on_rarex = NULL;
	}
    }
    pthread_cond_broadcast(&rarex_reader_dorm);
}

void
cuP_rarex_lock_read(cu_rarex_t *rarex)
{
    cuP_tstate_t tstate;
    unsigned int rarex_snap;
    cu_mutex_lock(&cuP_global_mutex);
    rarex_snap = AO_load_acquire(rarex);
    if (rarex_snap < cuP_RAREX_WRITE_MULTIPLIER) {
	if (rarex_snap > 1)
	    _release_all_readers(rarex);
    }
    else {
	if (_unaccounted_count(rarex, rarex_snap) == 1)
	    _release_a_writer(rarex);
	tstate = cuP_tstate_glck();
	tstate->jammed_on_rarex = rarex;
	tstate->jammed_on_write = cu_false;
	while (tstate->jammed_on_rarex)
	    pthread_cond_wait(&rarex_reader_dorm, &cuP_global_mutex);
    }
    cu_mutex_unlock(&cuP_global_mutex);
}

void
cuP_rarex_lock_write(cu_rarex_t *rarex)
{
    cuP_tstate_t tstate;
    unsigned int rarex_snap;
    cu_mutex_lock(&cuP_global_mutex);
    rarex_snap = AO_load_acquire(rarex);
    if (_unaccounted_count(rarex, rarex_snap) > 1) {
	tstate = cuP_tstate_glck();
	tstate->jammed_on_rarex = rarex;
	tstate->jammed_on_write = cu_true;
	while (tstate->jammed_on_rarex)
	    pthread_cond_wait(&rarex_writer_dorm, &cuP_global_mutex);
    }
    cu_mutex_unlock(&cuP_global_mutex);
}

void
cuP_rarex_wake_up(cu_rarex_t *rarex)
{
    unsigned int rarex_snap;
    cu_mutex_lock(&cuP_global_mutex);
    rarex_snap = AO_load_acquire(rarex);
    if (rarex_snap < cuP_RAREX_WRITE_MULTIPLIER) {
	if (rarex_snap > 0)
	    _release_all_readers(rarex);
    }
    else if (_unaccounted_count(rarex, rarex_snap) == 0)
	_release_a_writer(rarex);
    cu_mutex_unlock(&cuP_global_mutex);
}
