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

#ifndef CU_WCHAR_H
#define CU_WCHAR_H

#include <cu/fwd.h>
#include <iconv.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_wchar_h cu/wchar.h: Unicode Characters
 *@{\ingroup cu_mod */

/*!A thread-local \c iconv_t descriptor for converting from \ref cu_wchar_t
 * strings to \c char strings. */
iconv_t cu_iconv_for_wchar_to_char(void);

/*!A thread-local \c iconv_t descriptor for converting from \c char strings to
 * \ref cu_wchar_t strings. */
iconv_t cu_iconv_for_char_to_wchar(void);

/*!@}*/
CU_END_DECLARATIONS

#endif
