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

#ifndef CUOO_LAYOUT_H
#define CUOO_LAYOUT_H

#include <cuoo/fwd.h>
#include <cuoo/hcobj.h>
#include <cu/conf.h>
#include <cu/inherit.h>
#include <stdio.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuoo_layout_h cuoo/layout.h: Packing Memory-Layout Allocator
 * @{\ingroup cuoo_mod */

struct cuoo_layout_s
{
    CUOO_HCOBJ
    cuoo_layout_t prefix;
    cu_word_t alloc_mask;	/* alloc_mask == 0 means pointer */
    unsigned short bitalign;
    unsigned short max_free_width;
    cu_offset_t bitoffset;
};
extern cuoo_stdtype_t cuooP_layout_type;
#define cuoo_layout_type() cuoo_stdtype_to_type(cuooP_layout_type)

#define cuoo_layout_prefix(oml) ((oml)->prefix)
#define cuoo_layout_bits_alloc_mask(oml) ((oml)->alloc_mask)

/*!The layout of \a prefix with \a bitsize bits aligned on \a bitalign bits
 * packed inside or after.  The bit-offset of the allocated bits is stored in
 * <tt>*\a bitoffset</tt>.  \pre \a bitalign must be 1 (bitfield), 8 (a
 * byte), or a larger power of two. */
cuoo_layout_t cuoo_layout_pack_bits(cuoo_layout_t prefix,
				      cu_offset_t bitsize, cu_offset_t bitalign,
				      cu_offset_t *bitoffset);

/*!The layout of \a lyo0 with \a lyo1 packed inside or appended. */
cuoo_layout_t cuoo_layout_product(cuoo_layout_t lyo0, cuoo_layout_t lyo1,
				    cu_offset_t *bitoffset);

/*!Return a layout which can hold the union of \a lyo0 and \a lyo1.  Holes
 * present in both arguments are preserved. */
cuoo_layout_t cuoo_layout_union(cuoo_layout_t lyo0, cuoo_layout_t lyo1);

#if 0 /* unimplemeted */
/* The layout of \a prefix with \a bitsize bits aligned on \a bitalign bits
 * appended. */
cuoo_layout_t cuoo_layout_append_bits(cuoo_layout_t prefix,
					cu_offset_t bitsize, cu_offset_t bitalign,
					cu_offset_t *bitoffset);

/* The layout of \a prefix with \a member appended to the end. */
cuoo_layout_t cuoo_layout_append(cuoo_layout_t prefix,
				   cuoo_layout_t member,
				   cu_offset_t *bitoffset);
#endif

/*!The number of bytes needed to represent \a layout rounded up to
 * a multiple of its alignment. */
cu_offset_t cuoo_layout_size(cuoo_layout_t layout);

/*!The number of bits needed to represent \a lyo. */
cu_offset_t cuoo_layout_bitsize(cuoo_layout_t lyo);

/*!The alignment of \a layout in bytes, rounded upwards. */
CU_SINLINE cu_offset_t cuoo_layout_align(cuoo_layout_t layout)
{ return layout? (layout->bitalign + 7)/8 : 1; }

/*!The alignment of \a layout in bits.  This is 1 for bitfields, otherwise
 * 8, 16, etc up to the word size. */
CU_SINLINE cu_offset_t cuoo_layout_bitalign(cuoo_layout_t layout)
{ return layout? layout->bitalign : 1; }

extern cuoo_layout_t cuooP_layout_ptr;
extern cuoo_layout_t cuooP_layout_void;

/*!The layout of a pointer. */
CU_SINLINE cuoo_layout_t cuoo_layout_ptr()
{ return cuooP_layout_ptr; }

/*!The empty layout, this may be represented by \c NULL. */
CU_SINLINE cuoo_layout_t cuoo_layout_void()
{ return cuooP_layout_void; }

/*!Dump of \a lyo to \a out for debugging. */
void cuoo_layout_dump(cuoo_layout_t lyo, FILE *out);

/*!@}*/
CU_END_DECLARATIONS

#endif
