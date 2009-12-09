/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007--2009  Petter Urkedal <urkedal@nbi.dk>
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

#include <cu/box.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cuoo_fwd_h cuoo/fwd.h: Forward Declarations
 ** @{ \ingroup cuoo_mod */

/** An expression tree-code, representing either an operator, a type, or a
 ** special code used for variables. */
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

/** An integer which indicates the type and representation of a dynamic
 ** object. */
typedef uint_fast16_t		cuoo_shape_t;

/** The type of expression trees which also works as a generic type of
 ** dynamically typed objects. */
typedef void			*cuex_t;

typedef struct cuoo_layout	*cuoo_layout_t;		/* layout.h */
typedef struct cuoo_prop	*cuoo_prop_t;		/* prop.h */
typedef cu_box_t (*cuoo_impl_t)(cu_word_t, ...);	/* type.h */
typedef unsigned int		cuoo_propkey_t;		/* type.h */
typedef struct cuoo_type	*cuoo_type_t;		/* type.h */

/** Call this to initialise the \ref cuoo_mod "cuoo" modules before use. */
CU_SINLINE void cuoo_init(void) { cu_init(); }

/** @} */
CU_END_DECLARATIONS

#ifndef CU_NCOMPAT
#  include <cuoo/compat.h>
#endif
#endif
