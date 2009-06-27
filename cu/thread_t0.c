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

#include <cu/thread.h>
#include <cu/clos.h>
#include <cu/test.h>
#include <cu/memory.h>
#include <atomic_ops.h>
#include <unistd.h>

#define SPAWN_COUNT 16

static AO_t _tls_next_id = 0;
static pthread_key_t _tls_key;

typedef struct _tls_s *_tls_t;
struct _tls_s
{
    int id;
};

static int _thread_status[SPAWN_COUNT + 1];

cu_clop_def0(_on_thread_entry, void)
{
    _tls_t tls = cu_gnew_u(struct _tls_s);
    tls->id = AO_fetch_and_add(&_tls_next_id, 1);
    cu_test_assert_int_leq(tls->id, SPAWN_COUNT);
    cu_test_assert_int_eq(_thread_status[tls->id], 0);
    _thread_status[tls->id] = 1;
    pthread_setspecific(_tls_key, tls);
}

cu_clop_def0(_on_thread_exit, void)
{
    _tls_t tls = pthread_getspecific(_tls_key);
    _thread_status[tls->id] = 2;
    if (tls->id == 0)
	printf("[+] ... finished main thread cleanup.\n");
}

static void
_tls_destruct(void *tls_)
{
    _tls_t tls = tls_;
    cu_test_assert_int_geq(_thread_status[tls->id], 1);
}

static AO_t _atexit_count;

cu_clop_def0(_atexit, void)
{
    _tls_t tls = pthread_getspecific(_tls_key);
    AO_fetch_and_add_release_write(&_atexit_count, 1);
    cu_test_assert_int_eq(_thread_status[tls->id], 1);
}

void *
_thread_main(void *targ)
{
    long i;
    _tls_t tls = pthread_getspecific(_tls_key);
    if (tls->id % 2)
	cu_thread_atexit(_atexit);
    cu_test_assert_int_eq(_thread_status[tls->id], 1);

    /* Let half of the threads wait a bit. */
    i = mrand48();
    if (i > 0) {
	struct timespec tv;
	tv.tv_sec = 0;
	tv.tv_nsec = i % 5000000; /* Upto 5 ms. */
	nanosleep(&tv, NULL);
    }
    return NULL;
}

int
main()
{
    int i, j;
    int targ[SPAWN_COUNT];
    pthread_t th[SPAWN_COUNT];

    cu_init();
    pthread_key_create(&_tls_key, _tls_destruct);
    cu_register_thread_init(cu_clop_ref(_on_thread_entry),
			 cu_clop_ref(_on_thread_exit));
    cu_test_assert(_tls_next_id == 1);
    cu_test_assert_int_eq(_thread_status[0], 1); /* main thread */

    for (j = 0; j < 10; ++j) {
	_tls_next_id = 1;
	_atexit_count = 0;
	for (i = 1; i <= SPAWN_COUNT; ++i)
	    _thread_status[i] = 0;

	/* Start up threads, and cancel one. */
	for (i = 0; i < SPAWN_COUNT; ++i) {
	    targ[i] = i;
	    cu_thread_create(&th[i], NULL, _thread_main, &targ[i]);
	}
	pthread_cancel(th[j % SPAWN_COUNT]);

	/* Wait for the threads to exit. */
	for (i = 0; i < SPAWN_COUNT; ++i) {
	    if (cu_thread_join(th[i], NULL) != 0) {
		perror("cu_thread_join");
		exit(2);
	    }
	}

	/* Consistency check. */
	cu_test_assert_int_eq(_atexit_count, SPAWN_COUNT/2);
	for (i = 1; i <= SPAWN_COUNT; ++i)
	    cu_test_assert_int_eq(_thread_status[i], 2);
    }

    cu_test_assert_int_eq(_thread_status[0], 1); /* main thread */
    printf("[-] Pending main thread cleanup ...\n"); fflush(stdout);
    return 0;
}
