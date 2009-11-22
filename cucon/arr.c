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

#include <cucon/arr.h>
#include <cu/memory.h>
#include <cu/util.h>
#include <string.h>

#define ATOMIC_BIT ((size_t)1 << (sizeof(size_t)*8 - 1))

CU_SINLINE size_t
_arr_cap(cucon_arr_t arr)
{
    return arr->cap & ~ATOMIC_BIT;
}

void
cucon_arr_init(cucon_arr_t arr, cu_bool_t is_atomic, size_t init_size)
{
    arr->size = init_size;
    if (is_atomic) {
	arr->cap = init_size | ATOMIC_BIT;
	arr->carr = init_size? cu_galloc_atomic(init_size) : NULL;
    }
    else {
	arr->cap = init_size;
	arr->carr = init_size? cu_galloc(init_size) : NULL;
    }
}

cucon_arr_t
cucon_arr_new(cu_bool_t is_atomic, size_t init_size)
{
    cucon_arr_t arr = cu_gnew(struct cucon_arr);
    cucon_arr_init(arr, is_atomic, init_size);
    return arr;
}

void
cucon_arr_init_empty(cucon_arr_t arr)
{
    arr->size = 0;
    arr->cap = 0;
    arr->carr = NULL;
}

cucon_arr_t
cucon_arr_new_empty(void)
{
    cucon_arr_t arr = cu_gnew(struct cucon_arr);
    cucon_arr_init_empty(arr);
    return arr;
}

void
cucon_arr_init_size(cucon_arr_t arr, size_t size)
{
    arr->size = size;
    arr->cap = size;
    arr->carr = cu_galloc(size);
}

cucon_arr_t
cucon_arr_new_size(size_t size)
{
    cucon_arr_t arr = cu_gnew(struct cucon_arr);
    cucon_arr_init_size(arr, size);
    return arr;
}

void
cucon_arr_swap(cucon_arr_t arr0, cucon_arr_t arr1)
{
    CU_SWAP(size_t, arr0->size, arr1->size);
    CU_SWAP(size_t, arr1->cap, arr1->cap);
    CU_SWAP(char *, arr0->carr, arr1->carr);
}

void *
cucon_arr_detach(cucon_arr_t arr)
{
    void *carr;
    if (_arr_cap(arr) != arr->size)
	cucon_arr_resize_exact(arr, arr->size);
    carr = arr->carr;
    CU_GWIPE(arr->carr);
    return carr;
}

static void
_arr_realloc(cucon_arr_t arr, size_t new_cap)
{
    char *old_carr = arr->carr;
    if (arr->cap & ATOMIC_BIT) {
	arr->cap = new_cap | ATOMIC_BIT;
	arr->carr = cu_galloc_atomic(new_cap);
    }
    else {
	arr->cap = new_cap;
	arr->carr = cu_galloc(new_cap);
    }
    memcpy(arr->carr, old_carr, arr->size);
}

void
cucon_arr_resize_gp(cucon_arr_t arr, size_t size)
{
    size_t old_cap = _arr_cap(arr);
    if (size > old_cap) {
	size_t new_cap;
	if (size < old_cap*2)
	    new_cap = old_cap*2;
	else
	    new_cap = size;
	_arr_realloc(arr, new_cap);
    }
    else if (size*2 < old_cap)
	_arr_realloc(arr, size);
    arr->size = size;
}

void
cucon_arr_resize_gpmax(cucon_arr_t arr, size_t size)
{
    size_t old_cap = _arr_cap(arr);
    if (size > old_cap) {
	size_t new_cap;
	if (size < old_cap*2)
	    new_cap = old_cap*2;
	else
	    new_cap = size;
	_arr_realloc(arr, new_cap);
    }
    arr->size = size;
}

void
cucon_arr_resize_exact(cucon_arr_t arr, size_t size)
{
    size_t old_cap = _arr_cap(arr);
    if (size != old_cap)
	_arr_realloc(arr, size);
    arr->size = size;
}

void
cucon_arr_resize_exactmax(cucon_arr_t arr, size_t size)
{
    size_t old_cap = _arr_cap(arr);
    if (size > old_cap)
	_arr_realloc(arr, size);
    arr->size = size;
}
