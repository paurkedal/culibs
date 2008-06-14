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

#include <cucon/parr.h>
#include <cu/diag.h>

void
cucon_parr_init_fill(cucon_parr_t arr, size_t size, void *ptr)
{
    cucon_parr_init_size(arr, size);
    cucon_parr_fill_all(arr, ptr);
}

cucon_parr_t
cucon_parr_new_fill(size_t size, void *ptr)
{
    cucon_parr_t arr = cucon_parr_new_size(size);
    cucon_parr_fill_all(arr, ptr);
    return arr;
}

void
cucon_parr_fill_all(cucon_parr_t arr, void *ptr)
{
    void **itr = cucon_parr_begin(arr);
    void **itr_end = cucon_parr_end(arr);
    while (itr != itr_end) {
	*itr = ptr;
	++itr;
    }
}

void
cucon_parr_fill_range(cucon_parr_t arr, size_t start, size_t end, void *ptr)
{
    void **itr_begin = cucon_parr_begin(arr);
    void **itr = itr_begin + start;
    void **itr_end = itr_begin + end;
    if (itr_end > (void **)cucon_parr_end(arr))
	cu_bugf("Index %d points past end of %d element array.", end,
		cucon_parr_size(arr));
    if (itr > itr_end)
	cu_bugf("Start index %d is larger than end index %d.", start, end);
    while (itr != itr_end) {
	*itr = ptr;
	++itr;
    }
}

void
cucon_parr_resize_gp_fill(cucon_parr_t arr, size_t size, void *ptr)
{
    size_t old_size = cucon_parr_size(arr);
    cucon_parr_resize_gp(arr, size);
    if (size > old_size)
	cucon_parr_fill_range(arr, old_size, size, ptr);
}

void
cucon_parr_resize_gpmax_fill(cucon_parr_t arr, size_t size, void *ptr)
{
    size_t old_size = cucon_parr_size(arr);
    cucon_parr_resize_gpmax(arr, size);
    if (size > old_size)
	cucon_parr_fill_range(arr, old_size, size, ptr);
}

void
cucon_parr_resize_exact_fill(cucon_parr_t arr, size_t size, void *ptr)
{
    size_t old_size = cucon_parr_size(arr);
    cucon_parr_resize_exact(arr, size);
    if (size > old_size)
	cucon_parr_fill_range(arr, old_size, size, ptr);
}

void
cucon_parr_resize_exactmax_fill(cucon_parr_t arr, size_t size, void *ptr)
{
    size_t old_size = cucon_parr_size(arr);
    cucon_parr_resize_exactmax(arr, size);
    if (size > old_size)
	cucon_parr_fill_range(arr, old_size, size, ptr);
}
