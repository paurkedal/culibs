/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#define _POSIX_C_SOURCE 199309
#include <cuflow/cache.h>
#include <cuflow/cache_t0_tab.h>
#include <cuflow/time.h>
#include <math.h>
#include <time.h>

#define FN_ARR_SIZE 1

#define FN0_FNCODE CUFLOW_FNCODE(0, sizeof(fn0_key_s)/sizeof(cu_word_t))

cuflow_cached_edcl(fn0,
    ( double xL;
      double xH;
      int N; ),
    ( double I; ));

cuflow_cached_edef(fn0)
{
    double x;
    double I = 0.0;
    fn0_obj_t *obj;
    double L = key->xH - key->xL;
    double dx = L/key->N;

    printf("Computing fn0(%lg, %lg, %d)\n", key->xL, key->xH, key->N);
    obj = cuflow_cached_new(fn0, 0);
    for (x = key->xL; x <= key->xH; x += dx)
	I += exp(x);
    I *= dx;
    obj->I = I;
    cuflow_cacheobj_set_gain((cuflow_cacheobj_t)obj, 10*key->N);
    return obj;
}

void
test(cu_bool_t do_print)
{
    fn0_key_t key0;
    fn0_obj_t *obj0;
    int i;
    for (i = 0; i < 10; ++i) {
	key0.xL = 0.0;
	key0.xH = 1.0;
	key0.N = 1 << (1 + i);
	//key0.N = 1 << (1 + lrand48() % 10);
	obj0 = testcache_call(fn0, &key0);
	if (do_print)
	    printf("fn(%lg, %lg, %d) = %lf\n",
		   key0.xL, key0.xH, key0.N, obj0->I);
    }
}

int main()
{
    struct timespec tv;
    int i;
    cuflow_init();
    testcache_init(cuflow_default_cacheconf());
    tv.tv_sec = 0;
    test(cu_true);
    for (i = 5; i < 15; ++i) {
	tv.tv_nsec = 1L << i*2;
	printf("Sleeping for %lu ns.\n", (unsigned long)tv.tv_nsec);
	nanosleep(&tv, NULL);
	test(cu_false);
    }
    return 0;
}
