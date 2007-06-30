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

#include <cugra/bdd_buddy.h>
#include <cu/thread.h>

static cu_mutex_t buddy_mutex = CU_MUTEX_INITIALISER;
AO_t cugraP_bdd_varnum;

void
cugraP_bdd_setvarnum(int min)
{
    int cur;
    cu_mutex_lock(&buddy_mutex);
    cur = bdd_varnum();
    if (cur < min) {
	cur *= 2;
	if (cur < min)
	    cur = min;
	bdd_setvarnum(cur);
    }
    AO_store(&cugraP_bdd_varnum, cur);
    cu_mutex_unlock(&buddy_mutex);
}
