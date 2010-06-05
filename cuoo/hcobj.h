/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2010  Petter Urkedal <paurkedal@eideticdew.org>
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
#include <cu/inherit.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cuoo_hcobj_h cuoo/hcobj.h: Hash-consed Object Headers
 ** @{ \ingroup cuoo_mod
 **
 ** This header defines a macro \ref CUOO_HCOBJ to put right after the opening
 ** brace of a hash-consed object struct:
 ** \code
 ** struct my_obj {
 **     CUOO_HCOBJ
 **     ... key fields ...
 **     ... value fields if present ...
 ** };
 ** \endcode
 ** Note that there is no semicolon after the macro.  A macro is used here,
 ** because the need for this field depends on the configuration.
 **
 ** \def CUOO_HCOBJ
 ** Put this on the top of struct which define hash-consed objects, leaving out
 ** semicolon.
 **
 ** \def CUOO_HCOBJ_SHIFT
 ** The offset of the key-part of a hash-consed object struct.
 **/

typedef struct cuooP_hcobj *cuooP_hcobj_t;

#ifdef CUCONF_ENABLE_HASHCONS_DISCLAIM_OLD

# define CUOO_HC_GENERATION 0
# define CUOO_HCOBJ_NEEDED 1

# define CUOO_HCOBJ CUOO_OBJ cu_inherit (cuooP_hcobj);
# define CUOO_HCOBJ_SHIFT sizeof(struct cuooP_hcobj)
# define CUOO_HCOBJ_INIT CUOO_OBJ_INIT { 0 },

struct cuooP_hcobj
{
    AO_t hcset_next;
# if CUOO_HC_GENERATION
    AO_t generation;
# endif
};

#else /* !CUCONF_ENABLE_HASHCONS_DISCLAIM_OLD */

# define CUOO_HCOBJ
# define CUOO_HCOBJ_SHIFT 0
# define CUOO_HCOBJ_INIT

#endif /* !CUCONF_ENABLE_HASHCONS_DISCLAIM_OLD */

#define CUOO_HCOBJ_SHIFTW (CUOO_HCOBJ_SHIFT/CU_WORD_SIZE)
#define CUOO_HCOBJ_KEY(obj) ((void *)((char *)(obj) + CUOO_HCOBJ_SHIFT))

/** @} */
CU_END_DECLARATIONS

#endif
