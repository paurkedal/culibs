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

#include <cu/dyn.h>
#include <stdlib.h>

#define N_ALLOC 0x2000000

typedef struct test_s *test_t;
struct test_s
{
    CU_HCOBJ
    int arg0;
    int arg1;
};

cudyn_stdtype_t _test_type;
CU_SINLINE cudyn_type_t test_type()
{ return cudyn_stdtype_to_type(_test_type); }

void
test_GC_malloc(int alloc_count)
{
    int i;
    for (i = 0; i < alloc_count; ++i) {
	test_t o = GC_malloc(sizeof(struct test_s));
	o->arg0 = i;
	o->arg1 = i;
    }
}

void
test_onew(int alloc_count)
{
    int i;
    for (i = 0; i < alloc_count; ++i) {
	test_t o = cudyn_onew(test);
	o->arg0 = i;
	o->arg1 = i;
    }
}

void
test_hnew_noneq(int alloc_count)
{
    int i;
    for (i = 0; i < alloc_count; ++i) {
	struct test_s o;
	o.arg0 = i;
	o.arg1 = i;
	cudyn_hnew(test, &o);
    }
}

void
test_hnew_eq(int alloc_count)
{
    int i;
    for (i = 0; i < alloc_count; ++i) {
	struct test_s o;
	o.arg0 = 0;
	o.arg1 = 0;
	cudyn_hnew(test, &o);
    }
}

void
test_malloc_free(int alloc_count)
{
    int i;
    for (i = 0; i < alloc_count; ++i) {
	test_t o = malloc(sizeof(struct test_s));
	o->arg0 = i;
	o->arg1 = i;
	free(o);
    }
}

struct thread_carg_s
{
    pthread_t thread;
    int alloc_type;
    void (*test)(int alloc_count);
    int alloc_count;
    clock_t time_out;
};

void *
thread_main(void *carg)
{
#define carg ((struct thread_carg_s *)carg)
    carg->time_out = -clock();
    carg->test(carg->alloc_count);
    carg->time_out += clock();
    return NULL;
#undef carg
}

static char const *alloc_name_arr[] = {
    "GC_malloc",
    "cu_onew",
    "cu_hnew (different objects)",
    "cu_hnew (same objects)",
    "malloc, free",
};

int
main(int argc, char **argv)
{
    int alloc_count = N_ALLOC;
    int thread_alloc_count;
    time_t t;
    int alloc_type;
    int thread_count = 1;
    void (*test)(int);
    int i;
    struct thread_carg_s *thread_arg;

    cu_init();

    if (argc < 2 || sscanf(argv[1], "%d", &alloc_type) != 1) {
	fprintf(stderr, "%s ALLOC_TYPE\n", argv[0]);
	for (i = 0; i < sizeof(alloc_name_arr)/sizeof(alloc_name_arr[0]); ++i)
	    fprintf(stderr, "    %d: %s\n", i, alloc_name_arr[i]);
	exit(2);
    }
    if (alloc_type == 2)
	alloc_count /= 2;
    if (argc > 2) { thread_count = atoi(argv[2]); }

    switch (alloc_type) {
	case 0:
	    test = test_GC_malloc;
	    break;
	case 1:
	    test = test_onew;
	    break;
	case 2:
	    test = test_hnew_noneq;
	    break;
	case 3:
	    test = test_hnew_eq;
	    break;
	case 4:
	    test = test_malloc_free;
	    break;
    }

    _test_type = cudyn_stdtype_new_hcs(sizeof(struct test_s) - CU_HCOBJ_SHIFT);
    printf("thread_count = %d, alloc_count = %d, alloc_type = %s\n",
	   thread_count, alloc_count, alloc_name_arr[alloc_type]);
    t = -clock();
    thread_alloc_count = alloc_count / thread_count;
    alloc_count = thread_alloc_count * thread_count;
    thread_arg = cu_salloc(thread_count*sizeof(struct thread_carg_s));
    for (i = 0; i < thread_count; ++i) {
	thread_arg[i].alloc_count = thread_alloc_count;
	thread_arg[i].test = test;
	thread_arg[i].alloc_type = alloc_type;
	cu_thread_create(&thread_arg[i].thread, NULL,
			 thread_main, &thread_arg[i]);
    }
    for (i = 0; i < thread_count; ++i)
	cu_thread_join(thread_arg[i].thread, NULL);
    GC_gcollect();
    t += clock();
    printf("%s: %.3lg s per alloc\n", alloc_name_arr[alloc_type],
	   t/((double)CLOCKS_PER_SEC * alloc_count));
    return 0;
}
