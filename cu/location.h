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

#ifndef CU_LOCATION_H
#define CU_LOCATION_H

#include <cu/fwd.h>
#include <cu/debug.h>
#include <limits.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cu_location_h cu/location.h: Source Code Locations
 ** @{ \ingroup cu_mod */


/** \name Location Origin
 ** @{ */

struct cu_locorigin
{
    cu_str_t path;
    int tabstop;
};

void cu_locorigin_init(cu_locorigin_t origin, cu_str_t path, int tabstop);

cu_locorigin_t cu_locorigin_new(cu_str_t path, int tabstop);

int cu_locorigin_cmp(cu_locorigin_t origin0, cu_locorigin_t origin1);

CU_SINLINE cu_str_t cu_locorigin_path(cu_locorigin_t origin)
{ return origin->path; }

CU_SINLINE int cu_locorigin_tabstop(cu_locorigin_t origin)
{ return origin->tabstop; }


/** @}
 ** \name Location Boundary
 ** @{ */

struct cu_locbound
{
    cu_locorigin_t origin;
    int line;
    short col;
};

void cu_locbound_init(cu_locbound_t bound, cu_locorigin_t origin,
		      int line, int column);

void cu_locbound_init_copy(cu_locbound_t bound, cu_locbound_t bound0);

cu_locbound_t cu_locbound_new(cu_locorigin_t origin,
			      int line, int column);

cu_locbound_t cu_locbound_new_copy(cu_locbound_t bound0);

int cu_locbound_cmp(cu_locbound_t bound0, cu_locbound_t bound1);

CU_SINLINE cu_locorigin_t cu_locbound_origin(cu_locbound_t bound)
{ return bound->origin; }

CU_SINLINE int cu_locbound_line(cu_locbound_t bound)
{ return bound->line; }

CU_SINLINE int cu_locbound_column(cu_locbound_t bound)
{ return bound->col; }

CU_SINLINE void cu_locbound_skip_columns(cu_locbound_t bound, int n)
{
    cu_debug_assert(bound->col >= 0);
    bound->col += n;
}

void cu_locbound_put_tab(cu_locbound_t bound);
void cu_locbound_put_newline(cu_locbound_t bound);


/** @}
 ** \name Location
 ** @{ */

struct cu_location
{
    cu_locorigin_t origin;
    int   lbline, ubline;
    short lbcol,  ubcol;
};

void cu_location_init(cu_location_t loc, cu_locorigin_t origin,
		      int lbline, int lbcol, int ubline, int ubcol);

void cu_location_init_range(cu_location_t loc,
			    cu_locbound_t lbound, cu_locbound_t ubound);

void cu_location_init_cover(cu_location_t loc,
			    cu_location_t loc0, cu_location_t loc1);


cu_location_t cu_location_new(cu_locorigin_t origin,
			      int lbline, int lbcol, int ubline, int ubcol);

cu_location_t cu_location_new_range(cu_locbound_t lbound, cu_locbound_t ubound);

cu_location_t cu_location_new_cover(cu_location_t loc0, cu_location_t loc1);

CU_SINLINE cu_locorigin_t cu_location_origin(cu_location_t loc)
{ return loc->origin; }

CU_SINLINE int cu_location_origin_cmp(cu_location_t loc0, cu_location_t loc1)
{ return cu_locorigin_cmp(loc0->origin, loc1->origin); }

int cu_location_cmp(cu_location_t loc0, cu_location_t loc1);

CU_SINLINE cu_str_t cu_location_path(cu_location_t loc)
{ return cu_locorigin_path(loc->origin); }

CU_SINLINE int cu_location_height(cu_location_t loc)
{ return loc->ubline - loc->lbline; }

CU_SINLINE int cu_location_length(cu_location_t loc)
{ return loc->lbline != loc->ubline? INT_MAX : loc->ubcol - loc->lbcol; }

cu_locbound_t cu_location_lbound(cu_location_t loc);

cu_locbound_t cu_location_ubound(cu_location_t loc);

CU_SINLINE int cu_location_lb_line(cu_location_t loc) { return loc->lbline; }
CU_SINLINE int cu_location_lb_column(cu_location_t loc) { return loc->lbcol; }
CU_SINLINE int cu_location_ub_line(cu_location_t loc) { return loc->ubline; }
CU_SINLINE int cu_location_ub_column(cu_location_t loc) { return loc->ubcol; }

void cu_location_fprint(cu_location_t loc, FILE *file);

/** @}
 ** @} */
CU_END_DECLARATIONS

#endif
