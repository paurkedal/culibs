/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007--2009  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuflow/cached.h>
#include <cuflow/tstate.h>
#include <cuflow/workers.h>
#include <cu/test.h>

cuflow_cached_sdecl(_plus, (int x; int y;), (int z;));

cuflow_cached_sdef(_plus)
{
    cuflow_cached_arg_res(_plus);
    printf("Called plus with arguments %d and %d.\n", arg->x, arg->y);
    res->z = arg->x + arg->y;
    cuflow_cached_set_gain(_plus, 10);
}

void
_test()
{
    _plus_promise_t promise;
    _plus_result_t result;

    cuflow_cached_call(_plus, (arg->x = 10; arg->y = 20;), &result);
    cu_test_assert(result->z == 30);

    cuflow_cached_sched_call(_plus, (arg->x = 10; arg->y = 20;), &promise);
    result = _plus_fulfill(promise);
    cu_test_assert(result->z == 30);
}

int main()
{
    cuflow_init();
    cuflow_workers_spawn(0);
    _test();
    return 2*!!cu_test_bug_count();
}
