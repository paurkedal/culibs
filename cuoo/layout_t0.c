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

#include <cuoo/layout.h>
#include <cu/debug.h>
#include <cu/thread.h>
#include <cu/test.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define THREAD_CNT 5
#define TEST_CNT 50000

void
show_live_data_size(cuoo_layout_t lo)
{
    size_t size = 0;
    while (lo) {
	size += sizeof(struct cuoo_layout_s) + sizeof(cuex_meta_t);
	lo = lo->prefix;
    }
    printf("Approx. live data size = %ld\n", (long)size);
}

cuoo_layout_t
show_grow(cuoo_layout_t l, size_t bitsize, size_t bitalign)
{
    cu_offset_t bitoffset;
    cuoo_layout_t lr = cuoo_layout_pack_bits(l, bitsize, bitalign, &bitoffset);
    printf("grow(%p, %ld, %ld, => %ld) = %p\n",
	    l, (long)bitsize, (long)bitalign, (long)bitoffset, lr);
    return lr;
}

cuoo_layout_t
show_join(cuoo_layout_t l0, cuoo_layout_t l1)
{
    cu_offset_t bitoffset;
    cuoo_layout_t lr = cuoo_layout_product(l0, l1, &bitoffset);
    printf("join(%p, %p, => %ld) = %p\n", l0, l1, (long)bitoffset, lr);
    return lr;
}

void
simple_test()
{
    cuoo_layout_t l0, l1;
    l0 = show_grow(NULL, 3, 1);
    l0 = show_grow(l0, 4, 1);
    l0 = show_grow(l0, sizeof(long)*8, sizeof(long)*8);
    l1 = show_join(l0, l0);
    l1 = show_join(l0, l1);
}

void
union_test()
{
    cuoo_layout_t l0;
    cuoo_layout_t l1;
    printf("\nUnion Test\n\n");
    l0 = show_grow(NULL, 2, 1);
    l0 = show_grow(l0, 8, 8);
    l0 = show_grow(l0, 32, 32);
    l0 = show_grow(l0, 16, 8);
    cuoo_layout_dump(l0, stdout);
    l1 = show_grow(NULL, 1, 1);
    l1 = show_grow(l1, 16, 8);
    cuoo_layout_dump(l1, stdout);

    printf("Taking union\n");
    l0 = cuoo_layout_union(l0, l1);
    cuoo_layout_dump(l0, stdout);
    l0 = show_grow(l0, 1, 1);
    cuoo_layout_dump(l0, stdout);
}

void
product_test()
{
    cu_offset_t offset;
    cuoo_layout_t l0, l1, l2;
    l0 = show_grow(NULL, 3, 1);
    l0 = show_grow(l0, 8, 8);
    l1 = show_grow(NULL, 2, 1);
    printf("Product of\n");
    cuoo_layout_dump(l0, stdout);
    printf("and\n");
    cuoo_layout_dump(l1, stdout);
    l2 = cuoo_layout_product(l0, l1, &offset);
    printf("gives offset %ld in\n", (long int)offset);
    cuoo_layout_dump(l2, stdout);
}

void *
test_layout(void *data)
{
    int I = (uintptr_t)data;
    int i;
    cuoo_layout_t lo = 0;
    for (i = 0; i < I; ++i) {
	cu_offset_t bitsize = lrand48() % (sizeof(long)*12) + 1;
	cu_offset_t bitalign;
	cu_offset_t offset;
	if (i % (I*THREAD_CNT/20) == 0)
	    printf("%4.2lf complete\n", i/(double)I);
	//if (i % (I/100) == 0)
	//    GC_gcollect();
	if (bitsize > sizeof(long)*8) {
	    switch (bitsize % 8) {
	    case 0:
		bitalign = bitsize = sizeof(char)*8;
		break;
	    case 1:
		bitalign = bitsize = sizeof(short)*8;
		break;
	    case 2:
		bitalign = bitsize = sizeof(int)*8;
		break;
	    case 3:
		bitalign = bitsize = sizeof(long)*8;
		break;
	    default:
		if (lo)
		    lo = cuoo_layout_prefix(lo);
		if (lo)
		    lo = cuoo_layout_prefix(lo);
		goto next_i;
	    }
	}
	else
	    bitalign = 1;
	cu_test_assert(bitsize > 0);
	lo = cuoo_layout_pack_bits(lo, bitsize, bitalign, &offset);
	//if (I < 400)
	//    printf("%2d %2d %4d\n", bitsize, bitalign, offset);
    next_i:;
    }
    show_live_data_size(lo);
    return NULL;
}

long test_seed;

int
main()
{
    pthread_t th[THREAD_CNT];
    int i;
    cu_init();
    test_seed = time(NULL);
    //test_seed = 1121870991;
    srand48(test_seed);
    //test_layout((void *)100);
    simple_test();
    union_test();
    product_test();
#if THREAD_CNT <= 1
    test_layout((void *)(TEST_CNT/THREAD_CNT));
#else
    for (i = 0; i < THREAD_CNT; ++i) {
	int err;
	err = cu_pthread_create(&th[i], NULL, test_layout,
				(void *)(TEST_CNT/THREAD_CNT));
	if (err) {
	    fprintf(stderr, "%s\n", strerror(err));
	    return 1;
	}
    }
    for (i = 0; i < THREAD_CNT; ++i)
	cu_pthread_join(th[i], NULL);
#endif
    return 2*!!cu_test_bug_count();
}
