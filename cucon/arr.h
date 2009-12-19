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


/* DEPRECATED.  This file is here for backwards compatibility only.  It will be
 * removed in a future version. */

#ifndef CUCON_ARR_H
#define CUCON_ARR_H

#include <cucon/array.h>

CU_BEGIN_DECLARATIONS

#define cucon_arr		cucon_array
#define cucon_arr_init		cucon_array_init
#define cucon_arr_new		cucon_array_new
void cucon_arr_init_empty(cucon_arr_t arr)		CU_ATTR_DEPRECATED;
cucon_arr_t cucon_arr_new_empty(void)			CU_ATTR_DEPRECATED;
void cucon_arr_init_size(cucon_arr_t arr, size_t size)	CU_ATTR_DEPRECATED;
cucon_arr_t cucon_arr_new_size(size_t size)		CU_ATTR_DEPRECATED;
#define cucon_arr_swap		cucon_array_swap
#define cucon_arr_size		cucon_array_size
#define cucon_arr_ref_at	cucon_array_ref_at
#define cucon_arr_detach	cucon_array_detach
#define cucon_arr_resize_gp	cucon_array_resize_gp
#define cucon_arr_resize_gpmax	cucon_array_resize_gpmax
#define cucon_arr_resize_exact	cucon_array_resize_exact
#define cucon_arr_resize_exactmax cucon_array_resize_exactmax
#define cucon_arr_extend_gp	cucon_array_extend_gp
#define cucon_arr_extend_exact	cucon_array_extend_exact
#define cucon_arr_begin		cucon_array_begin
#define cucon_arr_end		cucon_array_end

CU_END_DECLARATIONS

#endif
