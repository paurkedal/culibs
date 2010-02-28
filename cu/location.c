/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <urkedal@nbi.dk>
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

#include <cu/location.h>
#include <cu/str.h>
#include <cu/memory.h>
#include <string.h>


/* cu_locorigin
 * ============= */

void
cu_locorigin_init(cu_locorigin_t origin, cu_str_t path, int tabstop)
{
    origin->path = path;
    origin->tabstop = tabstop;
}

cu_locorigin_t
cu_locorigin_new(cu_str_t path, int tabstop)
{
    cu_locorigin_t origin = cu_gnew(struct cu_locorigin);
    origin->path = path;
    origin->tabstop = tabstop;
    return origin;
}

int
cu_locorigin_cmp(cu_locorigin_t origin0, cu_locorigin_t origin1)
{
    int c;
    if (!origin0->path) return origin1->path ? -1 : 0;
    if (!origin1->path) return 1;
    c = cu_str_cmp(origin0->path, origin1->path);
    if (c) return c;
    else   return origin0->tabstop - origin1->tabstop;
}


/* cu_locbound
 * =========== */

void
cu_locbound_init(cu_locbound_t bound,
		 cu_locorigin_t origin, int line, int col)
{
    bound->origin = origin;
    bound->line = line;
    bound->col = col;
}

void
cu_locbound_init_copy(cu_locbound_t bound, cu_locbound_t bound0)
{
    memcpy(bound, bound0, sizeof(struct cu_locbound));
}

cu_locbound_t
cu_locbound_new(cu_locorigin_t origin, int line, int col)
{
    cu_locbound_t bound = cu_gnew(struct cu_locbound);
    cu_locbound_init(bound, origin, line, col);
    return bound;
}

cu_locbound_t
cu_locbound_new_copy(cu_locbound_t point0)
{
    cu_locbound_t bound = cu_gnew(struct cu_locbound);
    cu_locbound_init_copy(bound, point0);
    return bound;
}

int
cu_locbound_cmp(cu_locbound_t bound0, cu_locbound_t bound1)
{
    int c = cu_locorigin_cmp(bound0->origin, bound1->origin);
    if (c) return c;
    if (bound0->line < bound1->line) return -1;
    if (bound0->line > bound1->line) return  1;
    if (bound0->col  < bound1->col)  return -1;
    if (bound0->col  > bound1->col)  return  1;
    return 0;
}

void
cu_locbound_put_tab(cu_locbound_t bound)
{
    int tabstop = bound->origin->tabstop;
    bound->col = (bound->col / tabstop + 1) * tabstop;
}

void
cu_locbound_put_newline(cu_locbound_t bound)
{
    ++bound->line;
    if (bound->col > 0)
	bound->col = 0;
}


/* cu_location
 * =========== */

static int
_location_lb_cmp(cu_location_t loc0, cu_location_t loc1)
{
    if (loc0->lbline < loc1->lbline) return -1;
    if (loc0->lbline > loc1->lbline) return  1;
    if (loc0->lbcol  < loc1->lbcol)  return -1;
    if (loc0->lbcol  > loc1->lbcol)  return  1;
    return 0;
}

static int
_location_ub_cmp(cu_location_t loc0, cu_location_t loc1)
{
    if (loc0->ubline < loc1->ubline) return -1;
    if (loc0->ubline > loc1->ubline) return  1;
    if (loc0->ubcol  < loc1->ubcol)  return -1;
    if (loc0->ubcol  > loc1->ubcol)  return  1;
    return 0;
}

void
cu_location_init(cu_location_t loc, cu_locorigin_t origin,
		 int lbline, int lbcol, int ubline, int ubcol)
{
    loc->origin = origin;
    loc->lbline = lbline;
    loc->lbcol = lbcol;
    loc->ubline = ubline;
    loc->ubcol = ubcol;
}

void
cu_location_init_range(cu_location_t loc,
		       cu_locbound_t lbound, cu_locbound_t ubound)
{
    if (lbound->origin != ubound->origin)
	cu_bugf("Point locations passed to cu_location_init must share the "
		"same location context.");
    if (cu_locbound_cmp(lbound, ubound) > 0)
	cu_bugf("Attempt to create location from negative range.");

    loc->origin = lbound->origin;
    loc->lbline = lbound->line;
    loc->lbcol = lbound->col;
    loc->ubline = ubound->line;
    loc->ubcol = ubound->col;
}

