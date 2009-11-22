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

#ifndef CUTEXT_UCS4SRC_H
#define CUTEXT_UCS4SRC_H

#include <cu/sref.h>
#include <cutext/src.h>
#include <cucon/pmap.h>
#include <cu/cstr.h>
#include <cutext/conv.h>

CU_BEGIN_DECLARATIONS

struct cutext_ucs4src
{
    struct cutext_src src;
    struct cu_sref srf;
    int indentation; /* -1 if unknown (i.e. before first non-blank on line) */
    cutext_status_t st;
    cucon_pmap_t properties;
};

/* Create an UCS-4 source from 'producer'.  The three last parameters
 * are used only to construct a cu_sref_t. */
void cutext_ucs4src_init(cutext_ucs4src_t ucs4src, cutext_producer_t producer,
			 cu_str_t path, int line, int column);
cutext_ucs4src_t cutext_ucs4src_new(cutext_producer_t producer,
				    cu_str_t path, int line, int column);

/* Create an UCS-4 source from 'producer' with conversion from an detected
 * character set among UTF-8, UTF-16, UTF-16LE, UCS-4 or UCS-4LE. */
void cutext_ucs4src_init_detect(cutext_ucs4src_t usc4src,
				cutext_producer_t producer,
				cu_str_t path, int line, int column);
cutext_ucs4src_t cutext_ucs4src_new_detect(cutext_producer_t producer,
					   cu_str_t path,
					   int line, int column);
CU_SINLINE cucon_pmap_t
cutext_ucs4src_properties(cutext_ucs4src_t ucs4src)
{
    return ucs4src->properties;
}

CU_SINLINE cu_str_t
cutext_ucs4src_getprop_by_key(cutext_ucs4src_t ucs4src, char const *key)
{
    if (!ucs4src->properties)
	return NULL;
    return (cu_str_t)cucon_pmap_find_ptr(ucs4src->properties, key);
}

CU_SINLINE cu_str_t
cutext_ucs4src_getprop_by_cstr(cutext_ucs4src_t ucs4src, char const *key)
{
    if (!ucs4src->properties)
	return NULL;
    return (cu_str_t)cucon_pmap_find_ptr(ucs4src->properties, cu_struniq(key));
}

CU_SINLINE cu_sref_t
cutext_ucs4src_sref(cutext_ucs4src_t ucs4src)
{
    return &ucs4src->srf;
}

CU_SINLINE cu_sref_t
cutext_ucs4src_sref_copy(cutext_ucs4src_t ucs4src)
{
    return cu_sref_new_copy(&ucs4src->srf);
}

/*!Return the indentation of the current line or -1 if the current position
 * is before or at the first non-blank character. */
CU_SINLINE int
cutext_ucs4src_indentation(cutext_ucs4src_t ucs4src)
{
    return ucs4src->indentation;
}

CU_SINLINE cu_bool_t
cutext_ucs4src_is_eof(cutext_ucs4src_t ucs4src)
{
    return cutext_src_data_size(&ucs4src->src) == 0;
}

/* Have a peek at the next character. */
CU_SINLINE cu_wchar_t
cutext_ucs4src_peek(cutext_ucs4src_t ucs4src)
{
    return *(cu_wchar_t *)ucs4src->src.buf.content_start;
}

/* Internal */
cu_wchar_t *
cutextP_ucs4src_peek_arr(cutext_ucs4src_t ucs4src, size_t size);

/* Returns an array of 'size' B lookahead or a zero-terminated one. */
CU_SINLINE cu_wchar_t *
cutext_ucs4src_peek_arr(cutext_ucs4src_t ucs4src, size_t size)
{
    size_t avail = cutext_src_data_size(&ucs4src->src)/4;
    if (size > avail)
	return cutextP_ucs4src_peek_arr(ucs4src, size);
    else
	return (cu_wchar_t *)ucs4src->src.buf.content_start;
}

/* Internal */
void cutextP_ucs4src_terminate(cutext_ucs4src_t ucs4src);

void cutext_ucs4src_advance(cutext_ucs4src_t ucs4src, size_t size);

/* Reads one character from ucs4src.  Returns 0 if EOS. */
CU_SINLINE cu_wchar_t
cutext_ucs4src_get(cutext_ucs4src_t ucs4src)
{
    cu_wchar_t ch = cutext_ucs4src_peek(ucs4src);
    cutext_ucs4src_advance(ucs4src, 1);
    return ch;
}

/* Backward compat. */
#define cutext_ucs4src_cct		cutext_ucs4src_init
#define cutext_ucs4src_cct_detect	cutext_ucs4src_init_detect

CU_END_DECLARATIONS

#endif
