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

#include <cuex/str_algo.h>
#include <cuex/oprdefs.h>
#include <cuex/opn.h>
#include <cu/str.h>
#include <cu/idr.h>
#include <cu/test.h>

void
test_idrjoin()
{
    cu_idr_t e0 = cu_idr_by_cstr("kitchen");
    cu_idr_t e1 = cu_idr_by_cstr("sink");
    cu_idr_t e2 = cu_idr_by_cstr("orchestra");
    cuex_t e01 = cuex_o2_pair(e0, e1);
    cuex_t e012 = cuex_o2_pair(e01, e2);
    char const *cstr;
    cu_str_t str;

    cstr = cuex_left_idrjoin_cstr(CUEX_O2_PAIR, e0, ":");
    cu_debug_assert(!strcmp(cstr, "kitchen"));
    cstr = cuex_left_idrjoin_cstr(CUEX_O2_PAIR, e01, "::");
    cu_debug_assert(!strcmp(cstr, "kitchen::sink"));
    cstr = cuex_left_idrjoin_cstr(CUEX_O2_PAIR, e012, "");
    cu_debug_assert(!strcmp(cstr, "kitchensinkorchestra"));
    str = cuex_left_idrjoin_str(CUEX_O2_PAIR, e012, ".");
    cu_debug_assert(!cu_str_cmp_cstr(str, "kitchen.sink.orchestra"));
}

int
main()
{
    cu_init();
    test_idrjoin();
    return 2*!!cu_test_bug_count();
}
