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

#ifndef CUTEXT_UCS4_H
#define CUTEXT_UCS4_H

#include <cu/fwd.h>
#include <cu/str.h>
#include <cu/wchar.h>
#include <stdint.h>

CU_BEGIN_DECLARATIONS

/* Convert 'src_size' UTF-8 bytes starting at 'src_arr' storing up to
 * 'dst_cap' UCS-4 characters starting at 'dst_arr', and set
 * '*dst_size' to the actual number stored.  Return 0 on success and
 * the 'errno' returned from 'iconv' on failure. */
int cutext_iconv_char_to_wchar(char const **src_arr, size_t *src_cnt,
			       cu_wchar_t **dst_arr, size_t *dst_cnt);

/* Convert 'src_size' UCS-4 characters starting at 'src_arr' storing
 * up to 'dst_cap' UTF-8 characters starting at 'dst_arr', and set
 * '*dst_size' to the actual numbers stored.  Return 0 on success and
 * the 'errno' returned from 'iconv' on failure. */
int cutext_iconv_wchar_to_char(cu_wchar_t const **src_arr, size_t *src_size,
			       char **dst_arr, size_t *dst_cap);

/* Convert a single UCS-4 character to an UTF-8 sequence. */
int cutext_wchar_to_charr(cu_wchar_t wc, char **dst_arr, size_t *dst_cnt);

cu_dsink_t cutext_dsink_open_iconv(char const *source_encoding,
				   char const *target_encoding,
				   cu_dsink_t target_sink);

CU_END_DECLARATIONS

#endif
