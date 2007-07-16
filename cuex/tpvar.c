/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuex/tpvar.h>

cuex_tpvar_t
cuex_tpvar(int index, cu_offset_t width_mask, cuex_t type_class)
{
    cudyn_hctem_decl(cuex_tpvar, tem);
    cudyn_hctem_init(cuex_tpvar, tem);
    cudyn_hctem_get(cuex_tpvar, tem)->width_mask = width_mask;
    cudyn_hctem_get(cuex_tpvar, tem)->type_class = type_class;
    return cuexP_halloc(cuex_tpvarmeta(index),
			cudyn_hctem_key_size(cuex_tpvar),
			cudyn_hctem_key(cuex_tpvar, tem));
}
