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
#include <cufo/sink.h>
#include <cutext/sink.h>
#include <cu/logging.h>
#include <cu/test.h>
#include <cu/wstring.h>
#include <cu/str.h>
#include <cu/sref.h>

struct cu_log_facility facility = {
    .origin = CU_LOG_USER,
    .severity = CU_LOG_ERROR,
};

void
print_page(cufo_stream_t fos)
{
    int i, j;
    cu_wstring_t wstring;
    cu_str_t str;
    cu_wchar_t *wcarr;

    cufo_entera(fos, cufoT_title, cufoA_id("main-title"));
    cufo_puts(fos, "Test Output of libcufo Formatter\n");
    cufo_leaveln(fos, cufoT_title);

    cufo_entera(fos, cufoT_para, cufoA_class("test-cls"));
    cufo_printf(fos, "Hello%(ld/sup) %<mellow%> world.\n", -52L, cufoT_italic);
    cufo_print_wstring(fos, CU_WSTRING_C("Hello wide world.\n"));
    cufo_leaveln(fos, cufoT_para);

    cufo_enter(fos, cufoT_codepre);
    for (i = 1; i < 100; i *= 2)
	for (j = 1; j < 10; j *= 2) {
	    double x = i/(double)j;
	    cufo_printf(fos,
			"%(d/sup)%<X%>%(d/sub) = %3d / %-2d = %4.2g = %5.2lf\n",
			i, cufoT_italic, j, i, j, x, x);
	}
    cufo_printf(fos, "%c %jc (alpha) < > &\n", '*', (cu_wint_t)0x3b1);
    for (i = 0x2200; i < 0x2210; ++i)
	cufo_printf(fos, "%#x '%jc' %s", i, (cu_wint_t)i,
		    i % 4 == 3? "\n" : "| ");
    cufo_leaveln(fos, cufoT_codepre);

    cufo_logf(fos, &facility, "This is a %s message.", "log");
    cufo_logf(fos, &facility, "%: This is a log message with a location %d.",
	      cu_sref_new_cstr("example.csv", 34, -1), 88);
    for (j = 0; j < 4; ++j) {
	i = (j + 1)*(j + 7)*(j + 1) % 10000;
	cufo_printf(fos, "Int format test: "
		    "%4hd = %4d = %+4ld = %-4hd = %d = %+ld = %#lx\n",
		    (short)i, i, (long)i, (short)i, i, (long)i, (long)i);
    }

    wstring = CU_WSTRING_C_O("\"quoted\" and \\backslashed.\tΓρεεκ.\n");
    str = cu_str_onew_cstr("\"quoted\" and \\backslashed.\tΓρεεκ.\n");
    cufo_printf(fos, "%%(wstring/r) format: %(wstring/r)\n", wstring);
    cufo_printf(fos, "%%! wstring format:   %!\n", wstring);
    cufo_printf(fos, "%%(str/r) format: %(str/r)\n", str);
    cufo_printf(fos, "%%! str format:   %!\n", str);

    cufo_printf(fos, "\nCharacter array tests:\n");
    wcarr = cu_wstring_array(CU_WSTRING_C("the string\0"));
    cufo_printf(fos, "%%js format:    |%js|\n", wcarr);
    cufo_printf(fos, "%%5js format:   |%5js|\n", wcarr);
    cufo_printf(fos, "%%-20js format: |%-20js|\n", wcarr);
    cufo_printf(fos, "%%20js format:  |%20js|\n", wcarr);
    cufo_putc(fos, '\n');
}

void
test_strip_target()
{
    cufo_stream_t fos;
    cu_str_t str;
    cu_wstring_t wstr, wstrp;

    fos = cufo_open_strip_str();
    cufo_printf(fos, "%c %03d %x %%", 'C', 79, 0x3219);
    str = cu_unbox_ptr(cu_str_t, cufo_close(fos));
    cu_test_assert(cu_str_cmp_cstr(str, "C 079 3219 %") == 0);

    fos = cufo_open_strip_wstring();
    cu_test_assert(fos);
    print_page(fos);
    wstr = cu_unbox_ptr(cu_wstring_t, cufo_close(fos));

    fos = cufo_open_strip_str();
    cu_test_assert(fos);
    print_page(fos);
    str = cu_unbox_ptr(cu_str_t, cufo_close(fos));
    wstrp = cu_wstring_of_chararr(cu_str_charr(str), cu_str_size(str));

    cu_test_assert(cu_wstring_cmp(wstr, wstrp) == 0);
}

