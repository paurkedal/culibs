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

#ifndef CUOO_FWD_H
#define CUOO_FWD_H

#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuoo_fwd_h cuoo/fwd.h: Forward Declarations
 *@{\ingroup cuoo_mod */

typedef uintptr_t cuex_meta_t;
#define CUEX_META_C(c) CU_UINTPTR_C(c)
#define CUEX_META_SIZE CUCONF_SIZEOF_INTPTR_T

/* The CUCONF_META_IN_OBJECT_STRUCT is not implemented, and may be dropped,
 * but it is still informative to prefix structs with dynamic-typed
 * constructors with CUOO_OBJ. */
#ifdef CUCONF_META_IN_OBJECT_STRUCT
#  define CUOO_OBJ cuex_meta_t cuex_meta_field;
#  define CUOO_OBJ_SHIFT sizeof(cuex_meta_t)
#  define CUOO_OBJ_NEEDED 1
#  define CUOO_OBJ_INIT 0
#else
#  define CUOO_OBJ
#  define CUOO_OBJ_SHIFT 0
#  define CUOO_OBJ_INIT
#endif

typedef void			*cuex_t;
typedef struct cuoo_layout_s	*cuoo_layout_t;		/* layout.h */
typedef struct cuoo_prop_s	*cuoo_prop_t;		/* prop.h */
typedef cu_word_t (*cuoo_impl_t)(cu_word_t, ...);	/* type.h */
typedef unsigned int		cuoo_propkey_t;		/* type.h */
typedef struct cuoo_stdtype_s	*cuoo_stdtype_t;	/* type.h */
typedef struct cuoo_type_s	*cuoo_type_t;		/* type.h */

void cuoo_init(void);

/*!@}*/
CU_END_DECLARATIONS

#endif
