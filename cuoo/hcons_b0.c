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

#include <cuoo/type.h>
#include <cuoo/hctem.h>
#include <cuoo/intf.h>
#include <stdlib.h>

#define N_ALLOC 0x2000000

CU_SINLINE void _ignore_pointer(void *ptr) {}

typedef struct _test *_test_t;
struct _test
{
    CUOO_HCOBJ
    int arg0;
    int arg1;
};

static cuoo_type_t _g_test_type;

CU_SINLINE cuoo_type_t _test_type()
{ return _g_test_type; }

static void
_test_GC_malloc(int alloc_count)
{
    int i;
    for (i = 0; i < alloc_count; ++i) {
	_test_t o = GC_malloc(sizeof(struct _test));
	o->arg0 = i;
	o->arg1 = i;
    }
}

static void
_test_onew(int alloc_count)
{
    int i;
    for (i = 0; i < alloc_count; ++i) {
	_test_t o = cuoo_onew(_test);
	o->arg0 = i;
	o->arg1 = i;
    }
}

static void
_test_hnew_noneq(int alloc_count)
{
    int i;
    for (i = 0; i < alloc_count; ++i) {
	cuoo_hctem_decl(_test, tem);
	cuoo_hctem_init(_test, tem);
	cuoo_hctem_get(_test, tem)->arg0 = i;
	cuoo_hctem_get(_test, tem)->arg1 = i;
	_ignore_pointer(cuoo_hctem_new(_test, tem));
    }
}

static void
_test_hnew_eq(int alloc_count)
{
    int i;
    for (i = 0; i < alloc_count; ++i) {
	cuoo_hctem_decl(_test, tem);
	cuoo_hctem_init(_test, tem);
	cuoo_hctem_get(_test, tem)->arg0 = 0;
	cuoo_hctem_get(_test, tem)->arg1 = 0;
	_ignore_pointer(cuoo_hctem_new(_test, tem));
    }
}

static void
_test_malloc_free(int alloc_count)
{
    int i;
    for (i = 0; i < alloc_count; ++i) {
	_test_t o = malloc(sizeof(struct _test));
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

static void *
_thread_main(void *carg)
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
    "cuoo_halloc (different objects)",
    "cuoo_halloc (same objects)",
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
	    test = _test_GC_malloc;
	    break;
	case 1:
	    test = _test_onew;
	    break;
	case 2:
	    test = _test_hnew_noneq;
	    break;
	case 3:
	    test = _test_hnew_eq;
	    break;
	case 4:
	    test = _test_malloc_free;
	    break;
	default:
	    exit(2);
    }

    _g_test_type = cuoo_type_new_opaque_hcs(
	cuoo_impl_none, sizeof(struct _test) - CUOO_HCOBJ_SHIFT);
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
			 _thread_main, &thread_arg[i]);
    }
    for (i = 0; i < thread_count; ++i)
	cu_thread_join(thread_arg[i].thread, NULL);
    GC_gcollect();
    t += clock();
    printf("%s: %.3lg s per alloc\n", alloc_name_arr[alloc_type],
	   t/((double)CLOCKS_PER_SEC * alloc_count));
    return 0;
}
