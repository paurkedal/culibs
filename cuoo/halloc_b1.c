/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#include <cuoo/halloc.h>
#include <cuoo/hctem.h>
#include <cuoo/intf.h>
#include <cu/test.h>
#include <cu/diag.h>

#define REPEAT 20
#define MAX_LOG_NODE_COUNT 20
#define MAX_ALLOC_COUNT ((size_t)2 << MAX_LOG_NODE_COUNT)
#define RETAIN_COUNT 1
#define N_CONST (1 << 10)

typedef struct _const  *_const_t;
typedef struct _tuple1 *_tuple1_t;
typedef struct _tuple2 *_tuple2_t;
typedef struct _tuple3 *_tuple3_t;

struct _const  { CUOO_HCOBJ cu_word_t c; };
struct _tuple1 { CUOO_HCOBJ void *arr[1]; };
struct _tuple2 { CUOO_HCOBJ void *arr[2]; };
struct _tuple3 { CUOO_HCOBJ void *arr[3]; };

static cuoo_type_t _the_const_type;
static cuoo_type_t _the_tuple1_type, _the_tuple2_type, _the_tuple3_type;
#define _const_type()  _the_const_type
#define _tuple1_type() _the_tuple1_type
#define _tuple2_type() _the_tuple2_type
#define _tuple3_type() _the_tuple3_type

static _const_t
_const_new(int c)
{
    cuoo_hctem_decl(_const, tem);
    cuoo_hctem_init(_const, tem);
    cuoo_hctem_get(_const, tem)->c = c;
    return cuoo_hctem_new(_const, tem);
}

static _tuple1_t
_tuple1_new(void *p0)
{
    cuoo_hctem_decl(_tuple1, tem);
    cuoo_hctem_init(_tuple1, tem);
    cuoo_hctem_get(_tuple1, tem)->arr[0] = p0;
    return cuoo_hctem_new(_tuple1, tem);
}

static _tuple2_t
_tuple2_new(void *p0, void *p1)
{
    cuoo_hctem_decl(_tuple2, tem);
    cuoo_hctem_init(_tuple2, tem);
    cuoo_hctem_get(_tuple2, tem)->arr[0] = p0;
    cuoo_hctem_get(_tuple2, tem)->arr[1] = p1;
    return cuoo_hctem_new(_tuple2, tem);
}

static _tuple3_t
_tuple3_new(void *p0, void *p1, void *p2)
{
    cuoo_hctem_decl(_tuple3, tem);
    cuoo_hctem_init(_tuple3, tem);
    cuoo_hctem_get(_tuple3, tem)->arr[0] = p0;
    cuoo_hctem_get(_tuple3, tem)->arr[1] = p1;
    cuoo_hctem_get(_tuple3, tem)->arr[2] = p2;
    return cuoo_hctem_new(_tuple3, tem);
}

static void *
_random_tree(size_t n)
{
    size_t n0, n1;

    if (!n)
	return _const_new(lrand48() % N_CONST);

    --n;
    switch (lrand48() % 3 + 1) {
	case 1:
	    return _tuple1_new(_random_tree(n));
	case 2:
	    n0 = lrand48() % (n + 1);
	    return _tuple2_new(_random_tree(n0), _random_tree(n - n0));
	case 3:
	    n0 = lrand48() % (n + 1);
	    n1 = lrand48() % (n + 1);
	    if (n1 < n0)
		CU_SWAP(size_t, n0, n1);
	    return _tuple3_new(_random_tree(n0), _random_tree(n1 - n0),
			       _random_tree(n - n1));
	default:
	    cu_bug_unreachable();
    }
}

static void
_test()
{
    void *trees[RETAIN_COUNT];
    int i, j;
    size_t tot_count = 0;
    clock_t t = -clock();
    printf("%7s %7s  %s\n", "size", "repeat", "alloc freq");
    printf("----------------------------\n");
    for (i = 0; i < REPEAT; ++i) {
	int log_node_count = lrand48() % MAX_LOG_NODE_COUNT;
	size_t node_count = lrand48() % ((size_t)1 << log_node_count) + 1;
	size_t n_alloc = MAX_ALLOC_COUNT/node_count;
	size_t sub_count = 0;
	clock_t t_sub = -clock();
	for (j = 0; j < n_alloc; ++j) {
	    trees[j % RETAIN_COUNT] = _random_tree(node_count);
	    sub_count += node_count;
	}
	t_sub += clock();
	printf("%7zd %7zd  %#6.3lg s\n", node_count, n_alloc,
	       t_sub/((double)CLOCKS_PER_SEC*sub_count));
	for (j = 0; j < RETAIN_COUNT; ++j)
	    trees[j] = NULL;
	tot_count += sub_count;
    }
    t += clock();
    printf("----------------------------\n");
    printf("%16s %#6.3lg s\n", "Avg.", t/((double)CLOCKS_PER_SEC*tot_count));
}

int
main()
{
    cu_init();
    _the_const_type = cuoo_type_new_opaque_hcs(
	cuoo_impl_none, sizeof(struct _const) - CUOO_HCOBJ_SHIFT);
    _the_tuple1_type = cuoo_type_new_opaque_hcs(
	cuoo_impl_none, sizeof(struct _tuple1) - CUOO_HCOBJ_SHIFT);
    _the_tuple2_type = cuoo_type_new_opaque_hcs(
	cuoo_impl_none, sizeof(struct _tuple2) - CUOO_HCOBJ_SHIFT);
    _the_tuple3_type = cuoo_type_new_opaque_hcs(
	cuoo_impl_none, sizeof(struct _tuple3) - CUOO_HCOBJ_SHIFT);
    _test();
    return 2*!!cu_test_bug_count();
}
