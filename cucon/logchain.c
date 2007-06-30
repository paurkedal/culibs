/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
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

#define cuconP_COMPILING_LOGCHAIN_C
#include <cucon/logchain.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cu/int.h>
#include <assert.h>
#include <string.h>


/* Local debugging
 * --------------- */

/* FIXME. Memory leaked while debugging since we revert to cucon_pmap_t
 * instead of cucon_wmap_t.  Don't enable debugging here. */
#define CUCON_LOGCHAIN_NDEBUG 1

#ifndef CUCON_LOGCHAIN_NDEBUG

#define cucon_wmap_t cucon_pmap_t
#define cucon_wmap_new cucon_pmap_new
#define cucon_wmap_find_mem cucon_pmap_find_mem
#define cucon_wmap_insert_mem cucon_pmap_insert_mem

static int cuconP_logchain_is_initialised;
static cucon_wmap_t cuconP_logchain_depth_map;

static void
cuconP_debug_logchain_assert_depth(void *lch, cucon_logchain_depth_t depth)
{
    size_t n_link;
    cucon_logchain_depth_t *real_depth;
    void *base;
    assert(cuconP_logchain_is_initialised);
    if (depth == 0)
	return;
    n_link = cu_uint_log2_lowbit(depth) + 1;
    base = cu_gc_base(lch);
    real_depth = cucon_wmap_find_mem(cuconP_logchain_depth_map, base);
    if (!real_depth)
	cu_debug_error("Memory at %p is not a logchain.", lch);
    else if (*real_depth != depth)
	cu_debug_error("Logchain at %p has depth %p, not %p",
			*real_depth, depth);
    else if (base != lch - n_link*sizeof(void *))
	cu_debug_error("Pointer %p to logchain is shifted, should be %p.",
			lch, base + n_link*sizeof(void *));
}

static void
cuconP_debug_logchain_set_depth(void *lch, cucon_logchain_depth_t depth)
{
    size_t n_link;
    void *base;
    cucon_logchain_depth_t *depth_slot;
    assert(cuconP_logchain_is_initialised);
    if (depth == 0)
	return;
    n_link = cu_uint_log2_lowbit(depth) + 1;
    base = cu_gc_base(lch);
    assert(base == lch - n_link*sizeof(void *));
    if (!cucon_wmap_insert_mem(cuconP_logchain_depth_map, base,
			     sizeof(cucon_logchain_depth_t),
			     &depth_slot))
	assert(!"Not reached.");
    *depth_slot = depth;
}

#else
#define cuconP_debug_logchain_assert_depth(lch, depth) ((void)0)
#define cuconP_debug_logchain_set_depth(lch, depth) ((void)0)
#endif



/* Logchain Memory Allocation
 * --------------------------
 *
 * Inspired by William Pugh's article, "Skip Lists: A Probabilistic
 * Alternative to Balanced Trees", although this is a different
 * datatype meant to solve a different problem. */

void *
cucon_logchain_galloc(cucon_logchain_depth_t depth, size_t size,
		    void *prev_lch)
{
    void *lch;
    size_t n_link;
    size_t i;
    size_t pow2_bit;
    if (depth == 0)
	return cu_galloc(size);
    n_link = cu_uint_log2_lowbit(depth) + 1;
    lch = cu_galloc(n_link*sizeof(void *) + size) + n_link*sizeof(void *);
    cuconP_debug_logchain_set_depth(lch, depth);
    pow2_bit = 1;
    for (i = 0; i < n_link; ++i) {
	cuconP_LOGCHAIN_LINK(lch, i) = prev_lch
	    = cucon_logchain_find(prev_lch, depth - 1, depth - pow2_bit);
	pow2_bit <<= 1;
    }
    return lch;
}

void *
cuconP_logchain_find_nontrivial(void *src_lch, size_t src_depth, size_t dst_depth)
{
    size_t pow2_bit;
    cu_logsize_fast_t bit;

    cuconP_debug_logchain_assert_depth(src_lch, src_depth);
    assert(dst_depth < src_depth);
    pow2_bit = 1;
    bit = 0;
    for (;;) {
	size_t depth_next = src_depth & ~pow2_bit;
	if (depth_next < dst_depth)
	    break;
	else if (depth_next != src_depth) {
/* 	    assert(bit < (src_depth == 0? 0 : cu_uint_log2_lowbit(src_depth) + 1)); */
	    src_lch = cuconP_LOGCHAIN_LINK(src_lch, bit);
	    src_depth = depth_next;
	    if (src_depth == dst_depth)
		return src_lch;
	}
	pow2_bit <<= 1;
	++bit;
    }
    src_depth -= dst_depth;
    for (;;) {
	assert(bit > 0);
	pow2_bit >>= 1;
	--bit;
	if (pow2_bit & src_depth) {
/* 	    assert(bit < (src_depth == 0? 0 : cu_uint_log2_lowbit(src_depth) + 1)); */
	    src_lch = cuconP_LOGCHAIN_LINK(src_lch, bit);
	    src_depth -= pow2_bit;
	    if (!src_depth)
		return src_lch;
	}
    }
}



/* Initialise */

void
cuconP_logchain_init(void)
{
#ifndef CUCON_LOGCHAIN_NDEBUG
    assert(!cuconP_logchain_is_initialised);
    cuconP_logchain_is_initialised = 1;
    cuconP_logchain_depth_map = cucon_wmap_new();
#endif
}
