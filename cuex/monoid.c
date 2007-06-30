/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuex/monoid.h>
#include <cuex/opn.h>
#include <cu/int.h>

#define LOG2_FANOUT 2
#define FANOUT (1 << LOG2_FANOUT)
#define FANOUT_MASK (FANOUT - 1)


static cuex_opn_t
monoid_fresh(cuex_meta_t mult, unsigned int depth, cuex_monoid_it_t *it)
{
    cuex_t v[FANOUT];
    cu_rank_t i;
    if (depth == 0) {
	cuex_t x = cuex_monoid_it_read(it);
	cu_debug_assert(x);
	return x;
    }
    for (i = 0; i < FANOUT && !cuex_monoid_it_is_end(it); ++i)
	v[i] = monoid_fresh(mult, depth - 1, it);
    if (i == 1)
	return v[0];
    else
	return cuex_opn_by_arr(cuexP_monoid_opr(mult, depth, i), v);
}

static cuex_opn_t
monoid_fill(cuex_meta_t mult, unsigned xdepth, cuex_t x, cuex_monoid_it_t *it)
{
    cuex_t v[FANOUT];
    cu_rank_t i, r;
    unsigned int depth;
    if (!cuex_is_monoid_product(mult, x)) {
	if (xdepth == 0)
	    return x;
	v[0] = x;
	i = r = 1;
	depth = 1;
    }
    else {
	r = cuex_opn_arity(x);
	cu_debug_assert(r > 0);
	--r;
	for (i = 0; i < r; ++i)
	    v[i] = cuex_opn_at(x, i);
	depth = cuexP_monoid_meta_depth(cuex_meta(x));
	cu_debug_assert(depth > 0);
	if (depth > 1)
	    v[i] = monoid_fill(mult, depth - 1, cuex_opn_at(x, i), it);
	else
	    v[i] = cuex_opn_at(x, i);
	++i;
    }
    do {
	while (i < FANOUT && !cuex_monoid_it_is_end(it))
	    v[i++] = monoid_fresh(mult, depth - 1, it);
	v[0] = cuex_opn_by_arr(cuexP_monoid_opr(mult, depth, i), v);
	i = 1;
	++depth;
    } while (depth <= xdepth && !cuex_monoid_it_is_end(it));
    return v[0];
}

cuex_opn_t
cuex_monoid_product(cuex_meta_t mult, cuex_t x, cuex_t y)
{
    cuex_monoid_it_t it;
    unsigned int depth;
    if (cuex_is_monoid_identity(mult, x))
	return y;
    if (cuex_is_monoid_identity(mult, y))
	return x;
    cuex_monoid_it_cct(&it, mult, y);
    if (cuex_is_monoid_product(mult, x)) {
	depth = cuexP_monoid_meta_depth(cuex_meta(x));
	cu_debug_assert(depth > 0);
	x = monoid_fill(mult, depth, x, &it);
    }
    else
	depth = 0;
    while (!cuex_monoid_it_is_end(&it)) {
	cuex_t v[FANOUT];
	cu_rank_t i;
	++depth;
	v[0] = x;
	for (i = 1; i < FANOUT && !cuex_monoid_it_is_end(&it); ++i)
	    v[i] = monoid_fresh(mult, depth - 1, &it);
	x = cuex_opn_by_arr(cuexP_monoid_opr(mult, depth, i), v);
    }
    return x;
}

size_t
cuex_monoid_factor_cnt(cuex_meta_t mult, cuex_t x)
{
    if (cuex_is_monoid_product(mult, x)) {
	cuex_meta_t meta = cuex_meta(x);
	unsigned int k = cuexP_monoid_meta_depth(meta);
	cu_rank_t r = cuex_opr_r(meta);
	cu_debug_assert(k > 0);
	if (r == 0) {
	    cu_debug_assert(k == 1);
	    return 0;
	}
	--r;
	return r*(1 << (LOG2_FANOUT*(k - 1)))
	     + cuex_monoid_factor_cnt(mult, cuex_opn_at(x, r));
    }
    else
	return 1;
}

cuex_t
cuex_monoid_factor_at(cuex_meta_t mult, cuex_t x, size_t i)
{
    while (cuex_is_monoid_product(mult, x)) {
	unsigned int k;
	size_t subnode_size, j;
	cuex_meta_t meta = cuex_meta(x);
	k = cuexP_monoid_meta_depth(meta);
	subnode_size = 1 << LOG2_FANOUT*(k - 1);
	j = i / subnode_size;
	x = cuex_opn_at(x, j);
	i = i % subnode_size;
	--k;
    }
    cu_debug_assert(i == 0);
    return x;
}

