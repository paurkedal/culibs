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

#include <cu/wordarr.h>
#include <cu/word.h>
#include <cu/bool.h>
#include <cu/util.h>

void
cu_wordarr_copy_bitimg(cu_bool1f_t f, size_t count, cu_word_t *dst,
		       cu_word_t const *src)
{
    switch (f) {
	case CU_BOOL1F_FALSE:
	    cu_wordarr_fill(count, dst, CU_WORD_C(0));
	    break;
	case CU_BOOL1F_TRUE:
	    cu_wordarr_fill(count, dst, ~CU_WORD_C(0));
	    break;
	case CU_BOOL1F_IDENT:
	    cu_wordarr_copy(count, dst, src);
	    break;
	case CU_BOOL1F_NOT:
	    cu_wordarr_copy_bitnot(count, dst, src);
	    break;
    }
}

void
cu_wordarr_copy_bitimg2(cu_bool2f_t f, size_t count, cu_word_t *dst,
			cu_word_t const *left, cu_word_t const *right)
{
    switch (f) {
	case CU_BOOL2F_FALSE:
	    cu_wordarr_fill(count, dst, CU_WORD_C(0));
	    break;

	case CU_BOOL2F_TRUE:
	    cu_wordarr_fill(count, dst, ~CU_WORD_C(0));
	    break;

	case CU_BOOL2F_LEFT:
	    right = left;
	    /* fall though */
	case CU_BOOL2F_RIGHT:
	    cu_wordarr_copy(count, dst, right);
	    break;

	case CU_BOOL2F_NOT_LEFT:
	    right = left;
	    /* fall though */
	case CU_BOOL2F_NOT_RIGHT:
	    cu_wordarr_copy_bitnot(count, dst, right);
	    break;

	/* Commutative */

	case CU_BOOL2F_NOR:
	    while (count--)
		*dst++ = ~(*left++ | *right++);
	    break;

	case CU_BOOL2F_XOR:
	    while (count--)
		*dst++ = *left++ ^ *right++;
	    break;

	case CU_BOOL2F_NAND:
	    while (count--)
		*dst++ = ~(*left++ & *right++);
	    break;

	case CU_BOOL2F_AND:
	    while (count--)
		*dst++ = *left++ & *right++;
	    break;

	case CU_BOOL2F_IFF:
	    while (count--)
		*dst++ = ~(*left++ ^ *right++);
	    break;

	case CU_BOOL2F_OR:
	    while (count--)
		*dst++ = *left++ | *right++;
	    break;

	/* Non-commutative */

	case CU_BOOL2F_NOT_AND:
	    CU_SWAP(cu_word_t const *, left, right);
	    /* fall though */
	case CU_BOOL2F_AND_NOT:
	    while (count--)
		*dst++ = *left++ & ~*right++;
	    break;

	case CU_BOOL2F_NOT_OR:
	    CU_SWAP(cu_word_t const *, left, right);
	    /* fall though */
	case CU_BOOL2F_OR_NOT:
	    while (count--)
		*dst++ = *left++ | ~*right++;
	    break;
    }
}

void
cu_wordarr_skewcopy(size_t count, cu_word_t *dst,
		    int src_offset, cu_word_t const *src)
{
    cu_word_t a;

    if (src_offset == 0) {
	cu_wordarr_copy(count, dst, src);
	return;
    }

    a = *src++;
    while (count--) {
	cu_word_t b = *src++;
	*dst++ = (a >> src_offset) | (b << (CU_WORD_WIDTH - src_offset));
	a = b;
    }
}

void
cu_wordarr_skewcopy_bitnot(size_t count, cu_word_t *dst,
			   int src_offset, cu_word_t const *src)
{
    cu_word_t a;

    if (src_offset == 0) {
	cu_wordarr_copy_bitnot(count, dst, src);
	return;
    }

    a = *src++;
    while (count--) {
	cu_word_t b = *src++;
	*dst++ = ~((a >> src_offset) | (b << (CU_WORD_WIDTH - src_offset)));
	a = b;
    }
}

