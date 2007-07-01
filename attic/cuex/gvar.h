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

#ifndef CUEX_GVAR_H
#define CUEX_GVAR_H

#include <cuex/fwd.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS

/*!The meta of a global variable quantifid as \a qcode. */
#define cuex_gvarmeta(qcode) cuex_varmeta_kqi(cuex_varkind_gvar, qcode, 0)

struct cuex_gvar_s
{
    CU_OBJ
    cuex_t context;
    cudyn_type_t type;
    void *value;
};

/*!Returns a new global variable.  If \a type is \c cudyn_cuex_type() then
 * \a value */
cuex_gvar_t cuex_gvar_new_e(cuex_t context, cudyn_type_t type, void *value);

/*!The type of the value of \a gvar. */
CU_SINLINE cudyn_type_t cuex_gvar_type(cuex_gvar_t gvar)
{ return gvar->type; }

CU_SINLINE cuex_t cuex_gvar_context(cuex_gvar_t gvar)
{ return gvar->context; }

CU_SINLINE void *cuex_gvar_get(cuex_gvar_t gvar)
{ return gvar->value; }

CU_END_DECLARATIONS

#endif
