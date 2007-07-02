/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_ASSOC_H
#define CUEX_ASSOC_H

#include <cuex/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_assoc_h cuex/assoc.h:
 *@{\ingroup cuex_mod */

typedef struct cuex_assoc_s *cuex_assoc_t;

extern cuex_t cuexP_assoc_empty;
extern cudyn_stdtype_t cuexP_assoc_type;

struct cuex_assoc_s
{
    CU_HCOBJ
    cu_word_t center;
    cuex_t left, right;
};

CU_SINLINE cuex_assoc_t
cuex_assoc_empty()
{ return cuexP_assoc_empty; }

CU_SINLINE cu_bool_t
cuex_is_assoc(cuex_t assoc)
{
    cuex_meta_t meta = cuex_meta(assoc);
    return (cuex_meta_is_opr(meta) && cuex_opr_r(meta) > 1)
	|| meta == cudyn_stdtype_to_meta(cuexP_assoc_type);
}

CU_SINLINE cu_bool_t
cuex_assoc_is_empty(cuex_t assoc)
{
    return assoc == cuexP_assoc_empty;
}

cuex_t cuex_assoc_find(cuex_t assoc, cuex_t key);
cuex_t cuex_assoc_insert(cuex_t assoc, cuex_t value);
cuex_t cuex_assoc_erase(cuex_t assoc, cuex_t key);
//cuex_t cuex_assoc_union(cuex_t assoc0, cuex_t assoc1);
//cuex_t cuex_assoc_isecn(cuex_t assoc0, cuex_t assoc1);

/*!@}*/
CU_END_DECLARATIONS

#endif