void
cu_wordarr_skewcopy_bitimg(cu_bool1f_t f, size_t count, cu_word_t *dst,
			   int src_offset, cu_word_t const *src)
{
    switch (f) {
	case CU_BOOL1F_FALSE:
	    cu_wordarr_fill(count, dst, CU_WORD_C(0));
	    break;

	case CU_BOOL1F_TRUE:
	    cu_wordarr_fill(count, dst, ~CU_WORD_C(0));
	    break;

	case CU_BOOL1F_IDENT:
	    cu_wordarr_skewcopy(count, dst, src_offset, src);
	    break;

	case CU_BOOL1F_NOT:
	    cu_wordarr_skewcopy_bitnot(count, dst, src_offset, src);
	    break;
    }
}

static void
_wordarr_semiskew_bitimg2(cu_bool2f_t f, size_t count, cu_word_t *dst,
			  cu_word_t const *src0,
			  int offset1, cu_word_t const *src1)
{
    cu_word_t a1, b1, x1;

    a1 = *src1++;
    switch (f) {

	/* Commutative */

	case CU_BOOL2F_NOR:
	    while (count--) {
		b1 = *src1++;
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = ~(*src0++ | x1);
		a1 = b1;
	    }
	    break;

	case CU_BOOL2F_XOR:
	    while (count--) {
		b1 = *src1++;
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = *src0++ ^ x1;
		a1 = b1;
	    }
	    break;

	case CU_BOOL2F_NAND:
	    while (count--) {
		b1 = *src1++;
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = ~(*src0++ & x1);
		a1 = b1;
	    }
	    break;

	case CU_BOOL2F_AND:
	    while (count--) {
		b1 = *src1++;
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = *src0++ & x1;
		a1 = b1;
	    }
	    break;

	case CU_BOOL2F_IFF:
	    while (count--) {
		b1 = *src1++;
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = ~(*src0++ ^ x1);
		a1 = b1;
	    }
	    break;

	case CU_BOOL2F_OR:
	    while (count--) {
		b1 = *src1++;
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = *src0++ | x1;
		a1 = b1;
	    }
	    break;

	/* Non-commutative */

	case CU_BOOL2F_NOT_AND:
	    while (count--) {
		b1 = *src1++;
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = ~*src0++ & x1;
		a1 = b1;
	    }
	    break;

	case CU_BOOL2F_AND_NOT:
	    while (count--) {
		b1 = *src1++;
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = *src0++ & ~x1;
		a1 = b1;
	    }
	    break;

	case CU_BOOL2F_NOT_OR:
	    while (count--) {
		b1 = *src1++;
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = ~*src0++ | x1;
		a1 = b1;
	    }
	    break;

	case CU_BOOL2F_OR_NOT:
	    while (count--) {
		b1 = *src1++;
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = *src0++ | ~x1;
		a1 = b1;
	    }
	    break;

	default:
	    cu_debug_unreachable();
    }
}

