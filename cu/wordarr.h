/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_WORDARR_H
#define CU_WORDARR_H

#include <cu/fwd.h>
#include <cu/halloc.h>
#include <cu/hash.h>

CU_BEGIN_DECLARATIONS

CU_SINLINE cu_bool_t
cu_wordarr_eq(size_t cnt, cu_word_t *arr0, cu_word_t *arr1)
{
    while (cnt--)
	if (*arr0++ != *arr1++)
	    return cu_false;
    return cu_true;
}

CU_SINLINE void
cu_wordarr_copy(size_t cnt, cu_word_t *dst, cu_word_t *src)
{
    while (cnt--)
	*dst++ = *src++;
}

#define cu_wordarr_hash cu_wordarr_hash_bj

CU_END_DECLARATIONS

#endif
