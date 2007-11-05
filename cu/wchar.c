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

#ifdef CU_WCHAR_IS_STDC
#  define WCHAR_ENCODING "WCHAR_T"
#elif defined(CUCONF_WORDS_BIGENDIAN)
#  define WCHAR_ENCODING "UTF-32"
#else
#  define WCHAR_ENCODING "UTF-32LE"
#endif

iconv_t
cu_iconv_for_wchar_to_char(void)
{
    cuP_tstate_t tls = cuP_tstate();
    if (!tls->iconv_ucs4_to_utf8)
	tls->iconv_ucs4_to_utf8 = iconv_open("UTF-8", WCHAR_ENCODING);
    iconv(tls->iconv_ucs4_to_utf8, NULL, NULL, NULL, NULL);
    return tls->iconv_ucs4_to_utf8;
}

iconv_t
cu_iconv_for_char_to_wchar(void)
{
    cuP_tstate_t tls = cuP_tstate();
    if (!tls->iconv_utf8_to_ucs4)
	tls->iconv_utf8_to_ucs4 = iconv_open(WCHAR_ENCODING, "UTF-8");
    iconv(tls->iconv_utf8_to_ucs4, NULL, NULL, NULL, NULL);
    return tls->iconv_utf8_to_ucs4;
}
