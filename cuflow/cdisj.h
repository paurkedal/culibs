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

/*!Type used to maintain a disjunction of several conditions across threads.
 * This is a simple atomic integer with a special interpretation.  Typically a
 * non-zero value means that work scheduled by the current thread is not yet
 * completed. */
typedef AO_t cuflow_cdisj_t;

#ifndef CU_IN_DOXYGEN
void cuflowP_cdisj_wait_while(cuflow_cdisj_t *, cu_bool_t cond_val);
void cuflowP_cdisj_broadcast(cuflow_cdisj_t *);
#endif

/*!Waits while \c *\a cdisj is non-zero, tending to scheduled work if the
 * condition is not fulfilled. */
CU_SINLINE void
cuflow_cdisj_wait_while(cuflow_cdisj_t *cdisj)
{
    if (AO_load(cdisj))
	cuflowP_cdisj_wait_while(cdisj, cu_false);
}

/*!Waits until \c *\a cdisj becomes non-zero, tending to scheduled work if the
 * condition is not fulfilled */
CU_SINLINE void
cuflow_cdisj_wait_until(cuflow_cdisj_t *cdisj)
{
    if (AO_load(cdisj))
	cuflowP_cdisj_wait_while(cdisj, cu_true);
}

/*!Decrements \c *\a cdisj and if it becomes zero, broadcasts to other threads
 * possibly waiting for \a cdisj to become zero. */
CU_SINLINE void
cuflow_cdisj_sub1(cuflow_cdisj_t *cdisj)
{
    if (AO_fetch_and_sub1(cdisj) == 1)
	cuflowP_cdisj_broadcast(cdisj);
}

/*!A variant of \ref cuflow_cdisj_sub1 which release barrier (cf
 * AO_fetch_and_sub1_release). */
CU_SINLINE void
cuflow_cdisj_sub1_release(cuflow_cdisj_t *cdisj)
{
    if (AO_fetch_and_sub1_release(cdisj) == 1)
	cuflowP_cdisj_broadcast(cdisj);
}

/*!A variant of \ref cuflow_cdisj_sub1 with write-release barrier (cf
 * AO_fetch_and_sub1_release_write). */
CU_SINLINE void
cuflow_cdisj_sub1_release_write(cuflow_cdisj_t *cdisj)
{
    if (AO_fetch_and_sub1_release_write(cdisj) == 1)
	cuflowP_cdisj_broadcast(cdisj);
}

/*!@}*/
CU_END_DECLARATIONS

#endif
