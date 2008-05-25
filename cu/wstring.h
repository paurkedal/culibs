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

#ifndef CU_WSTRING_H
#define CU_WSTRING_H

#include <cu/fwd.h>
#include <cu/wchar.h>
#include <cuoo/type.h>

CU_BEGIN_DECLARATIONS

extern cuoo_type_t cuP_wstring_type;
extern cu_wstring_t cuP_wstring_empty;

/*!\defgroup cu_wstring_h cu/wstring.h: Unicode String
 *@{\ingroup cu_type_mod */

/*!A string of \ref cu_wchar_t characters. */
struct cu_wstring_s
{
    CUOO_OBJ
    size_t len;
    cu_wchar_t const *arr;
};

#ifdef CU_WCHAR_IS_STDC
#  define CU_WSTRING_C(s) \
	  cu_wstring_of_arr(L##s, sizeof(L##s)/sizeof(cu_wchar_t) - 1)
#else
#  define CU_WSTRING_C(s) cu_wstring_of_chararr(s, sizeof(s) - 1)
#endif

/*!Dynamic type of a \a cu_wstring_t. Only strings returned by functions ending
 * in \c _o are dynamically typed. */
CU_SINLINE cuoo_type_t cu_wstring_type()
{ return cuP_wstring_type; }

/*!The length of \a s. */
CU_SINLINE size_t cu_wstring_length(cu_wstring_t s) { return s->len; }

/*!The underlying array of \a s. This is not null-terminated. */
CU_SINLINE cu_wchar_t const *cu_wstring_array(cu_wstring_t s)
{ return s->arr; }

/*!Returns the character at position \a i. */
CU_SINLINE cu_wchar_t cu_wstring_at(cu_wstring_t s, size_t i)
{ cu_debug_assert(i < s->len); return s->arr[i]; }

/*!Returns an empty string. */
CU_SINLINE cu_wstring_t cu_wstring_empty() { return cuP_wstring_empty; }

/*!Returns a string with a copy of \a len characters from \a arr. */
cu_wstring_t cu_wstring_of_arr(cu_wchar_t const *arr, size_t len);

/*!A variant of \ref cu_wstring_of_arr which returns a dynamically typed
 * object. */
cu_wstring_t cu_wstring_of_arr_o(cu_wchar_t const *arr, size_t len);

/*!Returns a string with a reference to the first \a len characters of
 * \a arr. */
cu_wstring_t cu_wstring_of_arr_ref(cu_wchar_t const *arr, size_t len);

/*!A variant of \ref cu_wstring_of_arr_ref which returns a dynamically typed
 * object.  */
cu_wstring_t cu_wstring_of_arr_ref_o(cu_wchar_t const *arr, size_t len);

cu_wstring_t cu_wstring_of_chararr(char const *arr, size_t len);

cu_wstring_t cu_wstring_of_charstr(char const *str);

/*!The concatenation of \a sl and \a sr. */
cu_wstring_t cu_wstring_cat2(cu_wstring_t sl, cu_wstring_t sr);

/*!A variant of \ref cu_wstring_cat2 which returns a dynamically typed
 * object. */
cu_wstring_t cu_wstring_cat2_o(cu_wstring_t sl, cu_wstring_t sr);

cu_wstring_t cu_wstring_cat3(cu_wstring_t s0, cu_wstring_t s1,
			     cu_wstring_t s2);

cu_wstring_t cu_wstring_cat3_o(cu_wstring_t s0, cu_wstring_t s1,
			       cu_wstring_t s2);

/*!The slice from character \a i to (but not including) character \a j. */
cu_wstring_t cu_wstring_slice(cu_wstring_t s, size_t i, size_t j);

/*!A variant of \ref cu_wstring_slice which returns a dynamically typed
 * object. */
cu_wstring_t cu_wstring_slice_o(cu_wstring_t s, size_t i, size_t j);

/*!Returns negative, zero, or positive where \a sl is less than, equal to, or
 * greater than \a sr, respectively, according to some fixed arbitrary
 * ordering. */
int cu_wstring_cmp(cu_wstring_t sl, cu_wstring_t sr);

#if 0
/*!Returns negative, zero, or positive where \a sl is less than, equal to, or
 * greater than \a sr, respectively.  If the compiler defines \c
 * __STDC_ISO_10646__, then \c wcsncmp is used, which gives the correct order
 * according to the current locale.  Otherwise, the order is based on the
 * Unicode position of the characters. */
int cu_wstring_coll(cu_wstring_t sl, cu_wstring_t sr);
#endif

/*!The image of \a s under \a f. */
cu_wstring_t cu_wstring_image(cu_wstring_t s,
			      cu_clop(f, cu_wchar_t, cu_wchar_t));

/*!A variant of \ref cu_wstring_image which returns a dynamically typed
 * object. */
cu_wstring_t cu_wstring_image_o(cu_wstring_t s,
				cu_clop(f, cu_wchar_t, cu_wchar_t));

/*!True iff \a f maps each character of \a s to true.  For procedural usage, \a
 * f is called in order from first to last character, stopping on the first
 * false return. */
cu_bool_t cu_wstring_forall(cu_wstring_t s, cu_clop(f, cu_bool_t, cu_wchar_t));

/*!@}*/
CU_END_DECLARATIONS

#endif
