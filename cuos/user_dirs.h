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

#ifndef CUOS_USER_DIRS_H
#define CUOS_USER_DIRS_H

#include <cuos/dirpile.h>
#include <cu/installdirs.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cuos_user_dirs_h cuos/user_dirs.h: User Directories as Defined by XDG
 ** @{ \ingroup cuos_mod
 **
 ** This implements the
 ** <a href="http://standards.freedesktop.org/basedir-spec/latest/">XDG Base
 ** Directory Specification</a> with a few adjustments.
 **/

/** This functions forces re-probing of the user directories.  The may be
 ** useful if directories where created or the XDG_* environment variables
 ** where changed with \c setenv(3), \c unsetenv(3), etc. */
void cuos_reset_user_dirs(void);

/** The parent of the per-application user configuration directories.  This is
 ** the environment variable \c $XDG_CONFIG_HOME if set, otherwise
 ** <tt>$HOME/.config</tt>. */
cu_str_t cuos_user_config_home(void) CU_ATTR_PURE;

/** The parent of the per-application user data directories.  This is the
 ** environment variable \c $XDG_DATA_HOME if set, otherwise
 ** <tt>$HOME/.local/share</tt>. */
cu_str_t cuos_user_data_home(void) CU_ATTR_PURE;

/** The parent of per-application user cache directories.  This is the
 ** environment variable \c $XDG_CACHE_HOME if set, otherwise
 ** <tt>$HOME/.cache</tt>. */
cu_str_t cuos_user_cache_home(void) CU_ATTR_PURE;

cu_str_t cuos_user_config_path(char const *pkg_name, cu_str_t subpath);
cu_str_t cuos_user_data_path(char const *pkg_name, cu_str_t subpath);
cu_str_t cuos_user_cache_path(char const *pkg_name, cu_str_t subpath);

#define CUOS_USER_CONFIG	0
#define CUOS_USER_DATA		1
#define CUOS_USER_CACHE		2

/** A structure which holds package specific user directories for
 ** configuration, data, and cache, based on various environment variables and
 ** the package installation directories.  Use \ref CUOS_PKG_USER_DIRS_INITZ as
 ** a static initialiser. */
struct cuos_pkg_user_dirs
{
    char const *pkg_name, *var_prefix;
    cu_installdirs_t installdirs;
    AO_t init_done;
    pthread_mutex_t init_mutex;
    struct cuos_dirpile dirs[2];
    cu_str_t homes[3];
};

/** A static initialiser for \ref cuos_pkg_user_dirs to be used as
 ** \code
 ** struct cuos_pkg_user_dirs foo_user_dirs
 **     = CUOS_PKG_USER_DIRS_INITZ("foo", "FOO", foo_installdirs);
 ** \endcode
 ** after which <tt>cuos_pkg_user_config_dir(&foo_user_dirs)</tt> would give
 ** something like <tt>/home/jdoe/.config/foo</tt>.
 **
 ** This initialises part of the structure.  The rest is initialised on demand
 ** when calling the \c cuos_pkg_* functions.  The on-demand initalisation will
 ** look for environment variables starting with \a var_prefix, in particular
 ** if \a var_prefix is \c "FOO", then
 **   - <tt>$<i>FOO</i>_CONFIG_HOME</tt> is the primary user config directory,
 **   - <tt>$<i>FOO</i>_DATA_HOME</tt> is the primary user data directory, and
 **   - <tt>$<i>FOO</i>_CACHE_HOME</tt> is the primary user cache diretory
 **
 ** for this package.  If either of these are not set the subdirectory \a
 ** pkg_name under the corresponding XDG base directory is used instead.
 **
 ** Finally, \a foo_installdirs is used to set up additional paths for
 ** searching for package-installed files with \ref cuos_pkg_user_config_search
 ** and \ref cuos_pkg_user_data_search.
 ** You may pass \c NULL for foo_installdirs, but that means that if your
 ** package is installed under a prefix other than \c /usr/local or \c usr,
 ** installed files will not be found unless the user includes the location in
 ** the corresponding variables
 **   - <tt>$<i>FOO</i>_CONFIG_DIRS</tt> or <tt>$XDG_CONFIG_DIRS</tt>, and
 **   - <tt>$<i>FOO</i>_DATA_DIRS</tt> or <tt>$XDG_DATA_DIRS</tt>.
 **
 ** See \ref cuos_pkg_user_config_search and \ref cuos_pkg_user_data_search for
 ** the search order.
 **/
#define CUOS_PKG_USER_DIRS_INITZ(pkg_name, var_prefix, app_installdirs) \
    { pkg_name, var_prefix, app_installdirs, 0, CU_MUTEX_INITIALISER }

#ifndef CU_IN_DOXYGEN
void cuosP_pkg_user_dirs_ensure_init(cuos_pkg_user_dirs_t udirs);
#endif

/** Finish initialisation of \a udirs if not already done.  This is called by
 ** the other \c cuos_pkg_* functions, so you normally don't need to call it
 ** explicitely.  \a udirs must be statically initalised with \ref
 ** CUOS_PKG_USER_DIRS_INITZ in advance. */
CU_SINLINE void
cuos_pkg_user_dirs_ensure_init(cuos_pkg_user_dirs_t udirs)
{
    if (!AO_load_acquire_read(&udirs->init_done))
	cuosP_pkg_user_dirs_ensure_init(udirs);
}

/** The user config path from \a udirs. */
cu_str_t cuos_pkg_user_config_dir(cuos_pkg_user_dirs_t udirs);

/** The user data path from \a udirs. */
cu_str_t cuos_pkg_user_data_dir(cuos_pkg_user_dirs_t udirs);

/** The user cache path from \a udirs. */
cu_str_t cuos_pkg_user_cache_dir(cuos_pkg_user_dirs_t udirs);

/** Resolve a relative \a path under the config home of \a udirs. */
cu_str_t cuos_pkg_user_config_path(cuos_pkg_user_dirs_t udirs, cu_str_t path);

/** Reslove a relative \a path under the data home of \a udirs. */
cu_str_t cuos_pkg_user_data_path(cuos_pkg_user_dirs_t udirs, cu_str_t path);

/** Resolve a relative \a path under the cache home of \a udirs. */
cu_str_t cuos_pkg_user_cache_path(cuos_pkg_user_dirs_t udirs, cu_str_t path);

/** Locate \a path in the list of config search directories of \a udirs.  This
 ** returns the first existing match under
 **   - The primary config location as described in \ref
 **     CUOS_PKG_USER_DIRS_INITZ.
 **   - If <tt>$<i>FOO</i>_CONFIG_DIRS</tt> is set, any of this colon separated
 **     list of directories in order.
 **   - Else if <tt>$XDG_CONFIG_DIRS</tt> is set, any subdirectory \a pkg_name
 **     under that colon separated list in order.
 **   - Else if \a app_installdirs was passed to \ref CUOS_PKG_USER_DIRS_INITZ,
 **     then the \a pkg_name subdir under the \ref CU_INSTALLDIR_SYSCONFDIR
 **     location.
 **   - Else <tt>/etc/xdg/<i>pkg_name</i></tt>, then
 **     <tt>/etc/<i>pkg_name</i></tt>.
 **/
cu_str_t cuos_pkg_user_config_search(cuos_pkg_user_dirs_t udirs, cu_str_t path);

/** Locate \a path in the list of data search directories of \a udirs.  This
 ** returns the first existing match under
 **   - The primary config location as described in \ref
 **     CUOS_PKG_USER_DIRS_INITZ.
 **   - If <tt>$<i>FOO</i>_DATA_DIRS</tt> is set, any of this colon separated
 **     list of directories in order.
 **   - Else if <tt>$XDG_DATA_DIRS</tt> is set, any subdirectory \a pkg_name
 **     under that colon separated list in order.
 **   - Else if \a app_installdirs was passed to \ref CUOS_PKG_USER_DIRS_INITZ,
 **     then the \a pkg_name subdir under the \ref CU_INSTALLDIR_DATADIR
 **     location.
 **   - Else <tt>/usr/local/<i>pkg_name</i></tt> then
 **     <tt>/usr/<i>pkg_name</i></tt>.
 **/
cu_str_t cuos_pkg_user_data_search(cuos_pkg_user_dirs_t udirs, cu_str_t path);

/** @} */
CU_END_DECLARATIONS

#endif
