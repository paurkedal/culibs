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
    cucon_arr_t arr = cu_gnew(struct cucon_arr_s);
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
    cucon_arr_t arr = cu_gnew(struct cucon_arr_s);
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

void
cucon_arr_resize_gp(cucon_arr_t arr, size_t size)
{
    size_t old_cap = arr->cap;
    if (size > old_cap) {
	char *old_carr = arr->carr;
	size_t new_cap;
	if (size < old_cap*2)
	    new_cap = old_cap*2;
	else
	    new_cap = size;
	arr->cap = new_cap;
	arr->carr = cu_galloc(new_cap);
	memcpy(arr->carr, old_carr, arr->size);
    }
    else if (size*2 < old_cap) {
	char *old_carr = arr->carr;
	arr->cap = size;
	arr->carr = cu_galloc(size);
	memcpy(arr->carr, old_carr, arr->size);
    }
    arr->size = size;
}

void
cucon_arr_resize_gpmax(cucon_arr_t arr, size_t size)
{
    size_t old_cap = arr->cap;
    if (size > old_cap) {
	char *old_carr = arr->carr;
	size_t new_cap;
	if (size < old_cap*2)
	    new_cap = old_cap*2;
	else
	    new_cap = size;
	arr->cap = new_cap;
	arr->carr = cu_galloc(new_cap);
	memcpy(arr->carr, old_carr, arr->size);
    }
    arr->size = size;
}

void
cucon_arr_resize_exact(cucon_arr_t arr, size_t size)
{
    size_t old_cap = arr->cap;
    if (size != old_cap) {
	char *old_carr = arr->carr;
	arr->cap = size;
	arr->carr = cu_galloc(size);
	memcpy(arr->carr, old_carr, arr->size);
    }
    arr->size = size;
}

void
cucon_arr_resize_exactmax(cucon_arr_t arr, size_t size)
{
    size_t old_cap = arr->cap;
    if (size > old_cap) {
	char *old_carr = arr->carr;
	arr->cap = size;
	arr->carr = cu_galloc(size);
	memcpy(arr->carr, old_carr, arr->size);
    }
    arr->size = size;
}
