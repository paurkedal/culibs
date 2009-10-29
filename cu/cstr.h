/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_CSTR_H
#define CU_CSTR_H

#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS

char const *cu_struniq(char const *str);
char const *cu_struniq_int(int);

cu_bool_t cu_cstr_eq(char const *, char const *);
cu_hash_t cu_cstr_hash(char const *);
cu_hash_t cu_cstr_hash_x(char const *, cu_hash_t);

extern cu_clop(cu_cstr_eq_clop, cu_bool_t, char *, char *);
extern cu_clop(cu_cstr_hash_clop, cu_hash_t, char *);

char *cu_cstr_new_copy(char const *cstr);
char *cu_cstr_toupper(char *cstr);
char *cu_cstr_tolower(char *cstr);

char *cu_wcstombs(wchar_t const *wcs) CU_ATTR_DEPRECATED;
wchar_t *cu_mbstowcs(char const *str) CU_ATTR_DEPRECATED;

CU_END_DECLARATIONS

#endif
