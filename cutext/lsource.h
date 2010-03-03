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

#ifndef CUTEXT_LSOURCE_H
#define CUTEXT_LSOURCE_H

#include <cutext/bufsource.h>
#include <cu/location.h>
#include <cu/wchar.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cutext_lsource_h cutext/lsource.h: Lexical Source
 ** @{ \ingroup cutext_mod
 **
 ** This provides a wide character source with lookahead and tracking of source
 ** location and indentation.  As the name suggests, it's indented to provide
 ** input for lexical analyzers, but may be suitable for other textual
 ** processing, as well. */

struct cutext_lsource
{
    struct cutext_bufsource bsrc;
    int indentation;
    struct cu_locbound locb;
};

void cutext_lsource_init(cutext_lsource_t lsrc, cutext_source_t src,
			 cu_locbound_t locb_start);

cu_bool_t cutext_lsource_init_fopen(cutext_lsource_t lsrc,
				    char const *enc, cu_str_t path);

cutext_lsource_t cutext_lsource_new(cutext_source_t subsrc,
				    cu_locbound_t locb_start);

cutext_lsource_t cutext_lsource_new_fopen(char const *enc, cu_str_t path);

CU_SINLINE cu_locbound_t
cutext_lsource_locbound(cutext_lsource_t src)
{ return &src->locb; }

CU_SINLINE int
cutext_lsource_indentation(cutext_lsource_t src)
{ return src->indentation; }

size_t cutext_lsource_read(cutext_lsource_t lsrc, cu_wchar_t *buf,
			   size_t size);

size_t cutext_lsource_skip(cutext_lsource_t lsrc, size_t size);

cu_wchar_t const *cutext_lsource_look(cutext_lsource_t lsrc,
				      size_t size, size_t *size_out);

/** Returns the next character from the source, without skipping over it. */
cu_wchar_t cutext_lsource_peek(cutext_lsource_t src);

/** Returns and skips over the next character of \a src. */
cu_wchar_t cutext_lsource_get(cutext_lsource_t src);

/** A currently rough approximation to a Unicode version of \ref
 ** cu_locbound_put_char. */
void cutext_locbound_put_wchar(cu_locbound_t locb, cu_wchar_t ch);

/** @} */
CU_END_DECLARATIONS

#endif
