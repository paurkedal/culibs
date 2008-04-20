/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUTEXT_WCTYPE_H
#define CUTEXT_WCTYPE_H

#include <cu/wchar.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cutext_wctype_h cutext/wctype.h: Unicode Character Types
 *@{\ingroup cutext_mod
 *
 * \note
 * These character types are derived from the Unicode general categories, and
 * thus only an approximation to the locale-specific C99 classification.
 *
 * \note
 * The mapping of these functions for non-ASCII characters are subject to
 * revision.
 */

cu_bool_t cutext_iswalnum(cu_wint_t wc);
cu_bool_t cutext_iswalpha(cu_wint_t wc);
cu_bool_t cutext_iswblank(cu_wint_t wc);
cu_bool_t cutext_iswcntrl(cu_wint_t wc);

CU_SINLINE cu_bool_t
cutext_iswdigit(cu_wint_t wc)
{ return 0x30 <= wc && wc <= 0x39; }

cu_bool_t cutext_iswgraph(cu_wint_t wc);
cu_bool_t cutext_iswlower(cu_wint_t wc);
cu_bool_t cutext_iswprint(cu_wint_t wc);
cu_bool_t cutext_iswpunct(cu_wint_t wc);
cu_bool_t cutext_iswspace(cu_wint_t wc);
cu_bool_t cutext_iswupper(cu_wint_t wc);
cu_bool_t cutext_iswxdigit(cu_wint_t wc);

/*!@}*/
CU_END_DECLARATIONS

#endif
