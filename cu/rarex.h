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

#ifndef CU_RAREX_H
#define CU_RAREX_H

#include <cu/fwd.h>
#include <atomic_ops.h>
#include <cu/tstate.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_rarex cu/rarex.h: Read-Write Locks Optimised for Rarely Excluding Cases
 * @{ \ingroup cu_mod
 * Rarices are read-write locks implemented with a single word per lock
 * (in addition to some per-thread data).  They are cheap as long
 * as they are read-locked, or write access do not conflict with another
 * read or write, but significantly more expensive than mutices when they
 * clog.
 *
 * In other words, cu_rarex_t should be used only when you have good reason to
 * believe that access to the lock is hardly ever excludes another access, and
 * they can only beat good mutex implementations if the mutices would clog due
 * to exclusion on read access.
 *
 * \remark
 * If each of <i>N</i> threads spends <i>p</i>
 * of its time accessing the lock (exclusive or not),
 * then \f$q = (1 - p)^{N - 1}\f$ is the
 * probabilty for one thread to gain exclusive access.  So, the part spend
 * in a lock jam is \f$(1 - (1 - p)^{N - 1})p_{\mathrm w}\f$.
 * Note that <i>p</i> and <i>p</i><sub>w</sub> depend on the amount of
 * jamming, since handling the jam is more expensive than conflict-free
 * locking.
 *
 * Sometimes you can adapt your code to make this true.  For instance, since
 * the rarex is only a single word, you may put one on each element of the
 * array of a hash map.  If newly inserted elements have quasi-random hashes,
 * the chance of clogging a lock is very small.  On the other hand,
 * simultaneous multiple lookups are read-only and will not clog the locks.
 *
 * In most cases it is probably better to use mutices than adapting the
 * code, since they are fast on modern platforms (I think, I have
 * only tested on Linux x86).
 */

/*!\ingroup cuP_mod*/
#define cuP_RAREX_WRITE_MULTIPLIER 0x10000

/*!Construct the \a rarex lock. */
CU_SINLINE void
cu_rarex_cct(cu_rarex_t *rarex) { AO_store(rarex, 0); }

void cuP_rarex_lock_read(cu_rarex_t *);	/*!<\private*/
void cuP_rarex_lock_write(cu_rarex_t *);	/*!<\private*/
void cuP_rarex_wake_up(cu_rarex_t *);	/*!<\private*/

/*!Lock for non-exclusive access.  May be called recursively, and should have
 * matching calls to cu_rarex_unlock_read. */
CU_SINLINE void
cu_rarex_lock_read(cu_rarex_t *rarex)
{
    if (AO_fetch_and_add1_acquire_read(rarex) >= cuP_RAREX_WRITE_MULTIPLIER)
	cuP_rarex_lock_read(rarex);
}

/*!Unlock non-exclusive access. */
CU_SINLINE void
cu_rarex_unlock_read(cu_rarex_t *rarex)
{
    unsigned int snap = AO_fetch_and_sub1_release(rarex);
    if (snap > cuP_RAREX_WRITE_MULTIPLIER)
	cuP_rarex_wake_up(rarex);
}

/*!Try to lock for non-exclusive access without blocking.  Returns true iff
 * successful. */
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

/*!Lock for exclusive access.  Caller must not own any access to any
 * cu_rarex_t locks advance. */
CU_SINLINE void
cu_rarex_lock_write(cu_rarex_t *rarex)
{
    if (AO_fetch_and_add_acquire(rarex, cuP_RAREX_WRITE_MULTIPLIER) > 0)
	cuP_rarex_lock_write(rarex);
}

/*!Try to lock for exclusive access without blocking.  Returns true iff
 * successful. */
CU_SINLINE cu_bool_t
cu_rarex_trylock_write(cu_rarex_t *rarex)
{
    return AO_compare_and_swap_acquire(rarex, 0, cuP_RAREX_WRITE_MULTIPLIER);
}

/*!Release exclusive access. */
CU_SINLINE void
cu_rarex_unlock_write(cu_rarex_t *rarex)
{
    unsigned int snap;
    snap = AO_fetch_and_add_release_write(rarex,
					  -cuP_RAREX_WRITE_MULTIPLIER);
    if (snap > cuP_RAREX_WRITE_MULTIPLIER)
	cuP_rarex_wake_up(rarex);
}

/*!Attempts to promote a read lock to a write lock.  Returns non-zero on
 * success, otherwise the lock remains for read-access.  Note that depending
 * on the outcome, caller must either use cu_rarex_unlock_read or
 * cu_rarex_unlock_write to unlock. */
CU_SINLINE cu_bool_t
cu_rarex_try_promote(cu_rarex_t *rarex)
{
    return AO_compare_and_swap_full(rarex, 1, cuP_RAREX_WRITE_MULTIPLIER);
}

/*!@}*/
CU_END_DECLARATIONS

#endif
