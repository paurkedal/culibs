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

#ifndef CUFLOW_CDISJ_H
#define CUFLOW_CDISJ_H

#include <cuflow/fwd.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_cdisj_h cuflow/cdisj.h: Inter-Thread Conditionals
 *@{\ingroup cuflow_mod */

typedef AO_t cuflow_cdisj_t;

void cuflowP_cdisj_wait_while(cuflow_cdisj_t *, cu_bool_t cond_val);
void cuflowP_cdisj_broadcast(cuflow_cdisj_t *);

CU_SINLINE void
cuflow_cdisj_wait_while(cuflow_cdisj_t *cdisj)
{
    if (AO_load(cdisj))
	cuflowP_cdisj_wait_while(cdisj, 0);
}

CU_SINLINE void
cuflow_cdisj_wait_until(cuflow_cdisj_t *cdisj)
{
    if (AO_load(cdisj))
	cuflowP_cdisj_wait_while(cdisj, 1);
}

CU_SINLINE void
cuflow_cdisj_sub1(cuflow_cdisj_t *cdisj)
{
    if (AO_fetch_and_sub1(cdisj) == 1)
	cuflowP_cdisj_broadcast(cdisj);
}

CU_SINLINE void
cuflow_cdisj_sub1_release(cuflow_cdisj_t *cdisj)
{
    if (AO_fetch_and_sub1_release(cdisj) == 1)
	cuflowP_cdisj_broadcast(cdisj);
}

CU_SINLINE void
cuflow_cdisj_sub1_release_write(cuflow_cdisj_t *cdisj)
{
    if (AO_fetch_and_sub1_release_write(cdisj) == 1)
	cuflowP_cdisj_broadcast(cdisj);
}

/*!@}*/
CU_END_DECLARATIONS

#endif
