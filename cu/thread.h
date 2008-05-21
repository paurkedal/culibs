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

#ifndef CU_THREAD_H
#define CU_THREAD_H

#include <pthread.h>
#include <cu/fwd.h>
#include <cu/conf.h>
#include <cu/diag.h>
#include <cu/hash.h>
#include <string.h>
#include <errno.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_thread cu/thread.h: Multi-Threading (Extensions to pthreads)
 * @{ \ingroup cu_base_mod
 * These are mostly light wrappers around the pthread functions or
 * from the corresponding GC wrappers.  You may as well use the underlying
 * functions if you prefer.
 *
 * The mutex functions defined here will, when debugging is enabled, check
 * the error codes from the pthread library, and abort on errors which should
 * not occur in a correct program.  These are \c EINVAL, \c EDEADLK and
 * \c EPERM, whereas \c EBUSY is reported *by cu_pthread_trylock as
 * a \c cu_false return. */

/* Threads
 * ======= */

int GC_pthread_create(pthread_t *, pthread_attr_t const *,
		      void *(*)(void *), void *);
int GC_pthread_join(pthread_t thread, void **retval);
int GC_pthread_detach(pthread_t thread);

/*!Register \a cleanup to be run at exit or cancellation of the current
 * thread, and before previously registered cleanup functions. */
void cu_thread_atexit(cu_clop0(cleanup, void));

/*!Alias for the GC wrapper for pthread_create. */
#define cu_thread_create GC_pthread_create
/*!Alias for the GC wrapper for pthread_join. */
#define cu_thread_join GC_pthread_join
/*!Alias for the GC wrapper for pthread_detach. */
#define cu_thread_detach GC_pthread_detach

/*!\deprecated*/
#define cu_pthread_create cu_thread_create
/*!\deprecated*/
#define cu_pthread_join cu_thread_join
/*!\deprecated*/
#define cu_pthread_detach cu_thread_detatch


/* Mutices
 * ======= */

typedef pthread_mutex_t cu_mutex_t;

/* CU_MUTEX_INITIALISER is set to error-checking type iff (1) the library
 * is configured to enable debugging (2) error-checking mutices are available,
 * and (3) the client has defined _GNU_SOURCE.  The latter condition is to
 * avoild opening other library features which the client does not request,
 * and it does not apply to explicitely constructed mutices. */
#if defined(CUCONF_ERRORCHECK_MUTEX_INITIALIZER) && defined(_GNU_SOURCE)
#   define CU_MUTEX_INITIALISER CUCONF_ERRORCHECK_MUTEX_INITIALIZER
#else
#   define CU_MUTEX_INITIALISER PTHREAD_MUTEX_INITIALIZER
#endif
#if defined(CUCONF_MUTEX_ERRORCHECK)
#   define cu_mutex_init(m) pthread_mutex_init(m, &cuP_mutexattr)
    extern pthread_mutexattr_t cuP_mutexattr;
#else
#   define cu_mutex_init(m) pthread_mutex_init(m, NULL)
#endif

/*!Lock \a m.  Aborts on bad usage if debugging is enabled. */
CU_SINLINE void
cu_mutex_lock(pthread_mutex_t *m)
{
#ifndef CU_NDEBUG
    int st = pthread_mutex_lock(m);
    if (st != 0)
	cu_bugf("Could not lock mutex: %s", strerror(st));
#else
    pthread_mutex_lock(m);
#endif
}

/*!Tries to lock \a m without blocking, and return true iff successful.
 * Aborts on bad usage if debugging is enabled. */
CU_SINLINE cu_bool_t
cu_mutex_trylock(pthread_mutex_t *m)
{
#ifndef CU_NDEBUG
    int st = pthread_mutex_trylock(m);
    if (st == 0)
	return cu_true;
    else if (st == EBUSY || st == EDEADLK)
	return cu_false;
    else
	cu_bugf("Could not lock mutex: %s", strerror(st));
#else
    return pthread_mutex_trylock(m) == 0;
#endif
}

/*!Unlock \a m.  Aborts on bad usage if debugging is enabled. */
CU_SINLINE void
cu_mutex_unlock(pthread_mutex_t *m)
{
#ifndef CU_NDEBUG
    int st = pthread_mutex_unlock(m);
    if (st)
	cu_bugf("Could not unlock mutex: %s", strerror(st));
#else
    pthread_mutex_unlock(m);
#endif
}


/* Per-Pointer Mutices
 * =================== */

#define cuP_PMUTEX_CNT 512
extern cu_mutex_t cuP_pmutex_arr[cuP_PMUTEX_CNT];

CU_SINLINE cu_mutex_t *cu_pmutex_mutex(void *ptr)
{ return &cuP_pmutex_arr[cu_hash_mix((cu_hash_t)ptr) % cuP_PMUTEX_CNT]; }

/*!Lock a mutex indexed by a hash mix of <tt>(cu_hash_t)\a ptr</tt>.
 * These per-pointer mutices must not be used recursively, since several
 * object pointers may have the same mutex. */
void cu_pmutex_lock(void *ptr);

/*!Attempt to lock a mutex indexed by a hash mix of <tt>(cu_hash_t)\a ptr</tt>.
 * See also \ref cu_pmutex_lock. */
cu_bool_t cu_pmutex_trylock(void *ptr);

/*!Unlock a mutex indexed by a hash mix of <tt>(cu_hash_t)\a ptr</tt>. */
void cu_pmutex_unlock(void *ptr);

/*!\deprecated Use cu_mutex_init. */
#define cu_mutex_cct cu_mutex_init

/*!@}*/
CU_END_DECLARATIONS

#endif
