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

#include <cuflow/errors.h>
#include <cuflow/except.h>
#include <stdio.h>

int
test()
{
    return test() + 1;
}

int
main()
{
    cu_init();
    cuflow_wind_gotos();
    printf("Warning: Unless you have set a reasonably low limit to the"
	   "stack size,\nthis test may use extensive resources and "
	   "freeze your machine for\nsome time.\n\n"
	   "Are you sure you want to run the test now? (type \"yes\" to run) ");
    fflush(stdout);
    if (fgetc(stdin) != 'y' || fgetc(stdin) != 'e' || fgetc(stdin) != 's' ||
	fgetc(stdin) != '\n')
	return 0;
    printf("Doing infinite recursive call.\n");
    test();
    fprintf(stderr, "Hmmm, no infinite recursion?\n");
    return 1;

unwind:
rewind:
    abort();
except:
    fprintf(stderr, "Stack overflow detected.\n");
    return 0;
}
