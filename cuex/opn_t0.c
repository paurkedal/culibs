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

#include <cuex/opn.h>
#include <cu/test.h>
#include <math.h>
#include <time.h>
#include <gc/gc.h>

#define NTHREADS 4
#define EXN_CNT (5000/NTHREADS)
#define REPEAT (5000000/NTHREADS)
#define EPSILON 1e-10


cuex_meta_t
check_ex(void *ex)
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_kind(meta) == cuex_meta_kind_opr) {
	cu_rank_t J = cuex_opr_r(meta);
	cu_rank_t j;
	cuex_meta_t opr = 1;
	for (j = 0; j < J; ++j)
	    opr += check_ex(cuex_opn_at(ex, j));
	opr %= 0x1000;
	cu_test_assert(cuex_opr(opr, J) == meta);
    }
    return meta;
}

static void *
test(void *thread_index)
{
    int i;
    cuex_t exn_arr[EXN_CNT];
    cuex_t scratch_arr[1024];
    clock_t t = -clock();
    for (i = 0; i < EXN_CNT; ++i)
	exn_arr[i] = cuex_opn(cuex_opr(1, 0));
    for (i = 0; i < REPEAT; ++i) {
	int dst = lrand48() % EXN_CNT;
	int j;
	int J = -(int)log(drand48()*(1.0 - EPSILON) + EPSILON);
	cuex_meta_t opr = 1;// = lrand48() % 20;
	cuex_opn_t e;
	//if (i < 50 && (int)thread_index == 0)
	//    printf("%5d %2d\n", dst, J);

	assert(0 <= J && J < 1024);
	for (j = 0; j < J; ++j) {
	    scratch_arr[j] = exn_arr[lrand48() % EXN_CNT];
	    opr += cuex_meta(scratch_arr[j]);
	}
	opr %= 0x1000;
	e = cuex_opn_by_arr(cuex_opr(opr, J), scratch_arr);
	check_ex(e);
	exn_arr[dst] = e;
    }
    t += clock();
    return (void *)t;
}

void
report(void *obj, void *cd)
{
    printf("Destroying %p\n", obj);
}

static cuex_opn_t save;

void *
simple_test()
{
    cuex_opn_t opn0 = cuex_opn(cuex_opr(1, 0));
    cuex_opn_t opn1 = cuex_opn(cuex_opr(2, 0));
    cuex_opn_t opn01 = cuex_opn(cuex_opr(3, 2), opn0, opn1);
    cuex_t arr[] = { opn0, opn1 };
    GC_register_finalizer(opn01, report, NULL, NULL, NULL);
    save = opn01;
    printf("%p\n", opn01);
    cu_test_assert(cuex_opn(cuex_opr(1, 0)) == opn0);
    cu_test_assert(cuex_opn(cuex_opr(3, 2), opn0, opn1) == opn01);
    GC_gcollect();
    //test(NULL);
    printf("%p, %p, %p\n", opn01, opn0, opn1);
    cu_test_assert(cuex_opn(cuex_opr(1, 0)) == opn0);
    cu_test_assert(cuex_opn(cuex_opr(3, 2), opn0, opn1) == opn01);
    cu_test_assert(cuex_opn_by_arr(cuex_opr(3, 2), arr) == opn01);
    cu_test_assert(cuex_opn(cuex_opr(3, 2), opn0, opn0) != opn01);
    cu_test_assert(cuex_opn(cuex_opr(2, 2), opn0, opn1) != opn01);
    return opn01;
}

long opn_t0_seed;

int
main()
{
    clock_t t = 0;
#if NTHREADS > 1
    int i;
    pthread_t th[NTHREADS];
#endif
    cu_init();
    simple_test();
    opn_t0_seed = time(NULL);
    //opn_t0_seed = 1121340589;
    srand48(opn_t0_seed);
#if NTHREADS <= 1
    t += (clock_t)test((void *)0);
#else
    for (i = 0; i < NTHREADS; ++i) {
	int err = GC_pthread_create(&th[i], NULL, test, (void *)i);
	if (err)
	    fprintf(stderr, "Could not create thread %d: %s\n",
		    i, strerror(err));
    }
    for (i = 0; i < NTHREADS; ++i) {
	void *res;
	pthread_join(th[i], &res);
	t += (clock_t)res;
    }
#endif
    printf("CPU time: %lg s\n", t/(double)CLOCKS_PER_SEC);
    return 2*!!cu_test_bug_count();
}
