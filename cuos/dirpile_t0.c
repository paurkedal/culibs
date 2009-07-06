/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuos/dirpile.h>
#include <cuos/fs.h>
#include <cu/str.h>
#include <cu/test.h>

void
test_dirpile()
{
    cuos_dirpile_t pile = cuos_dirpile_new();
    cu_str_t path_a = cu_str_new_cstr("tmp.dirpile_t0/a");
    cu_str_t path_b = cu_str_new_cstr("tmp.dirpile_t0/b");
    cu_str_t path_c = cu_str_new_cstr("tmp.dirpile_t0/c");
    cu_str_t path_x = cu_str_new_cstr("tmp.dirplie_t0/x");
    cu_str_t path;
    cuos_mkdir_rec(path_a, 0755);
    cuos_mkdir_rec(path_b, 0755);
    cuos_mkdir_rec(path_c, 0755);
    cu_test_assert(!cuos_dirpile_insert(pile, path_x, cu_true));
    cu_test_assert(cuos_dirpile_insert(pile, path_b, cu_true));
    cu_test_assert(cuos_dirpile_insert(pile, path_a, cu_true));
    cu_test_assert(cuos_dirpile_insert(pile, path_c, cu_false));
    fclose(fopen("tmp.dirpile_t0/a/f", "w"));
    fclose(fopen("tmp.dirpile_t0/c/f", "w"));
    cu_test_assert(!cuos_dirpile_first_match(pile, cu_str_new_cstr("g")));
    path = cuos_dirpile_first_match(pile, cu_str_new_cstr("f"));
    cu_test_assert(cu_str_cmp_cstr(path, "tmp.dirpile_t0/a/f") == 0);
    cuos_remove_rec(cu_str_new_cstr("tmp.dirpile_t0"));
}

int
main()
{
    cuos_init();
    test_dirpile();
    return 2*!!cu_test_bug_count();
}
