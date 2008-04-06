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
#include <cufo/tagdefs.h>
#include <cu/test.h>
#include <cu/wstring.h>

void
print_page(cufo_stream_t fos)
{
    int i, j;
    cufo_enter(fos, cufo_b_title);
    cufo_puts(fos, "Test Output of libcufo Formatter\n");
    cufo_leaveln(fos, cufo_b_title);

    cufo_enter(fos, cufo_b_para);
    cufo_puts(fos, "Hello world.\n");
    cufo_print_wstring(fos, CU_WSTRING_C("Hello wide world.\n"));
    cufo_leaveln(fos, cufo_b_para);

    cufo_enter(fos, cufo_b_codepre);
    for (i = 1; i < 100; i *= 2)
	for (j = 1; j < 10; j *= 2) {
	    double x = i/(double)j;
	    cufo_printf(fos, "%3d / %-2d = %4.2g = %5.2lf\n", i, j, x, x);
	}
    cufo_printf(fos, "%c %lc (alpha)\n", '*', (cu_wint_t)0x3b1);
    for (i = 0x2200; i < 0x2210; ++i)
	cufo_printf(fos, "%#x '%lc' %s", i, (cu_wint_t)i,
		    i % 4 == 3? "\n" : "| ");
    cufo_leaveln(fos, cufo_b_codepre);
}

int
main()
{
    cufo_stream_t fos;
    cufo_init();
    fos = cufo_open_fd(1, NULL);
    print_page(fos);
    cufo_close(fos);
    return 2*!!cu_test_bug_count();
}
