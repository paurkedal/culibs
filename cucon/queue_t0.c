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

#include <cucon/queue.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define REPETITIONS 100000

int
main()
{
    int i;
    int j = 0, k = 0;
    int s = 0, t = 0;
    int size_arr[REPETITIONS];
    cucon_queue_t q;

    cu_init();

    q = cucon_queue_new();
    for (i = 0; i < REPETITIONS; ++i) {
	if (lrand48() % 2) {
	    size_t size = size_arr[j++] = lrand48() % 300;
	    void *ptr = cucon_queue_push_mem(q, size);
	    memset(ptr, s++, size);
	}
	else if (j == k)
	    assert(cucon_queue_is_empty(q));
	else {
	    size_t l;
	    size_t size = size_arr[k++];
	    char *ptr;
	    assert(size == 0 || !cucon_queue_is_empty(q));
	    ptr = cucon_queue_front_mem(q);
	    for (l = 0; l < size; ++l)
		assert(ptr[l] == (char)t);
	    ++t;
	    cucon_queue_pop_mem(q, size);
	}
    }
    return 0;
}
