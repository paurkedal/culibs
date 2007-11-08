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

#ifndef CUEX_OPR_PRIV_H
#define CUEX_OPR_PRIV_H

#include <cuex/fwd.h>

CU_BEGIN_DECLARATIONS

/* sigprod/2aci2 and sigbase/2
 * --------------------------- */

typedef struct cuex_opr_sigprod_cache_s *cuex_opr_sigprod_cache_t;
struct cuex_opr_sigprod_cache_s
{
    cu_offset_t alloc_size[CUCONF_LOG2_MAXALIGN + 2];
};
#define cuex_o4aci_sigprod_cache_size \
	sizeof(struct cuex_opr_sigprod_cache_s)
void cuex_o4aci_sigprod_cache_cct(cuex_opn_t);

#define cuex_opr_sigbase_cache_s cuex_opr_sigprod_cache_s
#define cuex_o2_sigbase_cache_size \
	sizeof(struct cuex_opr_sigbase_cache_s)
void cuex_o2_sigbase_cache_cct(cuex_opn_t);


/* dunion/2aci2
 * ------------ */

typedef struct cuex_opr_dunion_cache_s *cuex_opr_dunion_cache_t;
struct cuex_opr_dunion_cache_s
{
    int count;
};
#define cuex_opr_dunion_2aci2_cache_size \
	sizeof(struct cuex_opr_dunion_cache_s)
void cuex_opr_dunion_2aci2_cache_cct(cuex_opn_t);

CU_END_DECLARATIONS

#endif