void
cu_wordarr_skewcopy_bitimg2(cu_bool2f_t f, size_t count, cu_word_t *dst,
			    int offset0, cu_word_t const *src0,
			    int offset1, cu_word_t const *src1)
{
    cu_word_t a0, a1, b0, b1, x0, x1;

    /* Dispatch nullary and unary operations to corresponding fuctions. */
    switch (f) {
	case CU_BOOL2F_FALSE:
	    cu_wordarr_fill(count, dst, CU_WORD_C(0));
	    return;

	case CU_BOOL2F_TRUE:
	    cu_wordarr_fill(count, dst, ~CU_WORD_C(0));
	    return;

	case CU_BOOL2F_LEFT:
	    src1 = src0;
	    offset1 = offset0;
	    /* fall though */
	case CU_BOOL2F_RIGHT:
	    cu_wordarr_skewcopy(count, dst, offset1, src1);
	    return;

	case CU_BOOL2F_NOT_LEFT:
	    src1 = src0;
	    offset1 = offset0;
	    /* fall though */
	case CU_BOOL2F_NOT_RIGHT:
	    cu_wordarr_skewcopy_bitnot(count, dst, offset1, src1);
	    return;

	default:
	    break;
    }

    /* The case where offset0 or offset1 is zero must be handled specially not
     * only for efficiency, but also to avoid reading past the end of src0 or
     * src1. */
    if (offset0 == 0) {
	if (offset1 == 0)
	    cu_wordarr_copy_bitimg2(f, count, dst, src0, src1);
	else
	    _wordarr_semiskew_bitimg2(f, count, dst, src0, offset1, src1);
	return;
    }
    else if (offset1 == 0) {
	_wordarr_semiskew_bitimg2(cu_bool2f_swap(f), count, dst,
				  src1, offset0, src0);
	return;
    }

    /* Compute binary operations with non-null offsets. */
    a0 = *src0++;  a1 = *src1++;
    switch (f) {

	/* Commutative */

	case CU_BOOL2F_NOR:
	    while (count--) {
		b0 = *src0++;  b1 = *src1++;
		x0 = (a0 >> offset0) | (b0 << (CU_WORD_WIDTH - offset0));
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = ~(x0 | x1);
		a0 = b0;  a1 = b1;
	    }
	    break;

	case CU_BOOL2F_XOR:
	    while (count--) {
		b0 = *src0++;  b1 = *src1++;
		x0 = (a0 >> offset0) | (b0 << (CU_WORD_WIDTH - offset0));
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = x0 ^ x1;
		a0 = b0;  a1 = b1;
	    }
	    break;

	case CU_BOOL2F_NAND:
	    while (count--) {
		b0 = *src0++;  b1 = *src1++;
		x0 = (a0 >> offset0) | (b0 << (CU_WORD_WIDTH - offset0));
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = ~(x0 & x1);
		a0 = b0;  a1 = b1;
	    }
	    break;

	case CU_BOOL2F_AND:
	    while (count--) {
		b0 = *src0++;  b1 = *src1++;
		x0 = (a0 >> offset0) | (b0 << (CU_WORD_WIDTH - offset0));
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = x0 & x1;
		a0 = b0;  a1 = b1;
	    }
	    break;

	case CU_BOOL2F_IFF:
	    while (count--) {
		b0 = *src0++;  b1 = *src1++;
		x0 = (a0 >> offset0) | (b0 << (CU_WORD_WIDTH - offset0));
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = ~(x0 ^ x1);
		a0 = b0;  a1 = b1;
	    }
	    break;

	case CU_BOOL2F_OR:
	    while (count--) {
		b0 = *src0++;  b1 = *src1++;
		x0 = (a0 >> offset0) | (b0 << (CU_WORD_WIDTH - offset0));
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = x0 | x1;
		a0 = b0;  a1 = b1;
	    }
	    break;

	/* Non-commutative */

	case CU_BOOL2F_NOT_AND:
	    while (count--) {
		b0 = *src0++;  b1 = *src1++;
		x0 = (a0 >> offset0) | (b0 << (CU_WORD_WIDTH - offset0));
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = ~x0 & x1;
		a0 = b0;  a1 = b1;
	    }
	    break;

	case CU_BOOL2F_AND_NOT:
	    while (count--) {
		b0 = *src0++;  b1 = *src1++;
		x0 = (a0 >> offset0) | (b0 << (CU_WORD_WIDTH - offset0));
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = x0 & ~x1;
		a0 = b0;  a1 = b1;
	    }
	    break;

	case CU_BOOL2F_NOT_OR:
	    while (count--) {
		b0 = *src0++;  b1 = *src1++;
		x0 = (a0 >> offset0) | (b0 << (CU_WORD_WIDTH - offset0));
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = ~x0 | x1;
		a0 = b0;  a1 = b1;
	    }
	    break;

	case CU_BOOL2F_OR_NOT:
	    while (count--) {
		b0 = *src0++;  b1 = *src1++;
		x0 = (a0 >> offset0) | (b0 << (CU_WORD_WIDTH - offset0));
		x1 = (a1 >> offset1) | (b1 << (CU_WORD_WIDTH - offset1));
		*dst++ = x0 | ~x1;
		a0 = b0;  a1 = b1;
	    }
	    break;

	default:
	    cu_debug_unreachable();
    }
}
