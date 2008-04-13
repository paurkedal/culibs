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

#include <cu/wchar.h>
#include <cu/tstate.h>
#include <string.h>

#ifdef CU_WCHAR_IS_STDC
char const *cu_wchar_encoding = "WCHAR_T";
#elif defined(CUCONF_WORDS_BIGENDIAN)
#  define CU_WCHAR_IS_UTF32BE_COMPAT 1
char const *cu_wchar_encoding = "UTF-32BE";
#else
#  define CU_WCHAR_IS_UTF32LE_COMPAT 1
char const *cu_wchar_encoding = "UTF-32LE";
#endif
/* TODO: Check and define CU_WCHAR_IS_UTF32_COMPAT if appropriate. */

iconv_t
cu_iconv_for_wchar_to_char(void)
{
    cuP_tstate_t tls = cuP_tstate();
    if (!tls->iconv_ucs4_to_utf8)
	tls->iconv_ucs4_to_utf8 = iconv_open("UTF-8", cu_wchar_encoding);
    iconv(tls->iconv_ucs4_to_utf8, NULL, NULL, NULL, NULL);
    return tls->iconv_ucs4_to_utf8;
}

iconv_t
cu_iconv_for_char_to_wchar(void)
{
    cuP_tstate_t tls = cuP_tstate();
    if (!tls->iconv_utf8_to_ucs4)
	tls->iconv_utf8_to_ucs4 = iconv_open(cu_wchar_encoding, "UTF-8");
    iconv(tls->iconv_utf8_to_ucs4, NULL, NULL, NULL, NULL);
    return tls->iconv_utf8_to_ucs4;
}

cu_bool_t
cu_encoding_is_wchar_compat(char const *enc)
{
    switch (*enc++) {
#if CU_WCHAR_IS_STDC
	case 'w': case 'W':
	    return strcasecmp(enc, "char_t") == 0;
#endif
#if CU_WCHAR_IS_UTF32BE_COMPAT || CU_WCHAR_IS_UTF32LE_COMPAT
	case 'u': case 'U':
	    switch (*enc++) {
		case 't': case 'T':
		    if (*enc != 'f' && *enc != 'F') return cu_false;
		    ++enc;
		    if (*enc == '-') ++enc;
		    if (*enc++ != '3') return cu_false;
		    if (*enc++ != '2') return cu_false;
		    goto check_endian;
		case 'c': case 'C':
		    if (*enc != 's' && *enc != 'S') return cu_false;
		    ++enc;
		    if (*enc == '-') ++enc;
		    if (*enc++ != '4') return cu_false;
		    goto check_endian;
		default:
		    return cu_false;
	    }
#endif
	default:
	    return cu_false;
    }

#if CU_WCHAR_IS_UTF32BE_COMPAT
check_endian:
# if CU_WCHAR_IS_UTF32_COMPAT
    if (!*enc) return cu_true;
# endif
    if (*enc != 'b' || *enc != 'B') return cu_false;
    ++enc;
    if (*enc != 'e' || *enc != 'E') return cu_false;
    ++enc;
    return !*enc;
#endif

#if CU_WCHAR_IS_UTF32LE_COMPAT
check_endian:
# if CU_WCHAR_IS_UTF32_COMPAT
    if (!*enc) return cu_true;
# endif
    if (*enc != 'l' || *enc != 'L') return cu_false;
    ++enc;
    if (*enc != 'e' || *enc != 'E') return cu_false;
    ++enc;
    return !*enc;
#endif
}
