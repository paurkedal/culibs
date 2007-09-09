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

#ifndef CUOO_INTF_H
#define CUOO_INTF_H

#include <cuoo/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuoo_intf_h cuoo/intf.h: Interface Numbers
 *@{\ingroup cuoo_mod */

/*!Value returned by interface dispatch functions to indicate that a requested
 * interface has no implementation. */
#define CUOO_IMPL_NONE		CU_WORD_C(0)

#define CUOO_INTF_COMPOUND	0x10	/* cuoo_compound_t */
#define CUOO_INTF_PRINT_FN	0x11	/* void (*)(void *, FILE *) */
#define CUOO_INTF_SERIALISABLE	0x12	/* cuoo_serialisable_t */
#define CUOO_INTF_NEXTLIB	0x100

/*!An interface dispatch which conistently returns \ref CUOO_IMPL_NONE. */
cu_word_t cuoo_impl_none(cu_word_t intf_number, ...);

/* US Spelling */
#define CUOO_INTF_SERIALIZABLE CUOO_INTF_SERIALISABLE

/*!@}*/
CU_END_DECLARATIONS

#endif
