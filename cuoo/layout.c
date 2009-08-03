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

#include <cuoo/layout.h>
#include <cu/debug.h>
#include <cu/int.h>
#include <cuoo/halloc.h>
#include <cuoo/intf.h>

#define HASH CU_HASH2
#define MAX_FREE_WIDTH_OF_ALT0 0
#define MAX_FREE_WIDTH_OF_ALT1 1  /* best */

cuoo_type_t cuoo_layout_meta;

#if MAX_FREE_WIDTH_OF_ALT0
static cu_offset_t
max_free_width_of_alt0(cu_offset_t lbound, cu_word_t alloc_mask)
{
#ifdef BITALLOC_FROM_MSB
    cu_word_t bit = 1;
    do {
	int cnt = 0;
	while ((alloc_mask & bit) == 0 && bit) {
	    ++cnt;
	    bit <<= 1;
	}
	if (cnt > lbound)
	    lbound = cnt;
	while ((alloc_mask & bit))
	    bit <<= 1;
    } while (bit);
#else
    cu_word_t bit = CU_WORD_C(1) << (sizeof(cu_word_t)*8 - 1);
    do {
	int cnt = 0;
	while ((alloc_mask & bit) == 0 && bit) {
	    ++cnt;
	    bit >>= 1;
	}
	if (cnt > lbound)
	    lbound = cnt;
	while ((alloc_mask & bit))
	    bit >>= 1;
    } while (bit);
#endif
    return lbound;
}
#endif
#if MAX_FREE_WIDTH_OF_ALT1
static cu_offset_t
max_free_width_of_alt1(cu_offset_t lbound, cu_word_t alloc_mask)
{
    /* Bits in alloc_mask are only allocated and never freed, but
     * fragmentation can occur due to alignment constraints.  */
#ifdef BITALLOC_FROM_MSB
    cu_word_t byte_mask = 0xff;
    do {
	int cnt = 0;
	cu_word_t bit_mask;
	while ((alloc_mask & byte_mask) == 0 && byte_mask) {
	    cnt += 8;
	    byte_mask <<= 8;
	}
	bit_mask = ~(byte_mask << 1) & byte_mask;
	while ((alloc_mask & bit_mask) == 0 && (byte_mask & bit_mask)) {
	    ++cnt;
	    bit_mask <<= 1;
	}
	if (cnt > lbound)
	    lbound = cnt;
	byte_mask <<= 8;
    } while (byte_mask);
#else
    cu_word_t byte_mask = (cu_word_t)0xff << (sizeof(cu_word_t)*8 - 8);
    do {
	int cnt = 0;
	cu_word_t bit_mask;
	while ((alloc_mask & byte_mask) == 0 && byte_mask) {
	    cnt += 8;
	    byte_mask >>= 8;
	}
	bit_mask = ~(byte_mask >> 1) & byte_mask;
	while ((alloc_mask & bit_mask) == 0 && (byte_mask & bit_mask)) {
	    ++cnt;
	    bit_mask >>= 1;
	}
	if (cnt > lbound)
	    lbound = cnt;
	byte_mask >>= 8;
    } while (byte_mask);
#endif
    return lbound;
}
#endif
#if MAX_FREE_WIDTH_OF_ALT0 && MAX_FREE_WIDTH_OF_ALT1
static clock_t max_free_width_of_time0 = 0;
static clock_t max_free_width_of_time1 = 0;
static void max_free_width_of_report()
{
    fprintf(stderr,
	    "benchmark: max_free_width_of_alt0: %lg\n"
	    "benchmark: max_free_width_of_alt1: %lg\n",
	    max_free_width_of_time0/(double)CLOCKS_PER_SEC,
	    max_free_width_of_time1/(double)CLOCKS_PER_SEC);
}
static cu_offset_t
max_free_width_of(cu_offset_t lbound, cu_word_t alloc_mask)
{
    cu_offset_t r0, r1;
    static int done_init = 0;
    if (!done_init) {
	atexit(max_free_width_of_report);
	done_init = 1;
    }
    max_free_width_of_time0 -= clock();
    r0 = max_free_width_of_alt0(lbound, alloc_mask);
    max_free_width_of_time0 += clock();
    max_free_width_of_time1 -= clock();
    r1 = max_free_width_of_alt1(lbound, alloc_mask);
    max_free_width_of_time1 += clock();
    cu_debug_assert(r0 == r1);
    return r0;
}
#else
#  if MAX_FREE_WIDTH_OF_ALT0
#    define max_free_width_of max_free_width_of_alt0
#  else
#    define max_free_width_of max_free_width_of_alt1
#  endif
#endif

