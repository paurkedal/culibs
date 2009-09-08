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

#include <cuos/fs.h>
#include <cuos/file.h>
#include <cuos/path.h>
#include <cu/test.h>
#include <cu/str.h>

static void
_create_file(cu_str_t path)
{
    FILE *fh = cuos_fopen(path, "w");
    cu_test_assert(fh);
    fclose(fh);
}

static void
fs_tests()
{
    cu_str_t top_dir = cu_str_new_cstr("tmp.fs.d");
    cu_str_t file_0 = cuos_path_join_str_cstr(top_dir, "file-0");
    cu_str_t dir_0 = cuos_path_join_str_cstr(top_dir, "dir-0");
    cu_str_t file_01 = cuos_path_join_str_cstr(dir_0, "file-01");

    cu_test_assert(cuos_mkdir_rec(dir_0, 0777));
    _create_file(file_0);
    _create_file(file_01);

    cu_test_assert(cuos_have_dir(top_dir));
    cu_test_assert(cuos_have_dir(dir_0));
    cu_test_assert(cuos_have_file(file_0));
    cu_test_assert(cuos_have_file(file_01));

    cu_test_assert(cuos_remove_rec(top_dir));

    cu_test_assert(!cuos_have_dir(top_dir));
    cu_test_assert(!cuos_have_dir(dir_0));
    cu_test_assert(!cuos_have_file(file_0));
    cu_test_assert(!cuos_have_file(file_01));
}

int
main()
{
    cu_init();
    fs_tests();
    return 2*!!cu_test_bug_count();
}
