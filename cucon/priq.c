/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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


#include <cucon/priq.h>
#include <cu/memory.h>
#include <string.h>
#include <stdio.h>


void
cucon_priq_init(cucon_priq_t q, cu_clop(prior, cu_bool_t, void *, void *))
{
    q->prior = prior;
    q->capacity = 0;
    q->count = 0;
    q->arr = 0;
}

void
cucon_priq_init_copy(cucon_priq_t q, cucon_priq_t q_src)
{
    q->prior = q_src->prior;
    q->capacity = q_src->capacity;
    q->count = q_src->count;
    q->arr = cu_galloc(sizeof(void*)*q->capacity);
    memcpy(q->arr, q_src->arr, sizeof(void*)*q->capacity);
}

cucon_priq_t
cucon_priq_new(cu_clop(prior, cu_bool_t, void *, void *))
{
    cucon_priq_t q = cu_gnew(struct cucon_priq);
    q->prior = prior;
    q->capacity = 0;
    q->count = 0;
    q->arr = 0;
    return q;
}

static void
_expand_by_2(cucon_priq_t q)
{
    size_t old_capacity = q->capacity;
    if (old_capacity == 0) {
	q->capacity = 4;
	q->arr = cu_galloc(sizeof(void*)*q->capacity);
    }
    else {
	void **old_arr = q->arr;
	q->capacity *= 2;
	q->arr = cu_galloc(sizeof(void*)*q->capacity);
	memcpy(q->arr, old_arr, sizeof(void*)*old_capacity);
	cu_gfree(old_arr);
    }
}

static void
_reduce_by_4(cucon_priq_t q)
{
    void **old_arr = q->arr;
    q->capacity /= 4;
    q->arr = cu_galloc(sizeof(void*)*q->capacity);
    memcpy(q->arr, old_arr, sizeof(void*)*q->capacity);
    cu_gfree(old_arr);
}

void
cucon_priq_insert(cucon_priq_t q, void *key)
{
    size_t n = q->count;
    if (n >= q->capacity)
	_expand_by_2(q);
    q->arr[n] = key;
    while (n > 0) {
	size_t m = (n + 1)/2 - 1;
	if (cu_call(q->prior, q->arr[m], q->arr[n]))
	    break;
	key = q->arr[m];
	q->arr[m] = q->arr[n];
	q->arr[n] = key;
	n = m;
    }
    ++q->count;
}

void *
cucon_priq_pop_front(cucon_priq_t q)
{
    size_t i = 0;
    size_t n;
    void *result;
    if (q->count == 0)
	return NULL;
    n = --q->count;
    result = q->arr[0];
    q->arr[0] = q->arr[n];
    for (;;) {
	void *tmp;
	size_t j0 = 2*i + 1;
	if (j0 >= n)
	    break;
	if (cu_call(q->prior, q->arr[j0 + 1], q->arr[j0]))
	    ++j0;
	if (cu_call(q->prior, q->arr[i], q->arr[j0]))
	    break;
	tmp = q->arr[i];
	q->arr[i] = q->arr[j0];
	q->arr[j0] = tmp;
	i = j0;
    }
    if (q->count > 8 && q->count <= q->capacity/4)
	_reduce_by_4(q);
    return result;
}

void
cucon_priq_dump(cucon_priq_t q,
	      cu_clop(print_key_fn, void, void *key, FILE *out),
	      FILE *out)
{
    size_t i, j;
    size_t w = 1;
    for (i = j = 0; i < q->count; ++i, ++j) {
	if (j == w) {
	    w *= 2;
	    j = 0;
	    fprintf(out, "\n");
	}
	else if (i != 0)
	    fprintf(out, ", ");
	cu_call(print_key_fn, q->arr[i], out);
    }
    fprintf(out, "\n");
}

