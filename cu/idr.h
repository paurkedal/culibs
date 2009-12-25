/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_IDR_H
#define CU_IDR_H

#include <cuoo/type.h>
#include <cu/wchar.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_idr_h cu/idr.h: Identifiers (Strings with Pointer Equality)
 * @{ \ingroup cu_type_mod */

struct cu_idr
{
    CUOO_HCOBJ
    cu_word_t key_size;
};

extern cuoo_type_t cuP_idr_type;

/*!Returns the type descriptor of all \c cu_idr_t objects. */
CU_SINLINE cuoo_type_t cu_idr_type()
{ return cuP_idr_type; }

/*!True iff the dynamically typed object at \a dyn is an \c cu_idr_t. */
CU_SINLINE cu_bool_t cu_is_idr(void *dyn)
{ return cuex_meta(dyn) == cuoo_type_to_meta(cu_idr_type()); }

/*!Returns the identifier with the name \a cstr.  The result is hashconsed,
 * meaning that two simultaneously live \c cu_idr_t objects constructed
 * from the same string (according to strcmp) have identical pointers. */
cu_idr_t cu_idr_by_cstr(char const *cstr);

/*!Returns the identifier with the name given by \a arr to \a arr +
 * \a size.  See \ref cu_idr_by_cstr. */
cu_idr_t cu_idr_by_charr(char const *arr, size_t size);

/*!Returns the identifier with the name given by \a arr to \a arr + \a len.
 * The Unicode string is converted to UTF-8. */
cu_idr_t cu_idr_by_wchararr(cu_wchar_t const *arr, size_t len);

/*!Returns the identifier of name \a s.  The Unicode string is converted to
 * UTF-8. */
cu_idr_t cu_idr_by_wstring(cu_wstring_t s);

/*!Return a C string strcmp-equal to the one passed upon costruction. */
CU_SINLINE char const *cu_idr_to_cstr(cu_idr_t idr)
{ return (char const *)(idr + 1); }

/*!A simple pointer comparison. */
CU_SINLINE cu_bool_t cu_idr_eq(cu_idr_t idr0, cu_idr_t idr1)
{ return idr0 == idr1; }

/*!Returns <tt>strcmp(cu_idr_to_cstr(\a idr0), cu_idr_to_cstr(\a idr1))</tt>.*/
int cu_idr_strcmp(cu_idr_t idr0, cu_idr_t idr1);

#define cu_idr_of_cstrq cu_idr_of_cstr
#define cu_idr_to_cstrq cu_idr_to_cstr

#ifndef CU_IN_DOXYGEN
extern cu_clop(cu_idr_strcmp_clop, int, cu_idr_t, cu_idr_t);
#endif

/*!@}*/
CU_END_DECLARATIONS

#endif
