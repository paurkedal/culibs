/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

CU_BEGIN_DECLARATIONS

typedef enum {
    cutext_chenc_none,
    cutext_chenc_unknown,
    cutext_chenc_utf8,
    cutext_chenc_utf16,
    cutext_chenc_utf16le,
    cutext_chenc_ucs2,
    cutext_chenc_ucs2le,
    cutext_chenc_ucs4,
    cutext_chenc_ucs4le,
#ifdef CUCONF_WORDS_BIGENDIAN
    cutext_chenc_utf16host = cutext_chenc_utf16,
    cutext_chenc_ucs2host = cutext_chenc_ucs2,
    cutext_chenc_ucs4host = cutext_chenc_ucs4,
#else
    cutext_chenc_utf16host = cutext_chenc_utf16le,
    cutext_chenc_ucs2host = cutext_chenc_ucs2le,
    cutext_chenc_ucs4host = cutext_chenc_ucs4le,
#endif
} cutext_chenc_t;

typedef struct cutext_buffer_s *cutext_buffer_t;
typedef struct cutext_src_s *cutext_src_t;
typedef struct cutext_ucs4src_s *cutext_ucs4src_t;

CU_END_DECLARATIONS

#endif
