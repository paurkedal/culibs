/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#include <cu/dsink.h>
#include <cu/diag.h>

cu_word_t
cu_dsink_control(cu_dsink_t sink, int op, ...)
{
    cu_word_t res;
    va_list va;
    va_start(va, op);
    res = (*sink->control)(sink, op, va);
    va_end(va);
    return res;
}

void
cu_dsink_assert_clogfree(cu_dsink_t sink)
{
    cu_word_t st = cu_dsink_control(sink, CU_DSINK_FN_IS_CLOGFREE);
    if (st != CU_DSINK_ST_SUCCESS)
	cu_bugf("Caller expected a clog-free sink, but sink does not "
		"guarante to be clog-free.");
}


#define DCOUNTSINK(sink) cu_from(cu_dcountsink, cu_dsink, sink)

size_t
dcountsink_write(cu_dsink_t sink, void const *arr, size_t len)
{
    DCOUNTSINK(sink)->count += len;
    return len;
}

cu_word_t
dcountsink_control(cu_dsink_t sink, int fn, va_list va)
{
    return CU_DSINK_ST_UNIMPL;
}

void
cu_dcountsink_init(cu_dcountsink_t sink)
{
    cu_dsink_init(cu_to(cu_dsink, sink), dcountsink_control, dcountsink_write);
    sink->count = 0;
}
