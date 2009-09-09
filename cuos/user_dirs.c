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

#include <cuos/user_dirs.h>
#include <cuos/path.h>
#include <cuos/dirpile.h>
#include <cu/thread.h>
#include <cu/str.h>
#include <atomic_ops.h>

cu_dlog_def(_file, "dtag=cuos.user_dirs");

static AO_t _init_done = 0; /* 1 for *_home and 2 for also *_dirs */
static pthread_mutex_t _init_mutex = CU_MUTEX_INITIALISER;

/* From the variables or fixed defaults. */
static struct cu_str_s _user_config_home;
static struct cu_str_s _user_data_home;
static struct cu_str_s _user_cache_home;

void cuos_reset_user_dirs(void) { AO_store(&_init_done, 0); }

static void
_init_home(cu_str_t dir_out, char const *var, char const *home_default)
{
    char const *dir;
    if ((dir = getenv(var)))
	cu_str_init_static_cstr(dir_out, dir);
    else if ((dir = getenv("HOME"))) {
	cu_str_t abs_dir = cuos_path_join_2cstr(dir, home_default);
	cu_str_init_copy(dir_out, abs_dir);
    }
    else {
	/* Can we make up a sensible default? */
	cu_errf("Could not determine user directory as "
		"neither $HOME nor $%s is set.", var);
	exit(69); /* EX_UNAVAILABLE */
    }
}

static void
_init_1_inner(void)
{
    _init_home(&_user_config_home, "XDG_CONFIG_HOME", ".config");
    _init_home(&_user_data_home, "XDG_DATA_HOME", ".local/share");
    _init_home(&_user_cache_home, "XDG_CACHE_HOME", ".cache");
}

static void
_init_1(void)
{
    cu_mutex_lock(&_init_mutex);
    if (_init_done < 1) {
	_init_1_inner();
	AO_store_release_write(&_init_done, 1);
    }
    cu_mutex_unlock(&_init_mutex);
}

#define INIT_1() if (AO_load_acquire_read(&_init_done) < 1) _init_1()

cu_str_t
cuos_user_config_home(void)
{
    INIT_1();
    return &_user_config_home;
}

cu_str_t
cuos_user_data_home(void)
{
    INIT_1();
    return &_user_data_home;
}

cu_str_t
cuos_user_cache_home(void)
{
    INIT_1();
    return &_user_cache_home;
}

cu_str_t
cuos_user_config_path(char const *pkg_name, cu_str_t file)
{
    return cuos_path_cat3_scs(cuos_user_config_home(), pkg_name, file);
}

cu_str_t
cuos_user_data_path(char const *pkg_name, cu_str_t file)
{
    return cuos_path_cat3_scs(cuos_user_data_home(), pkg_name, file);
}

cu_str_t
cuos_user_cache_path(char const *pkg_name, cu_str_t file)
{
    return cuos_path_cat3_scs(cuos_user_cache_home(), pkg_name, file);
}


