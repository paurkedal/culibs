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

#ifndef CU_INIT_H
#define CU_INIT_H

#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_init_h cu/init.h: Library Initialisation
 * @{\ingroup cu_mod */

extern cu_bool_t cuP_locale_is_utf8;

/*!Announces to culibs that the current application is \a name. */
void cu_set_application_name(char const *name);

/*!Returns a previously set application name.  \c cu_set_application_name must
 * have been set prior to calling this function, or a logic error is
 * raised. */
char const *cu_application_name_cstr(void);

/*!\copydoc cu_application_name_cstr */
cu_str_t cu_application_name_str(void);

/*!@}*/
CU_END_DECLARATIONS

#endif