void
test_text_target()
{
    int i, j;
    cufo_stream_t fos;
    cu_str_t str;
    cu_wstring_t wstr, wstrp;

    fos = cufo_open_text_str(NULL);
    cufo_printf(fos, "%c %03d %x", 'C', 79, 0x3219);
    str = cu_unbox_ptr(cu_str_t, cufo_close(fos));
    cu_test_assert(cu_str_cmp_cstr(str, "C 079 3219") == 0);

    fos = cufo_open_text_wstring(NULL);
    cu_test_assert(fos);
    print_page(fos);
    wstr = cu_unbox_ptr(cu_wstring_t, cufo_close(fos));

    fos = cufo_open_text_str(NULL);
    cu_test_assert(fos);
    print_page(fos);
    str = cu_unbox_ptr(cu_str_t, cufo_close(fos));
    wstrp = cu_wstring_of_chararr(cu_str_charr(str), cu_str_size(str));

    fos = cufo_open_auto_fd(1, cu_false);
    cufo_enter(fos, cufoT_codepre);
    for (j = 0; j < 16; ++j) {
	if (j % 2) {
	    for (i = 0; i < 40; ++i)
		cufo_printf(fos, "%02d; ", i);
	} else {
	    char const *s[] = {"w/o", "ra", "jar", " "};
	    int sp = 1;
	    for (i = 0; i < 80; ++i) {
		int j = lrand48() % (sizeof(s)/sizeof(s[0]) - sp);
		cufo_puts(fos, s[j]);
		cu_test_assert(cufo_stream_lastchar(fos)
			       == s[j][strlen(s[j])-1]);
		sp = j == sizeof(s)/sizeof(s[0]) - 1;
	    }
	}
	cufo_putc(fos, '\n');
	if (j < 8)
	    cufo_enter(fos, cufoT_indent);
	else
	    cufo_leave(fos, cufoT_indent);
    }
    cufo_putc(fos, '\n');
    cufo_leave(fos, cufoT_codepre);
    cufo_close(fos);

    cu_test_assert(cu_wstring_cmp(wstr, wstrp) == 0);
}

void
test_xml_target()
{
    cufo_stream_t fos;
    cutext_sink_t sink;

    fprintf(stderr, "UTF-8 XML sink structure:\n");
    sink = cutext_sink_fopen("UTF-8", "tmp.cufo_stream_t0.xml");
    cu_test_assert(sink != NULL);
    sink = cufo_sink_new_xml(sink);
    cutext_sink_debug_dump(sink);
    fos = cufo_open_sink(sink);
    print_page(fos);
    cufo_close(fos);

    fprintf(stderr, "UTF-16 XML sink structure:\n");
    sink = cutext_sink_fopen("UTF-16", "tmp.cufo_stream_t0_utf16.xml");
    cu_test_assert(sink != NULL);
    sink = cufo_sink_new_xml(sink);
    cutext_sink_debug_dump(sink);
    fos = cufo_open_sink(sink);
    print_page(fos);
    cufo_close(fos);
}

int
main()
{
    cufo_stream_t fos;
    cufo_init();

    fos = cufo_open_auto_fd(1, cu_false);
    cu_test_assert(fos);
    print_page(fos);
    cufo_close(fos);

    fos = cufo_open_text_file("UTF-8", NULL, "tmp.cufo_stream_t0.utf8");
    cu_test_assert(fos);
    print_page(fos);
    cufo_close(fos);

    fos = cufo_open_text_file("UTF-32", NULL, "tmp.cufo_stream_t0.utf32");
    cu_test_assert(fos);
    print_page(fos);
    cufo_close(fos);

    test_strip_target();
    test_text_target();
    test_xml_target();

    return 2*!!cu_test_bug_count();
}
