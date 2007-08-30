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

#ifndef CUCON_STR_H
#define CUCON_STR_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <cu/fwd.h>
#include <cu/clos.h>
#include <cuoo/type.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_str cu/str.h: Strings
 * @{ \ingroup cu_mod */

struct cu_str_s
{
    CUOO_OBJ
    char *arr;
    size_t len;
    size_t cap;
};

extern cuoo_stdtype_t cuP_str_type;

/*!The dynamic type of \c cu_str_t. */
CU_SINLINE cuoo_type_t cu_str_type()
{ return cuoo_stdtype_to_type(cuP_str_type); }

/*!True iff \a s is a \c cu_str_t.
 * \pre s is a dynamically typed object. */
CU_SINLINE cu_bool_t cu_is_str(void *s)
{ return cuex_meta(s) == cuoo_type_to_meta(cu_str_type()); }

/*!True iff \a str is empty. */
CU_SINLINE cu_bool_t cu_str_is_empty(cu_str_t str) { return str->len == 0; }
/*!The number of bytes contained in \a str. */
CU_SINLINE size_t cu_str_size(cu_str_t str) { return str->len; }
/*!The byte at position \a i in \a str.
 * \pre 0 ≤ \a i < <tt>cu_str_size(\a str)</tt> */
CU_SINLINE char cu_str_at(cu_str_t str, size_t i) { return str->arr[i]; }

/*!Create an empty string with no capacity. */
cu_str_t cu_str_new(void);
/*!Create a dynamically typed empty string. */
cu_str_t cu_str_onew(void);
/*!Construct an empty string.  Can be used to create a string on the
 * stack, in static memory or to inline a string in another struct. */
void cu_str_cct(cu_str_t dest);

/*!Return a string containing the characters \a cstr to 'cstr + len'. */
cu_str_t cu_str_new_charr(char const *cstr, size_t len);
/*!\copydoc cu_str_new_charr*/
cu_str_t cu_str_onew_charr(char const *cstr, size_t len);
/*!Create a string from the C string (0-terminated) \a cstr. */
cu_str_t cu_str_new_cstr(char const *cstr);
/*!\copydoc cu_str_new_cstr*/
cu_str_t cu_str_onew_cstr(char const *cstr);

/*!Create a string from a statically stored C string. */
cu_str_t cu_str_new_cstr_static(char const *cstr);
/*!\copydoc cu_str_new_cstr_static*/
cu_str_t cu_str_onew_cstr_static(char const *cstr);
/*!\copydoc cu_str_new_cstr_static*/
void cu_str_cct_cstr_static(cu_str_t str, char const *cstr);

/*!Create a string and format the contents as \c sprintf. */
cu_str_t cu_str_new_fmt(char const *fmt, ...);
/*!\copydoc cu_str_new_fmt*/
cu_str_t cu_str_onew_fmt(char const *fmt, ...);
/*!Create a string and format the contents as \c vsprintf. */
cu_str_t cu_str_new_vfmt(char const *fmt, va_list va);
/*!\copydoc cu_str_new_vfmt */
cu_str_t cu_str_onew_vfmt(char const *fmt, va_list va);

/*!Create a string of \a n uninitialised characters. */
void cu_str_cct_uninit(cu_str_t str, size_t n);
/*!\copydoc cu_str_cct_uninit*/
cu_str_t cu_str_new_uninit(size_t n);
/*!\copydoc cu_str_cct_uninit*/
cu_str_t cu_str_onew_uninit(size_t n);

/*!Return a copy of \a str. */
cu_str_t cu_str_new_copy(cu_str_t str);
/*!Return a dynamically typed copy of \a str. */
cu_str_t cu_str_onew_copy(cu_str_t str);

/*!Assing \a src to \a dest.  The capacity remains in \a src. */
void cu_str_assign(cu_str_t dest, cu_str_t src);
#define cu_str_cct_copy cu_str_assign

/*!Return a new string which is the concatenation of \a x and \a y.  The
 * returned string will use the capacity of \a x if sufficient. */
cu_str_t cu_str_new_2str(cu_str_t x, cu_str_t y);
/*!\copydoc cu_str_new_2str*/
cu_str_t cu_str_onew_2str(cu_str_t x, cu_str_t y);
/*!\copydoc cu_str_new_2str*/
cu_str_t cu_str_new_str_char(cu_str_t x, char y);
/*!\copydoc cu_str_new_2str*/
cu_str_t cu_str_onew_str_char(cu_str_t x, char y);
/*!\copydoc cu_str_new_2str*/
cu_str_t cu_str_new_str_cstr(cu_str_t x, char const *y);
/*!\copydoc cu_str_new_2str*/
cu_str_t cu_str_onew_str_cstr(cu_str_t x, char const *y);
/*!\copydoc cu_str_new_2str*/
cu_str_t cu_str_new_cstr_str(char const *x, cu_str_t y);
/*!\copydoc cu_str_new_2str*/
cu_str_t cu_str_onew_cstr_str(char const *x, cu_str_t y);
/*!\copydoc cu_str_new_2str*/
cu_str_t cu_str_new_2cstr(char const *x, char const *y);
/*!\copydoc cu_str_new_2str*/
cu_str_t cu_str_onew_2cstr(char const *x, char const *y);

/*!Return a string which is the concatenation of the substrings from
 * \a s0 to \a s0 + \a n0 and from \a s1 to \a s1 + \a n1. */
cu_str_t cu_str_new_2charr(char const *s0, size_t n0,
				 char const *s1, size_t n1);
/*!\copydoc cu_str_new_2charr*/
cu_str_t cu_str_onew_2charr(char const *s0, size_t n0,
				  char const *s1, size_t n1);
/*!Return a string which is the concatenation of the substrings from
 * \a s0 to \a s0 + \a n0, from \a s1 to \a s1 + \a n1, and from
 * \a s2 to \a s2 + \a n2. */
cu_str_t cu_str_new_3charr(char const *s0, size_t n0,
				 char const *s1, size_t n1,
				 char const *s2, size_t n2);
/*!\copydoc cu_str_new_3charr*/
cu_str_t cu_str_onew_3charr(char const *s0, size_t n0,
				  char const *s1, size_t n1,
				  char const *s2, size_t n2);
/*!Convert \a idr to a string. */
cu_str_t cu_str_new_idr(cu_idr_t idr);
/*!\copydoc cu_str_new_idr */
cu_str_t cu_str_onew_idr(cu_idr_t idr);

/*!Assign to \a dest the substring of \a dest from \a pos to 'pos + len'. */
void cu_str_assign_substr(cu_str_t dest, size_t pos, size_t len);

/*!Return a new string which is the substring of \a x from \a pos to
 * \a pos + \a len. */
cu_str_t cu_str_substr(cu_str_t x, size_t pos, size_t len);

/*!Return <tt>cu_str_substr(x, pos, cu_str_size(x) - pos)</tt>. */
cu_str_t cu_str_suffix(cu_str_t x, size_t pos);

/*!Assign to \a dest the concatenation of itself with \a src. */
void cu_str_append_str(cu_str_t dest, cu_str_t src);

/*!Append a character to \a dest. */
void cu_str_append_char(cu_str_t dest, char ch);

/*!Append to \a dest the characters from \a s to \a s + \a n. */
void cu_str_append_charr(cu_str_t dest, char const *s, size_t n);
/*!Append the C string \a cstr to \a dest. */
void cu_str_append_cstr(cu_str_t dest, char const *cstr);

/*!Append a formatted string to \a dst. */
void cu_str_append_vfmt(cu_str_t dst, char const *fmt, va_list va);
/*!Append a formatted string to \a dst. */
void cu_str_append_fmt(cu_str_t dst, char const *fmt, ...);

/*!Drop \a char_count characters from the end of \a str. */
void cu_str_unappend(cu_str_t str, size_t char_count);

/*!Insert \a src into \a dst at 'pos. */
void cu_str_insert_substr(cu_str_t dst, cu_str_t src, size_t pos);

/*!Erase chars \a pos to \a pos + \a len from \a dst. */
void cu_str_erase_substr(cu_str_t dst, size_t pos, size_t len);

/*!Return a null-terminated C string equal to \a x. */
char const *cu_str_to_cstr(cu_str_t x);

/*!Convert to an identifier. */
cu_idr_t cu_str_to_idr(cu_str_t str);

/*!Return the possibly not null-terminated array of character data of
 * \a str. */
char const *cu_str_charr(cu_str_t str);
#define cu_str_charr(str) (CU_MARG(cu_str_t, (str))->arr)

/*!True iff \a x and \a y are equal. */
cu_bool_t cu_str_eq(cu_str_t x, cu_str_t y);
/*!A hash of \a x. */
cu_hash_t cu_str_hash(cu_str_t x);

/*!Return a zero if \a x and \a y are equal, otherwise return a negative
 * or a positive number if \a x is, respectively, less than or greater
 * than \a y according to some arbitrary ordering over strings.  Use
 * \ref cu_str_coll for a specified ordering. */
int cu_str_cmp(cu_str_t x, cu_str_t y);
/*!As \ref cu_str_cmp, except with a character array RHS. */
int cu_str_cmp_charr(cu_str_t x, char const *y_arr, size_t y_size);
/*!As \ref cu_str_cmp, except with a C string RHS. */
int cu_str_cmp_cstr(cu_str_t x, char const *y);

/*!True iff \a x has a substring \a y starting at \a pos. */
cu_bool_t cu_str_subeq(cu_str_t x, size_t pos, cu_str_t y);
/*!\copydoc cu_str_subeq */
cu_bool_t cu_str_subeq_cstr(cu_str_t x, size_t pos, char const *y);
/*!True iff \a x has a substring starting at \a pos which matches \a y_size
 * characters from \a y_arr. */
cu_bool_t cu_str_subeq_charr(cu_str_t x, size_t pos,
			     char const *y_arr, size_t y_size);

/*!Return the result of \a strcoll on the C representations of \a x and
 * \a y.  XX This only works if LC_CTYPE refers to an UTF-8 locale. */
int cu_str_coll(cu_str_t x, cu_str_t y);
/*!As \ref cu_str_coll, except with a character array RHS. */
int cu_str_coll_charr(cu_str_t x, char const *y_arr, size_t y_size);
/*!As \ref cu_str_coll, except with a C string RHS. */
int cu_str_coll_cstr(cu_str_t x, char const *y);

#define cu_str_index_none ((size_t)-1)
/*!Index of the first occurrence of \a ch in \a str or
 * \c cu_str_index_none if not found. */
size_t cu_str_chr(cu_str_t str, char ch);
/*!Index of the last occurrence of \a ch in \a str or
 * \c cu_str_index_none if not found. */
size_t cu_str_rchr(cu_str_t str, char ch);
/*!Index of the first occurrence of \a ch in \a str in the range
 * \a pos to \a pos + \a len, or \c cu_str_index_none if none. */
size_t cu_str_chr_in_substr(cu_str_t str, size_t pos, size_t len, char ch);
/*!Index of the last occurrence of \a ch in \a str in the range
 * \a pos to \a pos + \a len, or \c cu_str_index_none if none. */
size_t cu_str_rchr_in_substr(cu_str_t str, size_t pos, size_t len, char ch);

/*!Chop spaces from beginning and end of \a str.  Currently only ASCII
 * spaces are stripped. */
void cu_str_chop_space(cu_str_t str);

/*!Searches for the next white-space separated component of \a str, starting
 * at \c *\a pos_io + \c *\a len_io.  If there are only whitespace characters
 * following this start position, returns false, else sets \c *\a pos_io to
 * point to the first non-whitespace character, and \c *\a len_io to the
 * number on non-whitespace characters, and returns true.  Typical usage:
 * \code
 * size_t pos = 0;
 * size_t len = 0;
 * while (cu_str_next_space_delimited(str, &pos, &len))
 *     some_code(cu_str_substr(str, pos, len));
 * \endcode
 */
cu_bool_t cu_str_next_space_delimited(cu_str_t str,
				      size_t *pos_io, size_t *len_io);

/*!Return a quoted representation of \a str.  This is a limited implementation
 * as it only handles a few ASCII control characters.  It uses the unual
 * C format. */
cu_str_t cu_str_quote(cu_str_t str);

extern cu_clop(cu_str_eq_clop,   cu_bool_t, cu_str_t, cu_str_t);
extern cu_clop(cu_str_hash_clop, cu_hash_t, cu_str_t);
extern cu_clop(cu_str_cmp_clop,        int, cu_str_t, cu_str_t);
extern cu_clop(cu_str_coll_clop,       int, cu_str_t, cu_str_t);

/* obsolete These are useful when creating sets and maps of string keys.
cu_bool_t cu_nonclos(cu_str_eq_nonclos)(void *, void *);
cu_hash_t cu_nonclos(cu_str_hash_nonclos)(void *);
int cu_nonclos(cu_str_cmp_nonclos)(void *, void *);
int cu_nonclos(cu_str_coll_nonclos)(void *, void *); */

/*!@}*/
CU_END_DECLARATIONS
#endif
