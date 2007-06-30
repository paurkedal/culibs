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
#include <atomic_ops.h>

AO_t startup_count = 0;
AO_t cleanup_count = 0;

cu_clop_def0(cleanup, void)
{
    AO_fetch_and_add_release_write(&cleanup_count, 1);
}

void *
thread_main(void *targ)
{
    AO_fetch_and_add_release_write(&startup_count, 1);
    cu_thread_atexit(cleanup);
    while (*(int *)targ) sleep(1);
    return NULL;
}

int
main()
{
    int targ[] = {0, 1};
    pthread_t th[2];
    cu_init();
    cu_thread_create(&th[0], NULL, thread_main, &targ[0]);
    cu_thread_create(&th[1], NULL, thread_main, &targ[1]);
    pthread_cancel(th[1]);
    if (cu_thread_join(th[0], NULL) != 0) {
	perror("cu_thread_join");
	exit(2);
    }
    if (cu_thread_join(th[1], NULL) != 0) {
	perror("cu_thread_join");
	exit(2);
    }
    cu_test_assert(cleanup_count == 2);
    return 0;
}