cuex_t
cuex_monoid_factor_prefix(cuex_meta_t mult, cuex_t x, size_t n)
{
    if (n == 0)
	return cuex_monoid_identity(mult);
tailcall:
    if (cuex_is_monoid_product(mult, x)) {
	unsigned int k = cuexP_monoid_meta_depth(cuex_meta(x));
	cuex_t v[FANOUT];
	size_t subnode_size = 1 << LOG2_FANOUT*(k - 1);
	size_t m = n / subnode_size;
	size_t l = n % subnode_size;
	size_t i;
	if (m == 0) {
	    x = cuex_opn_at(x, 0);
	    goto tailcall;
	}
	if (m == 1 && l == 0)
	    return cuex_opn_at(x, 0);
	for (i = 0; i < m; ++i)
	    v[i] = cuex_opn_at(x, i);
	if (l) {
	    v[i] = cuex_monoid_factor_prefix(mult, cuex_opn_at(x, i), l);
	    ++i;
	}
	return cuex_opn_by_arr(cuexP_monoid_opr(mult, k, i), v);
    }
    cu_debug_assert(n == 1);
    return x;
}

cuex_t
cuex_monoid_factor_range(cuex_meta_t mult, cuex_t x, size_t i, size_t j)
{
    cuex_monoid_it_t it;
    cu_debug_assert(i <= j);
    if (j == 0)
	return cuex_monoid_identity(mult);
    if (i == 0)
	return cuex_monoid_factor_prefix(mult, x, j);
    cuex_monoid_it_cct_range(&it, mult, x, i, j);
    return monoid_fresh(mult, CUEX_MONOID_MAXPL_DEPTH - 1, &it);
}

void
cuex_monoid_it_cct(cuex_monoid_it_t *it, cuex_meta_t mult, cuex_t x)
{
    if (cuex_is_monoid_product(mult, x)) {
	unsigned int k = cuexP_monoid_meta_depth(cuex_meta(x));
	cu_debug_assert(k > 0);
	it->size = cuex_monoid_factor_cnt(mult, x);
	while (k) {
	    it->stack[k] = x;
	    cu_debug_assert(cuexP_monoid_meta_depth(cuex_meta(x)) == k);
	    x = cuex_opn_at(x, 0);
	    --k;
	}
    }
    else
	it->size = 1;
    it->stack[0] = x;
    it->index = 0;
    it->mult = mult;
}

void
cuex_monoid_it_cct_at(cuex_monoid_it_t *it,
		      cuex_meta_t mult, cuex_t x, size_t i)
{
    it->mult = mult;
    it->index = i;
    if (cuex_is_monoid_product(mult, x)) {
	unsigned int k = cuexP_monoid_meta_depth(cuex_meta(x));
	cu_debug_assert(k > 0);
	it->size = cuex_monoid_factor_cnt(mult, x);
	if (i == it->size)
	    return;
	cu_debug_assert(i <= it->size);
	while (k) {
	    size_t index = (i >> LOG2_FANOUT*(k - 1)) & FANOUT_MASK;
	    it->stack[k] = x;
	    if (cuex_is_monoid_product(mult, x)
		    && cuexP_monoid_meta_depth(cuex_meta(x)) == k)
		x = cuex_opn_at(x, index);
	    else
		cu_debug_assert(index == 0);
	    --k;
	}
    }
    else
	it->size = 1;
    it->stack[0] = x;
}

void
cuex_monoid_it_cct_range(cuex_monoid_it_t *it,
			 cuex_meta_t mult, cuex_t x, size_t i, size_t j)
{
    cuex_monoid_it_cct_at(it, mult, x, i);
    cu_debug_assert(j <= it->size);
    it->size = j;
}

cuex_t
cuex_monoid_it_read(cuex_monoid_it_t *it)
{
    size_t i = it->index;
    size_t j = i + 1;
    unsigned int k;
    cuex_t r;
    if (j >= it->size) {
	if (j > it->size)
	    return NULL;
	else {
	    it->index = j;
	    return it->stack[0];
	}
    }
    k = (cu_ulong_bit_count(i ^ j) - 1)/LOG2_FANOUT;
    r = it->stack[0];
    it->index = j;
    ++k;
    while (k > 0) {
	size_t index = (j >> LOG2_FANOUT*(k - 1)) & FANOUT_MASK;
	if (!cuex_is_monoid_product(it->mult, it->stack[k])) {
	    cu_debug_assert(index == 0);
	    --k;
	    it->stack[k] = it->stack[k + 1];
	}
	else if (cuexP_monoid_meta_depth(cuex_meta(it->stack[k])) < k) {
	    cu_debug_assert(index == 0);
	    --k;
	    it->stack[k] = it->stack[k + 1];
	}
	else {
	    --k;
	    it->stack[k] = cuex_opn_at(it->stack[k + 1], index);
	}
    }
    return r;
}

cu_clos_efun(cuex_monoid_factor_src,
	     cu_prot(size_t, cuex_t *buf_arr, size_t buf_size))
{
    cu_clos_self(cuex_monoid_factor_src);
    size_t i = buf_size;
    while (i) {
	*buf_arr = cuex_monoid_it_read(&self->it);
	if (!*buf_arr)
	    return buf_size - i;
	--i;
	++buf_arr;
    }
    return 0;
}

