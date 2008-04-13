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
#include <cufo/attrdefs.h>
#include <cuos/dsink.h>
#include <cu/test.h>
#include <cu/wstring.h>
#include <cu/str.h>

void
print_page(cufo_stream_t fos)
{
    int i, j;
    cufo_entera(fos, cufoT_title, cufoA_id("main-title"));
    cufo_puts(fos, "Test Output of libcufo Formatter\n");
    cufo_leaveln(fos, cufoT_title);

    cufo_entera(fos, cufoT_para, cufoA_class("test-cls"));
    cufo_puts(fos, "Hello world.\n");
    cufo_print_wstring(fos, CU_WSTRING_C("Hello wide world.\n"));
    cufo_leaveln(fos, cufoT_para);

    cufo_enter(fos, cufoT_codepre);
    for (i = 1; i < 100; i *= 2)
	for (j = 1; j < 10; j *= 2) {
	    double x = i/(double)j;
	    cufo_printf(fos, "%3d / %-2d = %4.2g = %5.2lf\n", i, j, x, x);
	}
    cufo_printf(fos, "%c %lc (alpha) < > &\n", '*', (cu_wint_t)0x3b1);
    for (i = 0x2200; i < 0x2210; ++i)
	cufo_printf(fos, "%#x '%lc' %s", i, (cu_wint_t)i,
		    i % 4 == 3? "\n" : "| ");
    cufo_leaveln(fos, cufoT_codepre);
}

void
test_str_target()
{
    cufo_stream_t fos = cufo_open_strip_str();
    cu_str_t str;
    cu_wstring_t wstr, wstrp;

    cufo_printf(fos, "%c %03d %x", 'C', 79, 0x3219);
    str = cufo_close(fos);
    cu_test_assert(cu_str_cmp_cstr(str, "C 079 3219") == 0);

    fos = cufo_open_strip_wstring();
    cu_test_assert(fos);
    print_page(fos);
    wstr = cufo_close(fos);

    fos = cufo_open_strip_str();
    cu_test_assert(fos);
    print_page(fos);
    str = cufo_close(fos);
    wstrp = cu_wstring_of_chararr(cu_str_charr(str), cu_str_size(str));

    cu_test_assert(cu_wstring_cmp(wstr, wstrp) == 0);
}

void
test_xml_target()
{
    cufo_stream_t fos;
    cu_dsink_t sink;
    sink = cuos_dsink_open("tmp.cufo_stream_t0.xml");
    cu_test_assert(sink != NULL);
    fos = cufo_open_xmldirect("UTF-8", sink);
    print_page(fos);
    cufo_close(fos);
}

int
main()
{
    cufo_stream_t fos;
    cufo_init();

    fos = cufo_open_strip_fd(NULL, 1);
    cu_test_assert(fos);
    print_page(fos);
    cufo_close(fos);

    fos = cufo_open_strip_file(NULL, "tmp.cufo_stream_t0.utf8");
    cu_test_assert(fos);
    print_page(fos);
    cufo_close(fos);

    fos = cufo_open_strip_file("UTF-32", "tmp.cufo_stream_t0.utf32");
    cu_test_assert(fos);
    print_page(fos);
    cufo_close(fos);

    test_str_target();
    test_xml_target();

    return 2*!!cu_test_bug_count();
}