#if 0
static int
free_width(cu_word_t m)
{
    int i;
    int j = 0;
    int k = 0;
    for (i = 0; i < sizeof(cu_word_t)*8; ++i) {
	if (m & ((cu_word_t)1 << i))
	    j = 0;
	else if (++j > k)
	    k = j;
    }
    return k;
}
#endif

static cuoo_layout_t
layout_new_raw(cuoo_layout_t prefix, cu_word_t alloc_mask,
	       cu_offset_t bitalign)
{
    cuoo_layout_t layout;
    cuoo_hctem_decl(cuoo_layout, tem);
    cuoo_hctem_init(cuoo_layout, tem);
    layout = cuoo_hctem_get(cuoo_layout, tem);
    layout->prefix = prefix;
    layout->alloc_mask = alloc_mask;
    layout->bitalign = bitalign;
    if (prefix) {
	layout->bitoffset = prefix->bitoffset + sizeof(cu_word_t)*8;
	layout->max_free_width = max_free_width_of(prefix->max_free_width,
						    alloc_mask);
	if (prefix->bitalign > bitalign)
	    layout->bitalign = prefix->bitalign;
    }
    else {
	layout->bitoffset = 0;
	layout->max_free_width = max_free_width_of(0, alloc_mask);
    }
    return cuoo_hctem_new(cuoo_layout, tem);
}

static cuoo_layout_t
layout_new_pack(cuoo_layout_t lyo, cu_offset_t bitsize, cu_offset_t bitalign,
		cu_offset_t *bitoffset)
{
    cuoo_layout_t prefix = lyo->prefix;
    if (prefix && bitsize <= prefix->max_free_width) {
	prefix = layout_new_pack(prefix, bitsize, bitalign,
				 bitoffset);
	if (lyo->bitalign > bitalign)
	    bitalign = lyo->bitalign;
	return layout_new_raw(prefix, lyo->alloc_mask, bitalign);
    }
    else {
	cu_word_t alloc_mask = lyo->alloc_mask;
	cu_offset_t local_bitoffset;
	cu_word_t mask;
	cu_debug_assert(0 < bitsize && bitsize < sizeof(cu_word_t)*8);
#	ifdef BITALLOC_FROM_MSB
	    local_bitoffset = sizeof(cu_word_t)*8 - bitsize;
	    mask = ((CU_WORD_C(1) << bitsize) - 1) << local_bitoffset;
#	    define FIND_OFFSET(bitalign)			\
		while ((mask & alloc_mask)) {			\
		    mask >>= bitalign;				\
		    local_bitoffset -= bitalign;		\
		    cu_debug_assert(mask);			\
		}
#	else
	    local_bitoffset = 0;
	    mask = (CU_WORD_C(1) << bitsize) - 1;
#	    define FIND_OFFSET(bitalign)			\
		while ((mask & alloc_mask)) {			\
		    mask <<= bitalign;				\
		    local_bitoffset += bitalign;		\
		    cu_debug_assert(mask);			\
		}
#	endif
	switch (bitalign) {
	    case 1: FIND_OFFSET(1); break;
	    case 8: FIND_OFFSET(8); break;
#if CU_WORD_WIDTH == 16
	    case 16: cu_debug_assert((mask & alloc_mask) == 0); break;
#else
	    case 16: FIND_OFFSET(16); break;
#  if CU_WORD_WIDTH == 32
	    case 32: cu_debug_assert((mask & alloc_mask) == 0); break;
#  else
	    case 32: FIND_OFFSET(32); break;
#    if CU_WORD_WIDTH == 64 /* for us */
	    case 64: cu_debug_assert((mask & alloc_mask) == 0); break;
#    else
#      if CU_WORD_WIDTH == 128 /* for our descendants */
	    case 128: cu_debug_assert((mask & alloc_mask) == 0); break;
#      elif CU_WORD_WIDTH < 128
#	 error Unexpected CU_WORD_WIDTH, check the configuration.
#      else
#	 error Word width > 64?  Please add a case here.
#      endif
#    endif
#  endif
#endif
	    default: cu_debug_unreachable(); break;
	}
#	undef FIND_OFFSET
	alloc_mask |= mask;
	*bitoffset = lyo->bitoffset + local_bitoffset;
	if (bitalign < lyo->bitalign)
	    bitalign = lyo->bitalign;
	lyo = layout_new_raw(prefix, alloc_mask, bitalign);
	cu_debug_assert(mask != (cu_word_t)-1 ||
			 lyo->max_free_width == (lyo->prefix? 0 : lyo->prefix->max_free_width));
	return lyo;
    }
}

