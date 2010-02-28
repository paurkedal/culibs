/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#include <cutext/encoding.h>
#include <cu/debug.h>

/* Defined in generated file. */
char const *cutextP_encoding_name(cutext_encoding_t chenc);

char const *
cutext_encoding_name(cutext_encoding_t chenc)
{
    char const *name = cutextP_encoding_name(chenc);
    cu_debug_assert(name);
    return name;
}

cutext_encoding_t
cutext_guess_encoding(void const *s_, size_t n)
{
    unsigned char const *s = s_;

    if (n == 0)
        return CUTEXT_ENCODING_NONE;
    else if (n == 1 || n == 3)
        return CUTEXT_ENCODING_UTF8;
    else if (n == 2) {
        if ((s[0] == 0xfe && s[1] == 0xff) || s[0] == 0)
            return CUTEXT_ENCODING_UTF16;
        if ((s[0] == 0xff && s[1] == 0xfe) || s[1] == 0)
            return CUTEXT_ENCODING_UTF16LE;
        return CUTEXT_ENCODING_UTF8;
    }

    /* Detect by byte order mark 0x0000feff. */

    /* UTF-8 */
    if (s[0] == 0xef && s[1] == 0xbb && s[2] == 0xbf)
        return CUTEXT_ENCODING_UTF8;
    /* Network byte order */
    if (s[0] == 0xfe && s[1] == 0xff)
        return CUTEXT_ENCODING_UTF16;
    if (s[0] == 0x00 && s[1] == 0x00) {
        if (s[2] == 0xfe && s[3] == 0xff)
            return CUTEXT_ENCODING_UCS4;
    }
    /* Little endian */
    if (s[0] == 0xff && s[1] == 0xfe) {
        if (s[2] == 0x00 && s[3] == 0x00)
            return CUTEXT_ENCODING_UCS4LE;
        return CUTEXT_ENCODING_UTF16LE;
    }

    /* Detect by assuming the first character is ASCII */

    /* Network byte order */
    if (s[0] == 0) { /* UTF-16, UCS-2, UCS-4 */
        if (s[1] == 0)
            return CUTEXT_ENCODING_UCS4;
        return CUTEXT_ENCODING_UTF16;
    }
    /* Little endian */
    if (s[2] == 0 && s[3] == 0)
        return CUTEXT_ENCODING_UCS4LE;
    if (s[1] == 0)
        return CUTEXT_ENCODING_UTF16LE;
    if (s[2] != 0)
        return CUTEXT_ENCODING_UTF8;
    return CUTEXT_ENCODING_UNKNOWN;
}