void
cu_location_init_cover(cu_location_t loc,
		       cu_location_t loc0, cu_location_t loc1)
{
    if (loc0->origin != loc1->origin)
	cu_bugf("cu_location_init_cover called on incompatible locations.");
    loc->origin = loc0->origin;

    if (_location_lb_cmp(loc0, loc1) < 0) {
	loc->lbline = loc0->lbline;
	loc->lbcol = loc0->lbcol;
    }
    else {
	loc->lbline = loc1->lbline;
	loc->lbcol = loc1->lbcol;
    }

    if (_location_ub_cmp(loc0, loc1) < 0) {
	loc->ubline = loc1->ubline;
	loc->ubcol = loc1->ubcol;
    }
    else {
	loc->ubline = loc0->ubline;
	loc->ubcol = loc0->ubcol;
    }
}

cu_location_t
cu_location_new(cu_locorigin_t origin,
		int lbline, int lbcol, int ubline, int ubcol)
{
    cu_location_t loc = cu_gnew(struct cu_location);
    cu_location_init(loc, origin, lbline, lbcol, ubline, ubcol);
    return loc;
}

cu_location_t
cu_location_new_range(cu_locbound_t lbound, cu_locbound_t ubound)
{
    cu_location_t loc = cu_gnew(struct cu_location);
    cu_location_init_range(loc, lbound, ubound);
    return loc;
}

cu_location_t
cu_location_new_cover(cu_location_t loc0, cu_location_t loc1)
{
    cu_location_t loc = cu_gnew(struct cu_location);
    cu_location_init_cover(loc, loc0, loc1);
    return loc;
}

int
cu_location_lb_cmp(cu_location_t loc0, cu_location_t loc1)
{
    int c = cu_locorigin_cmp(loc0->origin, loc1->origin);
    if (c) return c;
    return _location_lb_cmp(loc0, loc1);
}

int
cu_location_ub_cmp(cu_location_t loc0, cu_location_t loc1)
{
    int c = cu_locorigin_cmp(loc0->origin, loc1->origin);
    if (c) return c;
    return _location_ub_cmp(loc0, loc1);
}

int
cu_location_cmp(cu_location_t loc0, cu_location_t loc1)
{
    int c = cu_locorigin_cmp(loc0->origin, loc1->origin);
    if (c) return c;
    c = _location_lb_cmp(loc0, loc1);
    if (c) return c;
    return _location_ub_cmp(loc0, loc1);
}

cu_locbound_t
cu_location_lbound(cu_location_t loc)
{
    return cu_locbound_new(loc->origin, loc->lbline, loc->lbcol);
}

cu_locbound_t
cu_location_ubound(cu_location_t loc)
{
    return cu_locbound_new(loc->origin, loc->ubline, loc->ubcol);
}

void
cu_location_fprint(cu_location_t loc, FILE *file)
{
    /* Vi starts counting at column 1, and while Emacs starts at 0, it assumes
     * that error messages contains columns staring from column 1.  So, we use
     * 1-based counting here. */

    cu_str_t path;

    if (loc == NULL) {
	fprintf(file, "*unknown*");
	return;
    }
    path = cu_location_path(loc);
    if (path)
	CU_DISCARD(fwrite(cu_str_charr(path), 1, cu_str_size(path), file));
    else
	fputs("*unknown*", file);

    if (loc->lbcol >= 0)
	fprintf(file, ":%d:%d", loc->lbline, loc->lbcol + 1);
    else
	fprintf(file, ":%d", loc->lbline);

    if (cu_location_length(loc)) {
	if (cu_location_height(loc) > 0) {
	    fprintf(file, "-%d", loc->ubline);
	    if (loc->ubcol >= 0)
		fprintf(file, ":%d", loc->ubcol + 1);
	}
	else if (loc->ubcol >= 0)
	    fprintf(file, "-%d", loc->ubcol + 1);
    }
}
