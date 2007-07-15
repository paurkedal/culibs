/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuex/misc.h>
#include <cu/dyn.h>
#include <cu/halloc.h>

struct cuex_long_s
{
    CU_HCOBJ
    long i;
};

cuex_t
cuex_of_long(long i)
{
    cudyn_hctem_decl(cuex_long, key);
    cudyn_hctem_init(cuex_long, key);
    cudyn_hctem_get(cuex_long, key)->i = i;
    return cudyn_hctem_new(cuex_long, key);
}

cudyn_stdtype_t cuexP_long_type;

void
cuexP_misc_init()
{
    cuexP_long_type = cudyn_stdtype_new_hcs(sizeof(long));
}
