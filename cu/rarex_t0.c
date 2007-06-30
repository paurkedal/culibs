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

#include <cu/rarex.h>
#include <gc/gc_pthread_redirects.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define RAREX_CNT 4
#define THREAD_CNT 4

/* With many reads per write the test case favours rarices over mutices.
 * Otherwise, mutices are preferred if well implemented on the platform. */
#define OPERATION_CNT 2000000
#define READ_PER_ROUND 100
#define WRITE_PER_ROUND 1
#define ACCESS_ROUNDS \
	OPERATION_CNT/(THREAD_CNT*(READ_PER_ROUND + WRITE_PER_ROUND))
#define VALUE_WORDS 1

#if 1
#  define cu_rarex_cct(r) pthread_mutex_init(r, NULL)
#  define cu_rarex_t pthread_mutex_t
#  define cu_rarex_lock_read(r) pthread_mutex_lock(r)
#  define cu_rarex_unlock_read(r) pthread_mutex_unlock(r)
#  define cu_rarex_lock_write(r) pthread_mutex_lock(r)
#  define cu_rarex_unlock_write(r) pthread_mutex_unlock(r)
#endif

struct unit_s
{
    cu_rarex_t rarex;
#if VALUE_WORDS > 0
    unsigned int value[VALUE_WORDS];
#endif
};

struct unit_s unit_arr[RAREX_CNT];
clock_t time_usage[THREAD_CNT];

void *
accessor(void *data)
{
    int i;
    unsigned int norand = lrand48();
#if VALUE_WORDS > 0
    unsigned int value[VALUE_WORDS];
#endif
    int thread_number = *(int *)data;
    time_usage[thread_number] = -clock();
    for (i = 0; i < ACCESS_ROUNDS; ++i) {
	int j;
	for (j = 0; j < READ_PER_ROUND; ++j) {
	    //int k = lrand48() % RAREX_CNT;
	    int k = norand % RAREX_CNT;
	    norand *= 7;
	    int r;
	    cu_rarex_lock_read(&unit_arr[k].rarex);
#if VALUE_WORDS > 0
	    memcpy(value, unit_arr[k].value, sizeof(value));
	    r = memcmp(value, unit_arr[k].value, sizeof(value));
	    assert(r == 0);
#endif
	    cu_rarex_unlock_read(&unit_arr[k].rarex);
	}
	for (j = 0; j < WRITE_PER_ROUND; ++j) {
	    //int k = lrand48() % RAREX_CNT;
	    int k = norand % RAREX_CNT;
	    norand *= 7;
	    int l;
	    int r;
	    cu_rarex_lock_write(&unit_arr[k].rarex);
#if VALUE_WORDS > 0
	    for (l = 0; l < VALUE_WORDS; ++l)
		unit_arr[k].value[l] = value[l] = lrand48();
	    r = memcmp(value, unit_arr[k].value, sizeof(value));
	    assert(r == 0);
#endif
	    cu_rarex_unlock_write(&unit_arr[k].rarex);
	}
    }
    time_usage[thread_number] += clock();
    return NULL;
}

int main()
{
    int i;
    pthread_t th[THREAD_CNT];
    double total_time;
    int thread_data[THREAD_CNT];
    cu_init();
    srand48(time(NULL));
    for (i = 0; i < RAREX_CNT; ++i)
	cu_rarex_cct(&unit_arr[i].rarex);
    for (i = 0; i < THREAD_CNT; ++i) {
	thread_data[i] = i;
	int err = pthread_create(&th[i], NULL, accessor, &thread_data[i]);
	if (err != 0) {
	    fprintf(stderr, "Failed to launch thread # %d: %s.\n",
		    i, strerror(err));
	    return 1;
	}
    }
    total_time = 0.0;
    for (i = 0; i < THREAD_CNT; ++i) {
	pthread_join(th[i], NULL);
	total_time += time_usage[i]/(double)CLOCKS_PER_SEC;
    }
    printf("CPU time: %lg s\n", total_time);
    return 0;
}
