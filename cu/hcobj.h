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

#ifndef CU_HCOBJ_H
#define CU_HCOBJ_H

#include <cu/fwd.h>
#include <cu/conf.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_hcobj_h cu/hcobj.h: Hash-consed Object Headers
 *@{\ingroup cu_mod
 * This header defines a macro \ref CU_HCOBJ to put right after the opening
 * brace of a hash-consed object struct:
 * \code
 * struct my_obj {
 *     CU_HCOBJ
 *     // key fields
 *     // value fields
 * };
 * \endcode
 * Note that there is no semicolon after the macro.  A macro is used here,
 * because the need for the header depends on the configuration.
 */

typedef struct cu_hcobj_s *cu_hcobj_t;

#ifdef CUCONF_ENABLE_GC_DISCLAIM

#if CUCONF_SIZEOF_LONG > 4
#  define CU_HC_GENERATION 0
#else
#  define CU_HC_GENERATION 0
#endif
#define CU_HCOBJ_NEEDED 1

/*!Put this, without semicolon, at the beginning of struct bodies which are
 * used for hash-consed objects. */
#define CU_HCOBJ CU_OBJ cu_inherit (cu_hcobj_s);

/*!The offset of the key-part of hash-consed object struct. */
#define CU_HCOBJ_SHIFT sizeof(struct cu_hcobj_s)

#define CU_HCOBJ_INIT CU_OBJ_INIT { 0 },

/*!Internal, use the \ref CU_HCOBJ macro instead. */
struct cu_hcobj_s
{
    AO_t hcset_next;
#if CU_HC_GENERATION
    AO_t generation;
#endif
};

#else /* !CUCONF_ENABLE_GC_DISCLAIM */

#define CU_HCOBJ
#define CU_HCOBJ_SHIFT 0
#define CU_HCOBJ_INIT

#endif /* !CUCONF_ENABLE_GC_DISCLAIM */

#define CU_HCOBJ_SHIFTW (CU_HCOBJ_SHIFT/CU_WORD_SIZE)

#define CU_HCOBJ_KEY(obj) ((void *)((char *)(obj) + CU_HCOBJ_SHIFT))

#define CU_HCOBJ_KEY_SIZEW(struct_size) \
    (CUDYN_OBJ_ALLOC_SIZEG(struct_size)*CU_GRAN_SIZEW - 1 - CU_HCOBJ_SHIFTW)

#define CU_HCOBJ_KEY_SIZE(struct_size) \
    (CU_HCOBJ_KEY_SIZEW(struct_size)*CU_WORD_SIZE)

/*!@}*/
CU_END_DECLARATIONS

#endif
