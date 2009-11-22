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

#include <cucon/rumap.h>
#include <cu/test.h>
#include <string.h>

void
put(cucon_rumap_t M, char const *s, char *v)
{
    cucon_rumap_t Mp = cucon_rumap_mref_by_char_arr(M, s, strlen(s));
    cucon_rumap_set_value(Mp, v);
}

void
check(cucon_rumap_t M, char const *s, char *v)
{
    char *vp;
    cucon_rumap_t Mp = cucon_rumap_mref_by_char_arr(M, s, strlen(s));
    cu_test_assert(Mp);
    vp = cucon_rumap_value(Mp);
    cu_test_assert(vp);
    cu_test_assert(strcmp(v, vp) == 0);
}

void
init_a(cucon_rumap_t M)
{
    cucon_rumap_init(M);
    put(M, "aa", "aa left");
    put(M, "ab", "ab left");
    put(M, "abcd", "abcd left");
    put(M, "z", "z left");
}

void
init_b(cucon_rumap_t M)
{
    cucon_rumap_init(M);
    put(M, "aa", "aa right");
    put(M, "abcd", "abcd right");
    put(M, "az", "az right");
}

void
test_union()
{
    struct cucon_rumap L, R;
    size_t overlap;

    init_a(&L);
    init_b(&R);

    overlap = cucon_rumap_assign_left_union(&L, &R);
    cu_test_assert_size_eq(overlap, 2);
    check(&L, "aa", "aa left");
    check(&L, "ab", "ab left");
    check(&L, "abcd", "abcd left");
    check(&L, "z", "z left");
    check(&L, "az", "az right");

    init_a(&L);
    init_b(&R);

    overlap = cucon_rumap_assign_right_union(&L, &R);
    cu_test_assert_size_eq(overlap, 2);
    check(&L, "aa", "aa right");
    check(&L, "ab", "ab left");
    check(&L, "abcd", "abcd right");
    check(&L, "z", "z left");
    check(&L, "az", "az right");
}

int
main()
{
    cu_init();
    test_union();
    return 2*!!cu_test_bug_count();
}