cuoo_layout_t
cuoo_layout_pack_bits(cuoo_layout_t prefix,
		      cu_offset_t bitsize, cu_offset_t bitalign,
		      cu_offset_t *bitoffset)
{
    if (bitsize == 0) {
	*bitoffset = 0;
	return prefix;
    }
    if (prefix && bitsize <= prefix->max_free_width)
	return layout_new_pack(prefix, bitsize, bitalign, bitoffset);
    else {
	cu_word_t alloc_mask;
	cuoo_layout_t lyo;
	while (bitsize > sizeof(cu_word_t)*8) {
	    prefix = layout_new_raw(prefix, (cu_word_t)-1, bitalign);
	    bitsize -= sizeof(cu_word_t)*8;
	}
	if (bitsize == sizeof(cu_word_t)*8)
	    alloc_mask = (cu_word_t)-1;
	else {
#ifdef BITALLOC_FROM_MSB
	    alloc_mask = ((CU_WORD_C(1) << bitsize) - 1)
			 << (sizeof(cu_word_t)*8 - bitsize);
#else
	    alloc_mask = (CU_WORD_C(1) << bitsize) - 1;
#endif
	}
	lyo = layout_new_raw(prefix, alloc_mask, bitalign);
#ifdef BITALLOC_FROM_MSB
	*bitoffset = lyo->bitoffset + sizeof(cu_word_t)*8 - bitsize;
#else
	*bitoffset = lyo->bitoffset;
#endif
	return lyo;
    }
}

cuoo_layout_t
cuoo_layout_product(cuoo_layout_t prefix, cuoo_layout_t member,
		    cu_offset_t *bitoffset)
{
    cu_offset_t member_width;
    if (!prefix) {
	*bitoffset = 0;
	return member;
    }
    member_width = cuoo_layout_bitsize(member);
    if (member_width < prefix->max_free_width) {
	cu_debug_assert(!member->prefix);
	return cuoo_layout_pack_bits(prefix, member_width, member->bitalign,
				      bitoffset);
    }
    else {
	cu_offset_t N = member->bitoffset/(sizeof(cu_word_t)*8) + 1;
	cu_offset_t n = N;
	cuoo_layout_t *memb_arr = cu_salloc(N*sizeof(cuoo_layout_t));
	while (--n) {
	    memb_arr[n] = member;
	    member = member->prefix;
	}
	cu_debug_assert(member && !member->prefix);
	prefix = layout_new_raw(prefix, member->alloc_mask, member->bitalign);
	*bitoffset = prefix->bitoffset;
	++memb_arr;
	for (n = 1; n < N; ++n) {
	    prefix = layout_new_raw(prefix,
				    (*memb_arr)->alloc_mask,
				    (*memb_arr)->bitalign);
	    ++memb_arr;
	}
	return prefix;
    }
}

