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

#ifndef CUEX_OPNTPL_H
#define CUEX_OPNTPL_H

#include <cuex/opn.h>

CU_BEGIN_DECLARATIONS

cuex_opn_t cuex_opntpl_new(cuex_meta_t opr);

CU_SINLINE cuex_t
cuex_opntpl_ref_at(cuex_opn_t opn, cu_rank_t i)
{
    return &opn->operand_arr[i];
}

cuex_t cuex_of_opntpl(cuex_t ex);

CU_END_DECLARATIONS

#endif
