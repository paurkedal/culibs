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

#include <cu/test.h>
#include <cuflow/wind.h>
#include <cuflow/except.h>
#include <stdio.h>

cuflow_xc_extern(runtime_error, cu_prot(void, char *what),
	      (char *what;));

cuflow_xc_define(runtime_error, cu_prot(void, char *what))
{
    self->what = what;
}

void g(FILE *file, int i)
{
    if (i == 't')
	cuflow_throw(runtime_error, "Testing exceptions.");
}

int f(int i)
{
    FILE *file = fopen("/etc/hosts", "r");
    long filepos = -1;
    fprintf(stderr, "Entering f.\n");
    cuflow_wind_gotos();
    g(file, i);
    fclose(file);
    fprintf(stderr, "Returning from f.");
    cuflow_wind_return(0);
rewind:
    fprintf(stderr, "Enter rewind label.\n");
    file = fopen("/etc/hosts", "r");
    cu_test_assert(filepos != -1);
    fseek(file, filepos, SEEK_SET);
    cuflow_rewind_continue;
unwind:
    fprintf(stderr, "Enter unwind label.\n");
    filepos = ftell(file);
    fclose(file);
    cuflow_unwind_continue;
except:
    fprintf(stderr, "Enter except label.\n");
    /* A frame where an exception occured can not be rewound, so there
     * is no need to store the file position. */
    fclose(file);
    if (cuflow_xc_is(runtime_error)) {
	cuflow_xc_dcln(runtime_error);
	fprintf(stderr, "runtime_error: %s\n", xc->what);
	cuflow_wind_return(-1);
    }
    /* Also a cuflow_wind_cleanup exception is thrown when all
     * continuations containing a snapshot of this frame have been
     * garbage collected.  We don't need to catch it, since we already
     * did the cleanup above. */
    cuflow_except_continue;
}


int
main()
{
    cu_init();
    f('t');
    f('t');
    return 0;
}