static cuoo_layout_t
layout_union_balanced(cuoo_layout_t l0, cuoo_layout_t l1)
{
    cuoo_layout_t l0p;
    cu_offset_t ba;
    if (l0 == l1)
	return l0;
    l0p = l0->prefix;
    if (l0p) {
	cuoo_layout_t l1p = l1->prefix;
	cu_debug_assert(l1p);
	l0p = layout_union_balanced(l0p, l1p);
    }
    ba = l0->bitalign;
    if (l1->bitalign > ba)
	ba = l1->bitalign;
    return layout_new_raw(l0p, l0->alloc_mask | l1->alloc_mask, ba);
}

static cuoo_layout_t
layout_union_left_heavy(cuoo_layout_t l0, cuoo_layout_t l1)
{
    cuoo_layout_t l0p = l0->prefix;
    if (l0p->bitoffset > l1->bitoffset)
	l0p = layout_union_left_heavy(l0p, l1);
    else
	l0p = layout_union_balanced(l0p, l1);
    return layout_new_raw(l0p, l0->alloc_mask, l0->bitalign);
}

cuoo_layout_t
cuoo_layout_union(cuoo_layout_t l0, cuoo_layout_t l1)
{
    if (l0->bitoffset > l1->bitoffset)
	return layout_union_left_heavy(l0, l1);
    else if (l1->bitoffset > l0->bitoffset)
	return layout_union_left_heavy(l1, l0);
    else
	return layout_union_balanced(l0, l1);
}

cu_offset_t
cuoo_layout_bitsize(cuoo_layout_t lyo)
{
    if (!lyo)
	return 0;
    else if (lyo->prefix)
	return lyo->bitoffset + sizeof(cu_word_t)*8;
    else
	return sizeof(cu_word_t)*8 - lyo->max_free_width;
}

cu_offset_t
cuoo_layout_size(cuoo_layout_t lyo)
{
    size_t bitsize;
    if (!lyo)
	return 0;
    else if (lyo->prefix)
	bitsize = lyo->bitoffset + sizeof(cu_word_t)*8;
    else
	bitsize = sizeof(cu_word_t)*8 - lyo->max_free_width;
    return (bitsize + 7)/8;
}

static void
_layout_dump(cuoo_layout_t lyo, FILE *out)
{
    int i;
    if (lyo->prefix)
	_layout_dump(lyo->prefix, out);
    fputs("    ", out);
    for (i = sizeof(cu_word_t)*8; i;) {
	--i;
	fputc(lyo->alloc_mask & (CU_WORD_C(1) << i)? '1' : '0', out);
	if (i % 8 == 0)
	    fputc(' ', out);
    }
    fprintf(out, "aligned on %d bits, max %d bits cont.\n",
	    lyo->bitalign, lyo->max_free_width);
}
void
cuoo_layout_dump(cuoo_layout_t lyo, FILE *out)
{
    fprintf(out, "layout @ %p\n", (void *)lyo);
    _layout_dump(lyo, out);
}

cuoo_type_t cuooP_layout_type;
cuoo_layout_t cuooP_layout_ptr;
cuoo_layout_t cuooP_layout_void;

static void
_layout_print(cuex_t e, FILE *out)
{ cuoo_layout_dump(e, out); }

static cu_box_t
_layout_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(_layout_print);
	default:
	    return CUOO_IMPL_NONE;
    }
}

void
cuooP_layout_init()
{
    cu_offset_t ignore;
    cuooP_layout_type =
	cuoo_type_new_opaque_hcs(_layout_impl,
			     sizeof(struct cuoo_layout_s) - CUOO_HCOBJ_SHIFT);
    cuooP_layout_ptr = cuoo_layout_pack_bits(NULL,
					  sizeof(void *)*8,
					  sizeof(void *)*8, &ignore);
    cuooP_layout_void = cuoo_layout_pack_bits(NULL, 0, 0, &ignore);
}
