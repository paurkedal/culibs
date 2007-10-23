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

#include <cu/test.h>
#include <cu/diag.h>
#include <cu/debug.h>
#include <stdio.h>
#include <stdlib.h>

static AO_t cuP_test_fail_count = 0;
static AO_t cuP_test_max_bug_count = 0;
static cu_test_bugaction_t cuP_test_bugaction = cu_test_bugaction_abort;

void
cu_test_on_bug(cu_test_bugaction_t action, unsigned long max_bug_count)
{
    cuP_test_bugaction = action;
    cuP_test_max_bug_count = max_bug_count;
}

int
cu_test_bug_count()
{
    return AO_load_acquire_read(&cuP_test_fail_count);
}

void
cuP_test_vbugf(char const *file, int line, char const *msg, va_list va)
{
    int bug_count = AO_fetch_and_add1(&cuP_test_fail_count) + 1;
    fprintf(stderr, "%s:%d: ", file, line);
    cu_vfprintf(stderr, msg, va);
    fputc('\n', stderr);
    if (bug_count < cuP_test_max_bug_count)
	return;
    else if (cuP_test_bugaction == cu_test_bugaction_cont)
	return;
    if (cuP_test_max_bug_count > 1)
	fprintf(stderr, "*** Limit of %lu bugs reached.\n",
		(unsigned long)cuP_test_max_bug_count);
    switch (cuP_test_bugaction) {
	case cu_test_bugaction_abort:
	    abort();
	case cu_test_bugaction_exit:
	    exit(2);
	default:
	    cu_debug_unreachable();
    }
}

void
cuP_test_bugf(char const *file, int line, char const *msg, ...)
{
    va_list va;
    va_start(va, msg);
    cuP_test_vbugf(file, line, msg, va);
    va_end(va);
}
