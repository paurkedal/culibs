/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010--2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#ifndef CU_LOCATION_H
#define CU_LOCATION_H

#include <cu/fwd.h>
#include <cu/debug.h>
#include <limits.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cu_location_h cu/location.h: Source Code Locations
 ** @{ \ingroup cu_util_mod */


/** \name Location Origin
 ** @{ */

/** The origin of a location, with associated information. */
struct cu_locorigin
{
    cu_str_t path;
    int tabstop;
};

/** Initialize \a origin as referring to the file at \a path having tabstops in
 ** steps of \a tabstop. */
void cu_locorigin_init(cu_locorigin_t origin, cu_str_t path, int tabstop);

/** Return a location referring to the file at \a path having tabstops in steps
 ** of \a tabstop. */
cu_locorigin_t cu_locorigin_new(cu_str_t path, int tabstop);

/** Return negative, 0, or positive whether \a origin0 is considered smaller,
 ** equal, or greater than \a origin1. */
int cu_locorigin_cmp(cu_locorigin_t origin0, cu_locorigin_t origin1);

/** The path of the file to which \a origin is refers. */
CU_SINLINE cu_str_t cu_locorigin_path(cu_locorigin_t origin)
{ return origin->path; }

/** The width of the tabstops used in \a origin. */
CU_SINLINE int cu_locorigin_tabstop(cu_locorigin_t origin)
{ return origin->tabstop; }


/** @}
 ** \name Location Boundary
 ** @{ */

/** A point reference within a text, used as the lower or upper bound of a
 ** location. */
struct cu_locbound
{
    cu_locorigin_t origin;
    int line;
    short col;
};

/** Initialize \a bound to refer to \a line and \a column within \a origin. */
void cu_locbound_init(cu_locbound_t bound, cu_locorigin_t origin,
		      int line, int column);

/** Initialize \a bound as the first position of the file at \a path using
 ** tabstop spaced at 8 columns. */
void cu_locbound_init_file(cu_locbound_t bound, cu_str_t path);

/** Initialize \a bound as a copy of \a bound0. */
void cu_locbound_init_copy(cu_locbound_t bound, cu_locbound_t bound0);

/** Return a location boundary which refers to \a line and \a column within \a
 ** origin. */
cu_locbound_t cu_locbound_new(cu_locorigin_t origin,
			      int line, int column);


/** Initialize \a bound as the first position of the file at \a path using 8
 ** column tabstops. */
cu_locbound_t cu_locbound_new_file(cu_str_t path);

/** Return a copy of \a bound0. */
cu_locbound_t cu_locbound_new_copy(cu_locbound_t bound0);

/** Returns negative, zero, or positive whether \a bound0 is considered less
 ** than, equal to, or greater than \a bound1.  Within the same file, later
 ** positions are greater. */
int cu_locbound_cmp(cu_locbound_t bound0, cu_locbound_t bound1);

/** The origin of \a bound. */
CU_SINLINE cu_locorigin_t cu_locbound_origin(cu_locbound_t bound)
{ return bound->origin; }

/** The current line number of \a bound. */
CU_SINLINE int cu_locbound_line(cu_locbound_t bound)
{ return bound->line; }

/** The current column number of \a bound. */
CU_SINLINE int cu_locbound_column(cu_locbound_t bound)
{ return bound->col; }

/** Advance \a bound by \a n columns. */
CU_SINLINE void cu_locbound_skip(cu_locbound_t bound, int n)
{
    cu_debug_assert(bound->col >= 0);
    bound->col += n;
}

/** Advance \a bound by 1 column. */
CU_SINLINE void cu_locbound_skip_1(cu_locbound_t bound)
{ cu_locbound_skip(bound, 1); }

/** Advance \a bound to the next tabstop. */
void cu_locbound_put_tab(cu_locbound_t bound);

/** Advance the line number of \a bound and reset the column to 0. */
void cu_locbound_put_newline(cu_locbound_t bound);

/** Advance \a bound over \a ch.  This works for ASCII characters.  Characters
 ** above 127 have no effect, which yields a first approximation to UTF-8.  */
void cu_locbound_put_char(cu_locbound_t bound, char ch);


/** @}
 ** \name Location
 ** @{ */

/** A reference to a source code location of finite width. */
struct cu_location
{
    cu_locorigin_t origin;
    int   lbline, ubline;
    short lbcol,  ubcol;
    cu_location_t chain_tail; /* backwards compat */
};

/** Initialize \a loc as the given range range within \a origin. */
void cu_location_init(cu_location_t loc, cu_locorigin_t origin,
		      int lbline, int lbcol, int ubline, int ubcol);

/** Initialize \a loc as a copy of \a loc0. */
void cu_location_init_copy(cu_location_t loc, cu_location_t loc0);

/** Initialize \a loc as the range from \a lbound to \a ubound, which must
 ** refer to the same origin. */
void cu_location_init_range(cu_location_t loc,
			    cu_locbound_t lbound, cu_locbound_t ubound);

/** Initialize \a loc as the narrowest location which covers \a loc0 and \a
 ** loc1, where the latter two must refer to the same origin. */
void cu_location_init_cover(cu_location_t loc,
			    cu_location_t loc0, cu_location_t loc1);

/** Initialize \a loc as the point location at \a bound. */
void cu_location_init_point(cu_location_t loc, cu_locbound_t bound);

/** Initialize \a loc as the point location at the lower boundary of \a loc0. */
void cu_location_init_point_lb(cu_location_t loc, cu_location_t loc0);

/** Initialize \a loc as the point location at the upper boundary of \a loc0. */
void cu_location_init_point_ub(cu_location_t loc, cu_location_t loc0);

/** Return the location for the given range within \a origin. */
cu_location_t cu_location_new(cu_locorigin_t origin,
			      int lbline, int lbcol, int ubline, int ubcol);

/** Return a location identical to \a loc0. */
cu_location_t cu_location_new_copy(cu_location_t loc0);

/** Return the location for the range from \a lbound to \a ubound, which must
 ** refer to the same origin. */
cu_location_t cu_location_new_range(cu_locbound_t lbound, cu_locbound_t ubound);

/** Return the narrowest location which covers \a loc0 and \a loc1, where the
 ** latter two must refer to the same origin. */
cu_location_t cu_location_new_cover(cu_location_t loc0, cu_location_t loc1);

/** The origin of \a loc. */
CU_SINLINE cu_locorigin_t cu_location_origin(cu_location_t loc)
{ return loc->origin; }

/** Returns a \ref cu_locorigin_cmp applied to the origins of \a loc0 and \a
 ** loc1. */
CU_SINLINE int cu_location_origin_cmp(cu_location_t loc0, cu_location_t loc1)
{ return cu_locorigin_cmp(loc0->origin, loc1->origin); }

/** Return negative, zero, or positive whether \a loc0 is smaller than, equal
 ** to, or greater than \a loc1, considering the lower bound bound significant
 ** than the upper. */
int cu_location_cmp(cu_location_t loc0, cu_location_t loc1);

/** The path of the file within which \a loc refer. */
CU_SINLINE cu_str_t cu_location_path(cu_location_t loc)
{ return cu_locorigin_path(loc->origin); }

/** The Number of lines spanned by \a loc, starting from zero if \a loc is
 ** contained within a line. */
CU_SINLINE int cu_location_height(cu_location_t loc)
{ return loc->ubline - loc->lbline; }

/** If \a loc is within a line, the number of columns spanned, otherwise \c
 ** INT_MAX. */
CU_SINLINE int cu_location_length(cu_location_t loc)
{ return loc->lbline != loc->ubline? INT_MAX : loc->ubcol - loc->lbcol; }

/** The lower boundary of \a loc. */
cu_locbound_t cu_location_lbound(cu_location_t loc);

/** The upper boundary of \a loc. */
cu_locbound_t cu_location_ubound(cu_location_t loc);

/** The line of the lower boundary of \a loc. */
CU_SINLINE int cu_location_lb_line(cu_location_t loc) { return loc->lbline; }

/** The column of the lower boundary of \a loc. */
CU_SINLINE int cu_location_lb_column(cu_location_t loc) { return loc->lbcol; }

/** The line of the upper boundary of \a loc. */
CU_SINLINE int cu_location_ub_line(cu_location_t loc) { return loc->ubline; }

/** The column of the upper boundary of \a loc. */
CU_SINLINE int cu_location_ub_column(cu_location_t loc) { return loc->ubcol; }

/** Print \a loc to \a file. */
void cu_location_fprint(cu_location_t loc, FILE *file);

/** @}
 ** @} */
CU_END_DECLARATIONS

#endif
