/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#include <cu/box.h>
#include <cu/test.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE 4096
#define REPEAT 50000

static void
_show_time(char const *name, clock_t t, int repeat, int arr_size)
{
    printf("%8s: %lg s\n", name, t/((double)CLOCKS_PER_SEC*repeat*arr_size));
}

typedef long double longdouble;

#define MAKE(type)							\
    									\
static void								\
_copy_##type##_arr(int n, volatile type *src_arr, volatile type *dst_arr) \
{									\
    int i;								\
    for (i = 0; i < n; ++i)						\
	dst_arr[i] = src_arr[i];					\
}									\
									\
static void								\
_box_##type##_arr(int n, volatile type *src_arr, volatile cu_box_t *dst_arr) \
{									\
    int i;								\
    for (i = 0; i < n; ++i)						\
	dst_arr[i] = cu_box_##type(src_arr[i]);				\
}									\
									\
static void								\
_unbox_##type##_arr(int n, volatile cu_box_t *src_arr, volatile type *dst_arr) \
{									\
    int i;								\
    for (i = 0; i < n; ++i)						\
	dst_arr[i] = cu_unbox_##type(src_arr[i]);			\
}									\
									\
static void								\
_bench_##type(int repeat)						\
{									\
    int i;								\
    clock_t t_copy = 0, t_box = 0, t_unbox = 0;				\
    type val_arr[ARRAY_SIZE];						\
    type scr_arr[ARRAY_SIZE];						\
    cu_box_t box_arr[ARRAY_SIZE];					\
    memset(val_arr, 0, sizeof(val_arr));				\
									\
    printf("%s:\n", #type);						\
    for (i = 0; i < repeat; ++i) {					\
	t_copy -= clock();						\
	_copy_##type##_arr(ARRAY_SIZE, val_arr, scr_arr);		\
	t_copy += clock();						\
									\
	t_box -= clock();						\
	_box_##type##_arr(ARRAY_SIZE, scr_arr, box_arr);		\
	t_box += clock();						\
									\
	t_unbox -= clock();						\
	_unbox_##type##_arr(ARRAY_SIZE, box_arr, val_arr);		\
	t_unbox += clock();						\
    }									\
									\
    _show_time("copy", t_copy, repeat, ARRAY_SIZE);			\
    _show_time("box", t_box, repeat, ARRAY_SIZE);			\
    _show_time("unbox", t_unbox, repeat, ARRAY_SIZE);			\
}									\
									\
CU_END_BOILERPLATE

MAKE(int);
MAKE(long);
MAKE(float);
MAKE(double);
MAKE(longdouble);

int
main()
{
    cu_init();

    _bench_int(REPEAT);
    _bench_long(REPEAT);
    _bench_float(REPEAT);
    _bench_double(REPEAT/4);
    _bench_longdouble(REPEAT/8);

    return 2*!!cu_test_bug_count();
}
