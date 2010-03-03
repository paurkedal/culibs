/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#include <cutext/lsource.h>
#include <cu/test.h>
#include <cu/size.h>
#include <cu/str.h>

static void
_dump_source(cutext_lsource_t lsrc, FILE *fp)
{
    for (;;) {
	size_t m = lrand48() % 32 + 1;
	size_t i, j, k;
	cu_wchar_t const *buf = cutext_lsource_look(lsrc, m, &k);
	if (k == 0)
	    return;
	for (j = 0; j < k; ++j) {
	    cu_wchar_t ch = buf[j];
	    i = sizeof(cu_wchar_t);
	    while (i > 0)
		fputc(0xff & (ch >> 8*(i--)), fp);
	}
	cu_test_assert(cutext_lsource_read(lsrc, NULL, m) == k);
    }
}

static void
_test(char const *coding, int i)
{
    struct cutext_lsource lsrc;
    FILE *fp;
    cu_str_t path_in, path_out;

    path_in = cu_str_new_fmt("cutext/lsource_t0_%s.txt", coding);
    path_out = cu_str_new_fmt("cutext/lsource_t0_%s_%d.txt", coding, i);
    cu_test_assert(
	cutext_lsource_init_fopen(&lsrc, i % 2? coding : NULL, path_in));
    fp = fopen(cu_str_to_cstr(path_out), "w");
    cu_test_assert(fp);
    _dump_source(&lsrc, fp);
    fclose(fp);
}

int
main()
{
    cu_init();

    _test("utf-8", 1);
    _test("utf-16", 1);
    _test("ucs-4", 1);
    _test("utf-8", 2);
    _test("utf-16", 2);
    _test("ucs-4", 2);

    return 2*!!cu_test_bug_count();
}
