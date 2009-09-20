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

#ifndef CU_VA_REF_H
#define CU_VA_REF_H

#include <cu/fwd.h>
#include <cu/conf.h>
#include <stdarg.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cu_va_ref_h cu/va_ref.h: Passing \c va_list as a Reference
 ** @{ \ingroup cu_util_mod */

#ifdef CUCONF_VA_LIST_IS_REF

typedef va_list cu_va_ref_t;
#define cu_va_ref_of_va_list(va) (va)
#define cu_va_ref_to_va_list(va) (va)
#define cu_va_ref_arg(va, t) va_arg(va, t)

#else

typedef va_list *cu_va_ref_t;
#define cu_va_ref_of_va_list(va) (&(va))
#define cu_va_ref_to_va_list(va) (*(va))
#define cu_va_ref_arg(va, t) va_arg(*(va), t)

#endif

/** @} */
CU_END_DECLARATIONS

#endif
