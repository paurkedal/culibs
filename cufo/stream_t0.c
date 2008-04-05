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

#include <cufo/stream.h>
#include <cu/test.h>
#include <cu/wstring.h>

int
main()
{
    int i, j;
    cufo_stream_t fos;
    cufo_init();
    fos = cufo_stream_new_fd(1, NULL);
    cufo_puts(fos, "Hello world.\n");
    cufo_print_wstring(fos, CU_WSTRING_C("Hello wide world.\n"));
    for (i = 1; i < 100; i *= 2)
	for (j = 1; j < 10; j *= 2) {
	    double x = i/(double)j;
	    cufo_printf(fos, "%3d / %-2d = %4.2g = %5.2lf\n", i, j, x, x);
	}
    cufo_printf(fos, "%c %lc (alpha)\n", '*', (cu_wint_t)0x3b1);
    for (i = 0x2200; i < 0x2210; ++i)
	cufo_printf(fos, "%#x '%lc' %s", i, (cu_wint_t)i,
		    i % 4 == 3? "\n" : "| ");
    cufo_flush(fos);
    return 2*!!cu_test_bug_count();
}
