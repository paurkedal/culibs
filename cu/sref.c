/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cu/sref.h>
#include <cu/memory.h>
#include <cu/str.h>
#include <string.h>


void
cu_sref_init(cu_sref_t srf, cu_str_t path, int line, int column)
{
    cu_location_init(srf, cu_locorigin_new(path, 8),
		     line, column, line, column);
}

cu_sref_t
cu_sref_new(cu_str_t path, int line, int column)
{
    cu_sref_t srf = cu_gnew(struct cu_sref);
    cu_sref_init(srf, path, line, column);
    return srf;
}

cu_sref_t
cu_sref_new_cstr(char const *path, int line, int column)
{
    return cu_sref_new(cu_str_new_cstr(path), line, column);
}

void
cu_sref_init_range(cu_sref_t srf, cu_str_t path,
		  int first_line, int first_column,
		  int last_line, int last_column)
{
    cu_debug_assert((first_column >= 0) == (last_column >= 0));
    cu_location_init(srf, cu_locorigin_new(path, 8),
		     first_line, first_column, last_line, last_column);
}

cu_sref_t
cu_sref_new_range(cu_str_t path,
		  int first_line, int first_column,
		  int last_line, int last_column)
{
    cu_sref_t srf = cu_gnew(struct cu_sref);
    cu_sref_init_range(srf, path,
		      first_line, first_column, last_line, last_column);
    return srf;
}

void
cu_sref_init_span(cu_sref_t srf, cu_sref_t first, cu_sref_t last)
{
    cu_location_init_cover(srf, first, last);
}

cu_sref_t
cu_sref_new_sref_range(cu_sref_t first, cu_sref_t last)
{
    cu_sref_t srf = cu_gnew(struct cu_sref);
    cu_sref_init_span(srf, first, last);
    return srf;
}

void
cu_sref_init_first(cu_sref_t srf, cu_sref_t range)
{
    cu_location_init(srf, cu_location_origin(range),
		     cu_location_lb_line(range), cu_location_lb_column(range),
		     cu_location_lb_line(range), cu_location_lb_column(range));
}

void
cu_sref_init_last(cu_sref_t srf, cu_sref_t range)
{
    cu_location_init(srf, cu_location_origin(range),
		     cu_location_ub_line(range), cu_location_ub_column(range),
		     cu_location_ub_line(range), cu_location_ub_column(range));
}

void
cu_sref_set_sref_first(cu_sref_t srf, cu_sref_t first)
{
    cu_debug_assert(cu_sref_compatible(srf, first));
    srf->lbline = first->lbline;
    srf->lbcol = first->lbcol;
}

void
cu_sref_set_sref_last(cu_sref_t srf, cu_sref_t last)
{
    cu_debug_assert(cu_sref_compatible(srf, last));
    srf->ubline = last->ubline;
    srf->ubcol = last->ubcol;
}

cu_bool_t
cu_sref_compatible(cu_sref_t srf0, cu_sref_t srf1)
{
    return cu_location_origin_cmp(srf0, srf1) == 0;
}

void
cu_sref_set_tabstop(cu_sref_t srf, int w)
{
    srf->origin->tabstop = w;
}

void
cu_sref_init_copy(cu_sref_t srf, cu_sref_t srf0)
{
    memcpy(srf, srf0, sizeof(struct cu_location));
}

cu_sref_t
cu_sref_new_copy(cu_sref_t srf0)
{
    cu_sref_t srf = cu_gnew(struct cu_sref);
    cu_sref_init_copy(srf, srf0);
    return srf;
}

void
cu_sref_advance_columns(cu_sref_t sref, int cnt)
{
    cu_debug_assert(cu_sref_is_point(sref));
    sref->ubcol = sref->lbcol += cnt;
}

void
cu_sref_newline(cu_sref_t srf)
{
    cu_debug_assert(cu_sref_is_point(srf));
    if (srf->lbcol > 0)
	srf->lbcol = srf->ubcol = 0;
    ++srf->lbline;
    ++srf->ubline;
}

void
cu_sref_tab(cu_sref_t srf)
{
    int tabstop = srf->origin->tabstop;
    cu_debug_assert(srf->lbcol == srf->ubcol);
    srf->lbcol = srf->ubcol = (srf->lbcol/tabstop + 1)*tabstop;
}
