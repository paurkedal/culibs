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


void
cu_sref_cct(cu_sref_t srf, cu_str_t path, int line, int column)
{
    srf->chain_tail = NULL;
    srf->path = path;
    srf->line = line;
    srf->column = column;
    srf->last_line = -1;
    srf->last_column = -1;
    srf->tabstop = 8;
}

cu_sref_t
cu_sref_new(cu_str_t path, int line, int column)
{
    cu_sref_t srf = cu_gnew(struct cu_sref_s);
    srf->chain_tail = NULL;
    srf->path = path;
    srf->line = line;
    srf->column = column;
    srf->last_line = -1;
    srf->last_column = -1;
    srf->tabstop = 8;
    return srf;
}

cu_sref_t
cu_sref_new_cstr(char const *path, int line, int column)
{
    return cu_sref_new(cu_str_new_cstr(path), line, column);
}

void
cu_sref_cct_range(cu_sref_t srf, cu_str_t path,
		  int first_line, int first_column,
		  int last_line, int last_column)
{
    cu_debug_assert((first_column >= 0) == (last_column >= 0));
    srf->chain_tail = NULL;
    srf->path = path;
    srf->line = first_line;
    srf->column = first_column;
    srf->last_line = last_line;
    srf->last_column = last_column;
    srf->tabstop = 8;
}

cu_sref_t
cu_sref_new_range(cu_str_t path,
		  int first_line, int first_column,
		  int last_line, int last_column)
{
    cu_sref_t srf = cu_gnew(struct cu_sref_s);
    cu_sref_cct_range(srf, path,
		      first_line, first_column, last_line, last_column);
    return srf;
}

void
cu_sref_cct_sref_range(cu_sref_t srf, cu_sref_t first, cu_sref_t last)
{
    cu_debug_assert(cu_sref_compatible(first, last));
    srf->chain_tail = NULL;
    srf->path = first->path;
    srf->line = first->line;
    srf->column = first->column;
    if (cu_sref_is_point(last)) {
	srf->last_line = last->line;
	srf->last_column = last->column;
    } else {
	srf->last_line = last->last_line;
	srf->last_column = last->last_column;
    }
    srf->tabstop = first->tabstop;
}

cu_sref_t
cu_sref_new_sref_range(cu_sref_t first, cu_sref_t last)
{
    cu_sref_t srf = cu_gnew(struct cu_sref_s);
    cu_sref_cct_sref_range(srf, first, last);
    return srf;
}

void
cu_sref_cct_sref_first(cu_sref_t srf, cu_sref_t range)
{
    srf->chain_tail = NULL;
    srf->path = range->path;
    srf->line = range->line;
    srf->column = range->column;
    srf->last_line = srf->last_column = -1;
    srf->tabstop = range->tabstop;
}

void
cu_sref_cct_sref_last(cu_sref_t srf, cu_sref_t range)
{
    srf->chain_tail = NULL;
    srf->path = range->path;
    if (cu_sref_is_point(range)) {
	srf->line = range->line;
	srf->column = range->column;
    } else {
	srf->line = range->last_line;
	srf->column = range->last_column;
    }
    srf->last_line = srf->last_column = -1;
    srf->tabstop = range->tabstop;
}

void
cu_sref_set_sref_first(cu_sref_t srf, cu_sref_t first)
{
    cu_debug_assert(cu_sref_compatible(srf, first));
    srf->line = first->line;
    srf->column = first->column;
}

void
cu_sref_set_sref_last(cu_sref_t srf, cu_sref_t last)
{
    cu_debug_assert(cu_sref_compatible(srf, last));
    if (cu_sref_is_point(last)) {
	srf->last_line = last->line;
	srf->last_column = last->column;
    } else {
	srf->last_line = last->last_line;
	srf->last_column = last->last_column;
    }
}

cu_bool_t
cu_sref_compatible(cu_sref_t srf0, cu_sref_t srf1)
{
    if (srf0->tabstop != srf1->tabstop)
	return cu_false;
    if (srf0->path)
	return srf1->path && cu_str_cmp(srf0->path, srf1->path) == 0;
    else
	return !srf1->path;
}

void
cu_sref_set_tabstop(cu_sref_t srf, int w)
{
    srf->tabstop = w;
}

void
cu_sref_cct_copy(cu_sref_t srf, cu_sref_t srf0)
{
    srf->chain_tail = srf0->chain_tail;
    srf->path = srf0->path;
    srf->line = srf0->line;
    srf->column = srf0->column;
    srf->last_line = srf0->last_line;
    srf->last_column = srf0->last_column;
    srf->tabstop = srf0->tabstop;
}

cu_sref_t
cu_sref_new_copy(cu_sref_t srf0)
{
    cu_sref_t srf = cu_gnew(struct cu_sref_s);
    cu_sref_cct_copy(srf, srf0);
    return srf;
}

void
cu_sref_newline(cu_sref_t srf)
{
    if (srf->column > 0)
	srf->column = 0;
    ++srf->line;
}

void
cu_sref_tab(cu_sref_t srf)
{
    srf->column = (srf->column/srf->tabstop + 1)*srf->tabstop;
}

void
cu_sref_fprint(cu_sref_t srf, FILE* file)
{
    if (!cu_sref_is_known(srf)) {
	fprintf(file, "#[unknown]");
	return;
    }
    if (srf->path)
	fwrite(cu_str_charr(srf->path), 1, cu_str_size(srf->path), file);
    else
	fputs("#[unknown]", file);
    if (srf->column >= 0)
	/* Note. 'vi' starts counting at column 1. 'emacs' starts at 0
	 * but assumes error messages starts at 1. */
	fprintf(file, ":%d:%d",
		cu_sref_line(srf), cu_sref_column(srf) + 1);
    else
	fprintf(file, ":%d", cu_sref_line(srf));
    if (srf->last_line >= 0) {
	cu_debug_assert((srf->last_column >= 0) == (srf->column >= 0));
	if (srf->last_line != srf->line) {
	    fprintf(file, "-%d", srf->last_line);
	    if (srf->last_column >= 0)
		fprintf(file, ":%d", srf->last_column);
	}
	else if (srf->last_column >= 0)
	    fprintf(file, "-%d", srf->last_column);
    }
}
