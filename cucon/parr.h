/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_PARR_H
#define CUCON_PARR_H

#include <cucon/parray.h>

CU_BEGIN_DECLARATIONS

#define cucon_parr			cucon_parray
#define cucon_parr_init_empty		cucon_parray_init_empty
#define cucon_parr_new_empty		cucon_parray_new_empty
#define cucon_parr_init_size		cucon_parray_init_size
#define cucon_parr_new_size		cucon_parray_new_size
#define cucon_parr_init_fill		cucon_parray_init_fill
#define cucon_parr_new_fill		cucon_parray_new_fill
#define cucon_parr_swap			cucon_parray_swap
#define cucon_parr_size			cucon_parray_size
#define cucon_parr_ref_at		cucon_parray_ref_at
#define cucon_parr_at			cucon_parray_at
#define cucon_parr_set_at		cucon_parray_set_at
#define cucon_parr_resize_gp		cucon_parray_resize_gp
#define cucon_parr_resize_gpmax		cucon_parray_resize_gpmax
#define cucon_parr_resize_exact		cucon_parray_resize_exact
#define cucon_parr_resize_exactmax	cucon_parray_resize_exactmax
#define cucon_parr_resize_gp_fill	cucon_parray_resize_gp_fill
#define cucon_parr_resize_gpmax_fill	cucon_parray_resize_gpmax_fill
#define cucon_parr_resize_exact_fill	cucon_parray_resize_exact_fill
#define cucon_parr_resize_exactmax_fill	cucon_parray_resize_exactmax_fill
#define cucon_parr_append_gp		cucon_parray_append_gp
#define cucon_parr_fill_all		cucon_parray_fill_all
#define cucon_parr_fill_range		cucon_parray_fill_range
#define cucon_parr_begin		cucon_parray_begin
#define cucon_parr_end			cucon_parray_end

CU_END_DECLARATIONS

#endif
