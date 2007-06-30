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

#ifndef CUGRA_BDD_BUDDY_H
#define CUGRA_BDD_BUDDY_H

#include <cugra/fwd.h>
#include <atomic_ops.h>
#include <bdd.h>

CU_BEGIN_DECLARATIONS

/*!@{\ingroup internal_mod*/
void cugraP_bdd_setvarnum(int);
extern AO_t cugraP_bdd_varnum;
/*!@}*/

/*!Calls <tt>bdd_setvarnum(\e i)</tt> for some \e i ≥ \a min such that the
 * number of variables is incremented in a geometrical progression. */
CU_SINLINE void
cugra_bdd_setvarnum(int min)
{
    if (AO_load(&cugraP_bdd_varnum) < min)
	cugraP_bdd_setvarnum(min);
}

CU_SINLINE BDD
cugra_bdd_ithvar(int i)
{
    cugra_bdd_setvarnum(i + 1);
    return bdd_ithvar(i);
}

CU_SINLINE BDD
cugra_bdd_nithvar(int i)
{
    cugra_bdd_setvarnum(i + 1);
    return bdd_nithvar(i);
}

CU_END_DECLARATIONS

#endif
