/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <urkedal@nbi.dk>
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

#include <cu/location.h>
#include <cu/test.h>
#include <cu/str.h>

int
main()
{
    cu_init();

    struct cu_locorigin origin;
    struct cu_locbound b0, b1, b2, b3;
    struct cu_location l0, l1, l2, l3;

    cu_locorigin_init(&origin, cu_str_new_cstr("TEST"), 8);
    cu_locbound_init(&b0, &origin, 1, 2);
    cu_locbound_init(&b1, &origin, 1, 2);

    cu_test_assert(cu_locbound_cmp(&b0, &b1) == 0);
    cu_locbound_skip_columns(&b1, 5);
    cu_test_assert(cu_locbound_cmp(&b0, &b1) < 0);
    cu_locbound_put_tab(&b1);
    cu_test_assert(cu_locbound_column(&b1) == 8);
    cu_locbound_put_tab(&b1);
    cu_test_assert(cu_locbound_column(&b1) == 16);
    cu_test_assert(cu_locbound_line(&b1) == 1);
    cu_locbound_put_newline(&b1);
    cu_test_assert(cu_locbound_column(&b1) == 0);
    cu_test_assert(cu_locbound_line(&b1) == 2);

    cu_location_init_range(&l0, &b0, &b1);
    cu_locbound_init(&b2, &origin, 8, 2);
    cu_locbound_init(&b3, &origin, 10, 52);
    cu_location_init_range(&l1, &b2, &b3);
    cu_location_init_cover(&l2, &l0, &l1);
    cu_location_init_cover(&l3, &l1, &l0);
    cu_test_assert(cu_location_lb_line(&l2) == 1);
    cu_test_assert(cu_location_lb_column(&l2) == 2);
    cu_test_assert(cu_location_ub_line(&l2) == 10);
    cu_test_assert(cu_location_ub_column(&l2) == 52);
    cu_test_assert(cu_location_cmp(&l2, &l3) == 0);
    cu_test_assert(cu_locbound_cmp(cu_location_lbound(&l2), &b0) == 0);
    cu_test_assert(cu_locbound_cmp(cu_location_ubound(&l2), &b3) == 0);

    return 2*!!cu_test_bug_count();
}