void
cuosP_pkg_user_dirs_ensure_init(cuos_pkg_user_dirs_t udirs)
{
    static char const *dirs_vars[] = {"XDG_CONFIG_DIRS", "XDG_DATA_DIRS"};
    static char const *home_vars[] = {"XDG_CONFIG_HOME", "XDG_DATA_HOME",
				      "XDG_CACHE_HOME"};
    static cu_str_t home_dirs[] = { &_user_config_home, &_user_data_home,
				    &_user_cache_home };

    cu_mutex_lock(&udirs->init_mutex);
    if (!udirs->init_done) {
	int i;
	char const *pkg_name = udirs->pkg_name;
	size_t var_prefix_length = strlen(udirs->var_prefix);
	char *var;

	/* Make sure _user_config_home and _user_data_home are set.  */
	INIT_1();
	cu_dlogf(_file, "Initialising user dirs for %s.", pkg_name);

	/* Prepare variable name with prefix. */
	var = cu_salloc(var_prefix_length + 13);
	strcpy(var, udirs->var_prefix);

	/* Initialise primary locations, udirs->homes. */
	for (i = 0; i < 3; ++i) {
	    char const *cstr;

	    strcpy(var + var_prefix_length, home_vars[i] + 3);
	    if ((cstr = getenv(var))) {	/* Use ${FOO_*_HOME} if set. */
		cu_dlogf(_file, "Using $%s.", var);
		udirs->homes[i] = cu_str_new_cstr(cstr);
	    }
	    else {			/* Else use XDG default. */
		cu_str_t dir = cuos_path_cat2_sc(home_dirs[i], pkg_name);
		cu_dlogf(_file, "Using $%s or default.", home_vars[i]);
		udirs->homes[i] = dir;
	    }
	}

	/* Initialise directories to search. */
	for (i = 0; i < 2; ++i) {
	    cuos_dirpile_t pile;

	    /* Initialise a new pile. */
	    pile = &udirs->dirs[i];
	    cuos_dirpile_init(pile);
	    cuos_dirpile_append(pile, udirs->homes[i]);

	    /* Add $FOO_CONFIG_DIRS or $FOO_DATA_DIRS if set, else add
	     * $XDG_CONFIG_DIRS or $XDG_DATA_DIRS if set. */
	    strcpy(var + var_prefix_length, dirs_vars[i] + 3);
	    if (getenv(var)) {
		cu_dlogf(_file, "Using $%s.", var);
		cuos_dirpile_insert_env(pile, cu_false, var, NULL);
	    }
	    else {
#ifndef CU_NDEBUG
		if (getenv(dirs_vars[i]))
		    cu_dlogf(_file, "Using $%s.", dirs_vars[i]);
#endif
		cuos_dirpile_insert_env(pile, cu_false, dirs_vars[i],
					cu_str_new_cstr(pkg_name));
	    }
	}

	/* If installdirs are provided, add package files as fallback. */
	if (udirs->installdirs) {
	    cu_str_t dir;
	    char const *sysconfdir, *datadir;

	    /* Check ${sysconfdir}/xdg/foo and ${sysconfdir}/foo. */
	    sysconfdir = udirs->installdirs[CU_INSTALLDIR_SYSCONFDIR].dir;
	    dir = cuos_path_cat3_ccc(sysconfdir, "xdg", pkg_name);
	    cuos_dirpile_append(&udirs->dirs[CUOS_USER_CONFIG], dir);
	    dir = cuos_path_cat2_cc(sysconfdir, pkg_name);
	    cuos_dirpile_append(&udirs->dirs[CUOS_USER_CONFIG], dir);

	    /* Check ${datadir}/foo. */
	    datadir = udirs->installdirs[CU_INSTALLDIR_DATADIR].dir;
	    dir = cuos_path_cat2_cc(datadir, pkg_name);
	    cuos_dirpile_append(&udirs->dirs[CUOS_USER_DATA], dir);
	}
	/* If installdirs is NULL, use the XDG standard directories. */
	else {
	    cu_str_t dir;
	    dir = cuos_path_cat2_cc("/etc/xdg", pkg_name);
	    cuos_dirpile_append(&udirs->dirs[CUOS_USER_CONFIG], dir);
	    dir = cuos_path_cat2_cc("/usr/local", pkg_name);
	    cuos_dirpile_append(&udirs->dirs[CUOS_USER_DATA], dir);
	    dir = cuos_path_cat2_cc("/usr", pkg_name);
	    cuos_dirpile_append(&udirs->dirs[CUOS_USER_DATA], dir);
	}

	AO_store_release_write(&udirs->init_done, 1);
    }
    cu_mutex_unlock(&udirs->init_mutex);
}

cu_str_t
cuos_pkg_user_config_dir(cuos_pkg_user_dirs_t udirs)
{
    cuos_pkg_user_dirs_ensure_init(udirs);
    return udirs->homes[CUOS_USER_CONFIG];
}

cu_str_t
cuos_pkg_user_data_dir(cuos_pkg_user_dirs_t udirs)
{
    cuos_pkg_user_dirs_ensure_init(udirs);
    return udirs->homes[CUOS_USER_DATA];
}

cu_str_t
cuos_pkg_user_cache_dir(cuos_pkg_user_dirs_t udirs)
{
    cuos_pkg_user_dirs_ensure_init(udirs);
    return udirs->homes[CUOS_USER_CACHE];
}

cu_str_t
cuos_pkg_user_config_path(cuos_pkg_user_dirs_t udirs, cu_str_t path)
{
    cuos_pkg_user_dirs_ensure_init(udirs);
    return cuos_path_cat2(udirs->homes[CUOS_USER_CONFIG], path);
}

cu_str_t
cuos_pkg_user_data_path(cuos_pkg_user_dirs_t udirs, cu_str_t path)
{
    cuos_pkg_user_dirs_ensure_init(udirs);
    return cuos_path_cat2(udirs->homes[CUOS_USER_DATA], path);
}

cu_str_t
cuos_pkg_user_cache_path(cuos_pkg_user_dirs_t udirs, cu_str_t path)
{
    cuos_pkg_user_dirs_ensure_init(udirs);
    return cuos_path_cat2(udirs->homes[CUOS_USER_CACHE], path);
}

cu_str_t
cuos_pkg_user_config_search(cuos_pkg_user_dirs_t udirs, cu_str_t path)
{
    cuos_pkg_user_dirs_ensure_init(udirs);
    return cuos_dirpile_first_match(&udirs->dirs[CUOS_USER_CONFIG], path);
}

cu_str_t
cuos_pkg_user_data_search(cuos_pkg_user_dirs_t udirs, cu_str_t path)
{
    cuos_pkg_user_dirs_ensure_init(udirs);
    return cuos_dirpile_first_match(&udirs->dirs[CUOS_USER_CONFIG], path);
}
