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

#include <cuflow/tstate.h>
#include <cu/thread.h>
#include <cu/memory.h>

static pthread_mutex_t _tstate_chain_mutex = CU_MUTEX_INITIALISER;
static cu_dlink_t _tstate_chain = NULL;

void cuflowP_exeq_init_tstate(cuflow_tstate_t);

static void
_tstate_init(cuflow_tstate_t tstate)
{
    cuflowP_exeq_init_tstate(tstate);
    cu_mutex_lock(&_tstate_chain_mutex);
    if (_tstate_chain)
	cu_dlink_insert_before(_tstate_chain, cu_to(cu_dlink, tstate));
    else {
	cu_dlink_init_singleton(cu_to(cu_dlink, tstate));
	_tstate_chain = cu_to(cu_dlink, tstate);
    }
    cu_mutex_unlock(&_tstate_chain_mutex);
}

static void
_tstate_destruct(cuflow_tstate_t tstate)
{
    cu_mutex_lock(&_tstate_chain_mutex);
    if (cu_dlink_is_singleton(cu_to(cu_dlink, tstate)))
	_tstate_chain = NULL;
    else
	cu_dlink_erase(cu_to(cu_dlink, tstate));
    cu_mutex_unlock(&_tstate_chain_mutex);
}

CU_THREADLOCAL_DEF(cuflow_tstate, cuflowP_tstate, _tstate);

void
cuflowP_tstate_init()
{
    CU_THREADLOCAL_INIT(cuflow_tstate, cuflowP_tstate, _tstate);
}
