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

#include <cutext/wctype.h>
#include <cutext/wccat.h>
#include <cu/test.h>
#include <ctype.h>
#include <stdio.h>

struct {
    char const *system_name;
    int (*system_pred)(int);
    char const *cutext_name;
    cu_bool_t (*cutext_pred)(cu_wint_t);
} tests[] = {
#define D(n) {"is"#n, is##n, "cutext_isw"#n, cutext_isw##n}
    D(alpha), D(cntrl), D(digit), D(punct), D(space),
    D(print), D(graph),
    D(alnum), D(upper), D(lower), D(xdigit),
    D(blank),
#undef D
};

int
main(int argc, char **argv)
{
    cu_wint_t ch;

    cutext_init();

    if (argc == 3) {
	cu_wint_t ch_s = strtol(argv[1], NULL, 16);
	cu_wint_t ch_e = strtol(argv[2], NULL, 16);
	for (ch = ch_s; ch <= ch_e; ++ch) {
	    int i;
	    printf("%6x", ch);
	    for (i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i)
		if ((*tests[i].cutext_pred)(ch)) {
		    fputc(' ', stdout);
		    fputs(tests[i].system_name + 2, stdout);
		}
	    fputc('\n', stdout);
	}
    }
    for (ch = 0; ch < 128; ++ch) {
	int i;
	for (i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
	    cu_bool_t t0 = !!tests[i].system_pred(ch);
	    cu_bool_t t1 = !!tests[i].cutext_pred(ch);
	    if (t0 != t1)
		cu_test_bugf("%s(0x%x) = %d != %d = %s(0x%x); cat = %d\n",
			     tests[i].system_name, ch, t0,
			     t1, tests[i].cutext_name, ch,
			     cutext_wchar_wccat(ch));
	}
    }
    return 2*!!cu_test_bug_count();
}
