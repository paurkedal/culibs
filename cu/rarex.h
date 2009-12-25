/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_RAREX_H
#define CU_RAREX_H

#include <cu/fwd.h>
#include <atomic_ops.h>
#include <cu/tstate.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cu_rarex_h cu/rarex.h: Read-Write Locks Optimised for Rarely Excluding Cases
 ** @{ \ingroup cu_util_mod
 **
 ** Rarices are read-write locks implemented with a single word per lock (in
 ** addition to some per-thread data).  They are cheap as long as they are
 ** read-locked, or write access do not conflict with another read or write,
 ** but significantly more expensive than mutices when they clog.  In other
 ** words, use this only when two threads hardly ever compete for the same
 ** lock.
 ** 
 ** If each of <i>N</i> threads spends <i>p</i> of its time accessing the lock
 ** (exclusive or not), then \f$q = (1 - p)^{N - 1}\f$ is the probability that
 ** a thread can acquire a write lock without delay.  Note the locking time is
 ** included in \f$p\f$.  For good efficiency keep \f$p \ll \frac1N\f$.
 ** 
 ** If unsure, use \c pthread_mutex_t or similar instead.
 **/

/** \ingroup cuP_mod */
#define cuP_RAREX_WRITE_MULTIPLIER 0x10000

/** Construct the \a rarex lock. */
CU_SINLINE void
cu_rarex_init(cu_rarex_t *rarex) { AO_store(rarex, 0); }

void cuP_rarex_lock_read(cu_rarex_t *);	/*!<\private*/
void cuP_rarex_lock_write(cu_rarex_t *);	/*!<\private*/
void cuP_rarex_wake_up(cu_rarex_t *);	/*!<\private*/

/** Lock for non-exclusive access.  May be called recursively, and should have
 ** matching calls to cu_rarex_unlock_read. */
CU_SINLINE void
cu_rarex_lock_read(cu_rarex_t *rarex)
{
    if (AO_fetch_and_add1_acquire_read(rarex) >= cuP_RAREX_WRITE_MULTIPLIER)
	cuP_rarex_lock_read(rarex);
}

/** Unlock non-exclusive access. */
CU_SINLINE void
cu_rarex_unlock_read(cu_rarex_t *rarex)
{
    unsigned int snap = AO_fetch_and_sub1_release(rarex);
    if (snap > cuP_RAREX_WRITE_MULTIPLIER)
	cuP_rarex_wake_up(rarex);
}

/** Try to lock for non-exclusive access without blocking.  Returns true iff
 ** successful. */
CU_SINLINE cu_bool_t
cu_rarex_trylock_read(cu_rarex_t *rarex)
{
    if (AO_fetch_and_add1_acquire_read(rarex)
	    >= cuP_RAREX_WRITE_MULTIPLIER) {
	cu_rarex_unlock_read(rarex);
	return cu_false;
    }
    else
	return cu_true;
}

/** Lock for exclusive access.  Caller must not own any access to any
 ** cu_rarex_t locks advance. */
CU_SINLINE void
cu_rarex_lock_write(cu_rarex_t *rarex)
{
    if (AO_fetch_and_add_acquire(rarex, cuP_RAREX_WRITE_MULTIPLIER) > 0)
	cuP_rarex_lock_write(rarex);
}

/** Try to lock for exclusive access without blocking.  Returns true iff
 ** successful. */
CU_SINLINE cu_bool_t
cu_rarex_trylock_write(cu_rarex_t *rarex)
{
    return AO_compare_and_swap_acquire(rarex, 0, cuP_RAREX_WRITE_MULTIPLIER);
}

/** Release exclusive access. */
CU_SINLINE void
cu_rarex_unlock_write(cu_rarex_t *rarex)
{
    unsigned int snap;
    snap = AO_fetch_and_add_release_write(rarex,
					  -cuP_RAREX_WRITE_MULTIPLIER);
    if (snap > cuP_RAREX_WRITE_MULTIPLIER)
	cuP_rarex_wake_up(rarex);
}

/** Attempts to promote a read lock to a write lock.  Returns non-zero on
 ** success, otherwise the lock remains for read-access.  Note that depending
 ** on the outcome, caller must either use \ref cu_rarex_unlock_read or \ref
 ** cu_rarex_unlock_write to unlock. */
CU_SINLINE cu_bool_t
cu_rarex_try_promote(cu_rarex_t *rarex)
{
    return AO_compare_and_swap_full(rarex, 1, cuP_RAREX_WRITE_MULTIPLIER);
}

/*!\deprecated Use cu_rarex_init. */
#define cu_rarex_cct cu_rarex_init

/** @} */
CU_END_DECLARATIONS

#endif
