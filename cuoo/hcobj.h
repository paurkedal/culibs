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

#ifndef CUOO_HCOBJ_H
#define CUOO_HCOBJ_H

#include <cuoo/fwd.h>
#include <cu/conf.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuoo_hcobj_h cuoo/hcobj.h: Hash-consed Object Headers
 *@{\ingroup cuoo_mod
 * This header defines a macro \ref CUOO_HCOBJ to put right after the opening
 * brace of a hash-consed object struct:
 * \code
 * struct my_obj {
 *     CUOO_HCOBJ
 *     // key fields
 *     // value fields
 * };
 * \endcode
 * Note that there is no semicolon after the macro.  A macro is used here,
 * because the need for the header depends on the configuration.
 */

typedef struct cuooP_hcobj_s *cuooP_hcobj_t;

#ifdef CUCONF_ENABLE_GC_DISCLAIM

#if CUCONF_SIZEOF_LONG > 4
#  define CUOO_HC_GENERATION 0
#else
#  define CUOO_HC_GENERATION 0
#endif
#define CUOO_HCOBJ_NEEDED 1

/*!Put this, without semicolon, at the beginning of struct bodies which are
 * used for hash-consed objects. */
#define CUOO_HCOBJ CUOO_OBJ cu_inherit (cuooP_hcobj_s);

/*!The offset of the key-part of hash-consed object struct. */
#define CUOO_HCOBJ_SHIFT sizeof(struct cuooP_hcobj_s)

#define CUOO_HCOBJ_INIT CUOO_OBJ_INIT { 0 },

/*!Internal, use the \ref CUOO_HCOBJ macro instead. */
struct cuooP_hcobj_s
{
    AO_t hcset_next;
#if CUOO_HC_GENERATION
    AO_t generation;
#endif
};

#else /* !CUCONF_ENABLE_GC_DISCLAIM */

#define CUOO_HCOBJ
#define CUOO_HCOBJ_SHIFT 0
#define CUOO_HCOBJ_INIT

#endif /* !CUCONF_ENABLE_GC_DISCLAIM */

#define CUOO_HCOBJ_SHIFTW (CUOO_HCOBJ_SHIFT/CU_WORD_SIZE)

#define CUOO_HCOBJ_KEY(obj) ((void *)((char *)(obj) + CUOO_HCOBJ_SHIFT))

#if 0 /* These were only used in idr.c */
#define CUOO_HCOBJ_KEY_SIZEW(struct_size) \
    (CUOO_OBJ_ALLOC_SIZEG(struct_size)*CU_GRAN_SIZEW - 1 - CUOO_HCOBJ_SHIFTW)

#define CUOO_HCOBJ_KEY_SIZE(struct_size) \
    (CUOO_HCOBJ_KEY_SIZEW(struct_size)*CU_WORD_SIZE)
#endif

/*!@}*/
CU_END_DECLARATIONS

#endif
