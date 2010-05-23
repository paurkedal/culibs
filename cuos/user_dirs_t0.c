/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009--2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#include <cuos/user_dirs.h>
#include <cuos/dirpile.h>
#include <cuos/fs.h>
#include <cu/test.h>
#include <cu/str.h>
#include <cu/thread.h>
#include <stdio.h>
#include <stdlib.h>

#define _str cu_str_new_cstr
#define _cstr cu_str_to_cstr
#define SHOW_STR(expr) printf(#expr" = %s\n", _cstr(expr))

cu_clop_def(_show_pile_entry, cu_bool_t, cu_str_t path)
{
    printf("\t%s\n", cu_str_to_cstr(path));
    return cu_true;
}

static void
_show_pile(char const *name, cuos_dirpile_t pile)
{
    printf("    %s:\n", name);
    cuos_dirpile_iterA_top(pile, cu_clop_ref(_show_pile_entry));
}

extern struct cu_installdirs cuconfP_installdirs;

static void
_show_pkg_user_dirs(cu_bool_t use_instdirs)
{
    struct cuos_pkg_user_dirs udirs
	= CUOS_PKG_USER_DIRS_INITZ("libfoo", "LIBFOO",
				   use_instdirs? &cuconfP_installdirs : NULL);
    cuos_pkg_user_dirs_ensure_init(&udirs);
    _show_pile("config_dirs", &udirs.dirs[CUOS_USER_CONFIG]);
    _show_pile("data_dirs", &udirs.dirs[CUOS_USER_DATA]);
    printf("    a_config_path = %s\n"
	   "    a_data_path = %s\n"
	   "    a_cache_path = %s\n",
	   _cstr(cuos_pkg_user_config_path(&udirs, _str("bar.conf"))),
	   _cstr(cuos_pkg_user_data_path(&udirs, _str("bar.data"))),
	   _cstr(cuos_pkg_user_cache_path(&udirs, _str("bar.cache"))));
}

int
main()
{
    cuos_init();

    SHOW_STR(cuos_user_config_home());
    SHOW_STR(cuos_user_data_home());
    SHOW_STR(cuos_user_cache_home());

    SHOW_STR(cuos_user_config_path("foo", _str("bar.conf")));
    SHOW_STR(cuos_user_data_path("foo", _str("bar.data")));
    SHOW_STR(cuos_user_cache_path("foo", _str("bar.cache")));

    /* First use XDG_* variables. */

    cuos_mkdir_rec(_str("tmp.user_dirs/home/jdoe/.config/libfoo"), 0777);
    cuos_mkdir_rec(_str("tmp.user_dirs/home/jdoe/.local/share/libfoo"), 0777);
    cuos_mkdir_rec(_str("tmp.user_dirs/usr/local/etc/libfoo"), 0777);
    cuos_mkdir_rec(_str("tmp.user_dirs/usr/local/share/libfoo"), 0777);

    setenv("XDG_CONFIG_HOME", "tmp.user_dirs/home/jdoe/.config", 1);
    setenv("XDG_DATA_HOME",   "tmp.user_dirs/home/jdoe/.local/share", 1);
    setenv("XDG_CACHE_HOME",  "tmp.user_dirs/home/jdoe/.local/var/cache", 1);
    setenv("XDG_CONFIG_DIRS", "tmp.user_dirs/usr/local/etc", 1);
    setenv("XDG_DATA_DIRS",   "tmp.user_dirs/usr/local/share", 1);
    cuos_reset_user_dirs();

    printf("With ${XDG_*} set:\n");
    _show_pkg_user_dirs(cu_true);

    /* Override with LIBFOO_* variables. */

    cuos_mkdir_rec(_str("tmp.user_dirs/jdoe/foo/config"), 0777);
    cuos_mkdir_rec(_str("tmp.user_dirs/jdoe/foo/data"), 0777);
    cuos_mkdir_rec(_str("tmp.user_dirs/opt/foo/etc/foo"), 0777);
    cuos_mkdir_rec(_str("tmp.user_dirs/opt/foo/share/foo"), 0777);

    setenv("LIBFOO_CONFIG_HOME", "tmp.user_dirs/jdoe/foo/config", 1);
    setenv("LIBFOO_DATA_HOME", "tmp.user_dirs/jdoe/foo/data", 1);
    setenv("LIBFOO_CACHE_HOME", "tmp.user_dirs/jdoe/foo/cache", 1);
    setenv("LIBFOO_CONFIG_DIRS", "tmp.user_dirs/opt/foo/etc/foo", 1);
    setenv("LIBFOO_DATA_DIRS", "tmp.user_dirs/opt/foo/share/foo", 1);

    printf("With ${LIBFOO_*} set, as well:\n");
    _show_pkg_user_dirs(cu_true);

    unsetenv("XDG_CONFIG_HOME");
    unsetenv("XDG_DATA_HOME");
    unsetenv("XDG_CACHE_HOME");
    unsetenv("XDG_DATA_DIRS");
    unsetenv("XDG_CONFIG_DIRS");
    unsetenv("LIBFOO_CONFIG_HOME");
    unsetenv("LIBFOO_DATA_HOME");
    unsetenv("LIBFOO_CACHE_HOME");
    unsetenv("LIBFOO_CONFIG_DIRS");
    unsetenv("LIBFOO_DATA_DIRS");
    cu_installdirs_set(&cuconfP_installdirs, CU_INSTALLDIR_PREFIX,
		       "tmp.user_dirs/usr/local");
    cu_installdirs_reset(&cuconfP_installdirs);
    cu_installdirs_finish(&cuconfP_installdirs);
    cuos_reset_user_dirs();

    /* Finally without variables. */

    printf("Vars unset, with installdirs:\n");
    _show_pkg_user_dirs(cu_true);

    printf("Vars unset, without installdirs:\n");
    _show_pkg_user_dirs(cu_false);

    cuos_remove_rec(cu_str_new_cstr("tmp.user_dirs"));

    return 2*!!cu_test_bug_count();
}
