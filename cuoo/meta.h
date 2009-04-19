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

#ifndef CUOO_META_H
#define CUOO_META_H

#include <cuoo/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuoo_meta_h cuoo/meta.h: Tree Codes
 *@{\ingroup cuoo_mod */

/* Meta Descriptor Format
 *
 *   Objects:
 *	bit0 = 0
 *	bit1 = 0 (reserved)
 *	bit2..bitN = address of type descriptor divided by 4
 *
 *   For operations and other expression constructs the whole meta-word
 *   is considered to for the operator (opcode), and is composed of
 *	bit0 = 1
 *	bit1 = 0 (reserved)
 *	bit3..bit14 = arity
 *	bit14..bitN = distinguises operators of the same arity
 *   OBS. Leave arity less significant than the rest of the operator,
 *   so that client code can use '<' to distinuish special constructs
 *   from normal operators.
 */

#define CUEX_PRIoMETA PRIoPTR
#define CUEX_PRIuMETA PRIuPTR
#define CUEX_PRIxMETA PRIxPTR
#define CUEX_PRIXMETA PRIXPTR

#define CUEX_META_MASK(shift, width) \
    (((CUEX_META_C(1) << (width)) - CUEX_META_C(1)) << (shift))

#define CUEX_METAKIND_WIDTH	 2
#define CUEX_METAKIND_MASK	 3

#define CUEX_OPR_FLAGS_WIDTH	 1
#if CUCONF_SIZEOF_VOID_P == 4
#  define CUEX_OPR_ARITY_WIDTH	 9
#  define CUEX_OPR_SELECT_WIDTH	20
#elif CUCONF_SIZEOF_VOID_P == 8
#  define CUEX_OPR_ARITY_WIDTH	24
#  define CUEX_OPR_SELECT_WIDTH	37
#else
#  error Unexpected word size, hand-tune new case here.
#endif

#define CUEX_OPR_FLAGS_SHIFT	CUEX_METAKIND_WIDTH
#define CUEX_OPR_ARITY_SHIFT	(CUEX_OPR_FLAGS_SHIFT + CUEX_OPR_FLAGS_WIDTH)
#define CUEX_OPR_SELECT_SHIFT	(CUEX_OPR_ARITY_SHIFT + CUEX_OPR_ARITY_WIDTH)

#define CUEX_META_WIDTH (CUEX_OPR_SELECT_SHIFT + CUEX_OPR_SELECT_WIDTH)
#if CUEX_META_WIDTH != CUCONF_WIDTHOF_VOID_P
#  error CUEX_OPR_META_WIDTH does not add up to the width of a pointer
#endif

#define CUEX_OPR_FLAGS_MASK \
    CUEX_META_MASK(CUEX_OPR_FLAGS_SHIFT, CUEX_OPR_FLAGS_WIDTH)
#define CUEX_OPR_ARITY_MASK \
    CUEX_META_MASK(CUEX_OPR_ARITY_SHIFT, CUEX_OPR_ARITY_WIDTH)
#define CUEX_OPR_SELECT_MASK \
    CUEX_META_MASK(CUEX_OPR_SELECT_SHIFT, CUEX_OPR_SELECT_WIDTH)

#define CUEX_OPRFLAG_CTOR (CUEX_META_C(1) << CUEX_OPR_FLAGS_SHIFT)

typedef enum {
    cuex_meta_kind_type = 0,
    cuex_meta_kind_opr = 1,
    cuex_meta_kind_other = 2,	/* varmeta or extension */
    cuex_meta_kind_ignore = 3,	/* Collides with GC internal freelist link. */
} cuex_meta_kind_t;

#define cuex_meta_kind(meta) ((cuex_meta_kind_t)((meta) & 3))

/*!Cast a \ref cuex_meta_t to a \ref cuoo_type_s "cuoo_type_t", assumig \ref
 * cuex_meta_is_type(\a meta) is true. */
#define cuoo_type_from_meta(meta) ((cuoo_type_t)(meta))
/*!Cast a \ref cuoo_type_s "cuoo_type_t" to a \ref cuex_meta_t. */
#define cuoo_type_to_meta(ptr) ((cuex_meta_t)(ptr))

/* Meta codes: other.varmeta */
#define CUEXP_VARMETA_SELECT_WIDTH 3
#define CUEXP_VARMETA_SELECT_MASK  CUEX_META_C(7)
#define CUEXP_VARMETA_SELECT_VALUE CUEX_META_C(6)
#define CUEXP_VARMETA_WSIZE_SHIFT CUEXP_VARMETA_SELECT_WIDTH
#define CUEXP_VARMETA_WSIZE_WIDTH 2
#define CUEXP_VARMETA_WSIZE_MASK \
    CUEX_META_MASK(CUEXP_VARMETA_WSIZE_SHIFT, CUEXP_VARMETA_WSIZE_WIDTH)
#define cuexP_varmeta_wsize(meta) \
    ((meta & CUEXP_VARMETA_WSIZE_MASK) >> CUEXP_VARMETA_WSIZE_SHIFT)


/* Operators
 * ---------
 *
 * Operators are the same type as cuex_meta_t, no need for cast, but before
 * using cuex_opr_r, check that it is really an operator. */

/*!True iff \a meta is an operator, i.e. it is the meta of an operation. */
CU_SINLINE cu_bool_t
cuex_meta_is_opr(cuex_meta_t meta)
{ return cuex_meta_kind(meta) == cuex_meta_kind_opr; }

/*!The arity of the operator \a opr. */
CU_SINLINE cu_rank_t
cuex_opr_r(cuex_meta_t opr)
{ return ((opr) & CUEX_OPR_ARITY_MASK) >> CUEX_OPR_ARITY_SHIFT; }

/*!True iff \a meta is an operator of rank \a r. */
CU_SINLINE cu_bool_t
cuex_meta_is_opr_r(cu_rank_t r, cuex_meta_t meta)
{ return cuex_meta_is_opr(meta) && cuex_opr_r(meta) == r; }

/*!The meta of a dynamic object or operation \a obj. */
CU_SINLINE cuex_meta_t
cuex_meta(void *obj)
{ return *((cuex_meta_t *)obj - 1) - 1; }

/*!@}*/
CU_END_DECLARATIONS

#endif
