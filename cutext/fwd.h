/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#ifndef CUTEXT_FWD_H
#define CUTEXT_FWD_H

#include <cu/fwd.h>
#include <cu/conf.h>

CU_BEGIN_DECLARATIONS

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

typedef struct cutext_sink *cutext_sink_t;
typedef struct cutext_countsink *cutext_countsink_t;
typedef struct cutext_buffersink *cutext_buffersink_t;

typedef struct cutext_source *cutext_source_t;
typedef struct cutext_source_descriptor *cutext_source_descriptor_t;

typedef struct cutext_src *cutext_src_t;
typedef struct cutext_ucs4src *cutext_ucs4src_t;

typedef enum {
    cutext_status_ok,
    cutext_status_buffer_too_small,
    cutext_status_eos,
    cutext_status_error,
} cutext_status_t;

/** Initialises the cutext and dependent modules. */
void cutext_init(void);

CU_END_DECLARATIONS

#ifndef CU_NCOMPAT
#  include <cutext/compat.h>
#endif
#endif
