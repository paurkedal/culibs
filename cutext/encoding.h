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

#ifndef CUTEXT_ENCODING_H
#define CUTEXT_ENCODING_H

#include <cutext/fwd.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cutext_encoding_h cutext/encoding.h: Unicode Encodings
 ** @{ \ingroup cutext_mod */

/** Enumeration of Unicode encodings. */
typedef enum {
    CUTEXT_ENCODING_NONE,
    CUTEXT_ENCODING_UNKNOWN,
    CUTEXT_ENCODING_UTF8,
    CUTEXT_ENCODING_UTF16,
    CUTEXT_ENCODING_UTF16LE,
    CUTEXT_ENCODING_UCS2,
    CUTEXT_ENCODING_UCS2LE,
    CUTEXT_ENCODING_UCS4,
    CUTEXT_ENCODING_UCS4LE,
#ifdef CUCONF_WORDS_BIGENDIAN
    CUTEXT_ENCODING_UTF16HOST = CUTEXT_ENCODING_UTF16,
    CUTEXT_ENCODING_UCS2HOST = CUTEXT_ENCODING_UCS2,
    CUTEXT_ENCODING_UCS4HOST = CUTEXT_ENCODING_UCS4,
#else
    CUTEXT_ENCODING_UTF16HOST = CUTEXT_ENCODING_UTF16LE,
    CUTEXT_ENCODING_UCS2HOST = CUTEXT_ENCODING_UCS2LE,
    CUTEXT_ENCODING_UCS4HOST = CUTEXT_ENCODING_UCS4LE,
#endif
} cutext_encoding_t;

/** Return the name \a enc, in a form suitable for iconv. */
char const *cutext_encoding_name(cutext_encoding_t enc);

/** Return the encoding of name \a enc_name, or \c CUTEXT_ENCODING_NONE if \a
 ** enc_name does not name a Unicode coding. */
cutext_encoding_t cutext_encoding_by_name(char const *enc_name);

/** Guess the encoding of the data staring at \a data using up to \a size
 ** bytes.  Try to pass 4 bytes if available.   */
cutext_encoding_t cutext_guess_encoding(void const *data, size_t size);

/** @} */
CU_END_DECLARATIONS

#endif
