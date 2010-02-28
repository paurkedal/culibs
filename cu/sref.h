/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2010  Petter Urkedal <urkedal@nbi.dk>
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

/* !!! DEPRECATION WARNING !!!
 *
 * Everything in this header is deprecated and will be removed.  Use
 * cu/location.h instead.
 */

#ifndef CU_SREF_H
#define CU_SREF_H

#include <stdio.h>
#include <cu/location.h>

CU_BEGIN_DECLARATIONS

#define cu_sref cu_location
void cu_sref_init(cu_sref_t, cu_str_t path, int line, int column);
cu_sref_t cu_sref_new(cu_str_t path, int line, int column);
cu_sref_t cu_sref_new_cstr(char const *path, int line, int column);
void cu_sref_init_range(cu_sref_t, cu_str_t path,
			int first_line, int first_column,
			int last_line, int last_column);
cu_sref_t cu_sref_new_range(cu_str_t path,
			    int first_line, int first_column,
			    int last_line, int last_column);
void cu_sref_init_span(cu_sref_t, cu_sref_t first, cu_sref_t last);
cu_sref_t cu_sref_new_span(cu_sref_t first, cu_sref_t last);
void cu_sref_init_first(cu_sref_t srf, cu_sref_t src);
void cu_sref_init_last(cu_sref_t srf, cu_sref_t src);
void cu_sref_set_sref_first(cu_sref_t srf, cu_sref_t first);
void cu_sref_set_sref_last(cu_sref_t srf, cu_sref_t last);
cu_bool_t cu_sref_compatible(cu_sref_t srf0, cu_sref_t srf1);
CU_SINLINE cu_bool_t cu_sref_is_point(cu_sref_t srf)
{ return srf->lbline == srf->ubline && srf->lbcol == srf->ubcol; }
CU_SINLINE cu_bool_t cu_sref_is_range(cu_sref_t srf)
{ return !cu_sref_is_point(srf); }
CU_SINLINE void
cu_sref_set_chain_tail(cu_sref_t head, cu_sref_t chain_tail)
{ head->chain_tail = chain_tail; }
CU_SINLINE cu_sref_t
cu_sref_chain_tail(cu_sref_t srf) { return srf->chain_tail; }
void cu_sref_set_tabstop(cu_sref_t, int w);
CU_SINLINE cu_sref_t
cu_sref_unknown(void) { return NULL; }
CU_SINLINE int
cu_sref_is_known(cu_sref_t srf) { return srf != NULL; }
void cu_sref_init_copy(cu_sref_t srf, cu_sref_t srf0);
cu_sref_t cu_sref_new_copy(cu_sref_t);
CU_SINLINE cu_str_t
cu_sref_path(cu_sref_t srf) { return srf->origin->path; }
CU_SINLINE int
cu_sref_is_stdin(cu_sref_t srf) { return srf->origin->path == NULL; }
CU_SINLINE int
cu_sref_line(cu_sref_t srf) { return srf->lbline; }
CU_SINLINE int
cu_sref_column(cu_sref_t srf) { return srf->lbcol; }
CU_SINLINE void
cu_sref_next(cu_sref_t srf) { srf->ubcol = ++srf->lbcol; }
void cu_sref_tab(cu_sref_t);
void cu_sref_newline(cu_sref_t);
CU_SINLINE void
cu_sref_advance_char(cu_sref_t srf, char ch)
{
    switch (ch) {
	case '\n': cu_sref_newline(srf); break;
	case '\t': cu_sref_tab(srf); break;
	default: if ((unsigned char)ch <= 127) cu_sref_next(srf); break;
    }
}
void cu_sref_advance_columns(cu_sref_t sref, int cnt);
#define cu_sref_fprint cu_location_fprint
/*!\deprecated Use \ref cu_sref_init. */
#define cu_sref_cct		cu_sref_init
/*!\deprecated Use \ref cu_sref_init_range. */
#define cu_sref_cct_range	cu_sref_init_range
/*!\deprecated Use \ref cu_sref_init_span. */
#define cu_sref_cct_sref_range	cu_sref_init_span
/*!\deprecated Use \ref cu_sref_init_first. */
#define cu_sref_cct_sref_first	cu_sref_init_first
/*!\deprecated Use \ref cu_sref_init_last. */
#define cu_sref_cct_sref_last	cu_sref_init_last
/*!\deprecated Use \ref cu_sref_init_copy. */
#define cu_sref_cct_copy	cu_sref_init_copy
/*!\deprecated Use \ref cu_sref_new_span. */
#define cu_sref_new_sref_range	cu_sref_new_span

CU_END_DECLARATIONS
#endif
