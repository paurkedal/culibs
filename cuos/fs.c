/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#define CUOS_FS_C
#include <cuos/fs.h>
#include <cuos/path.h>
#include <cu/str.h>
#include <cucon/rbtree.h>
#include <cucon/list.h>
#include <cu/debug.h>
#include <cu/thread.h>
#include <cu/init.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <atomic_ops.h>


cu_bool_t
cuos_mkdir_rec(cu_str_t path, mode_t mode)
{
    cu_str_t dir;
    struct stat st;
    if (stat(cu_str_to_cstr(path), &st) == 0) {
	if (S_ISDIR(st.st_mode))
	    return cu_true;
	else {
	    cu_errf("Can not create directory %s, already exists and "
		     "is not a directory.",
		     cu_str_to_cstr(path));
	    return cu_false;
	}
    }
    if (cuos_path_is_empty(path)) {
	cu_errf("Could not stat the current directory.");
	return cu_false;
    }
    dir = cuos_path_dir(path);
    if (!cuos_mkdir_rec(dir, mode))
	return cu_false;
    if (mkdir(cu_str_to_cstr(path), mode) != 0) {
	cu_errf(strerror(errno));
	return cu_false;
    }
    return cu_true;
}

cu_bool_t
cuos_remove_rec(cu_str_t path)
{
    struct stat st;
    cu_debug_assert(!cu_str_is_empty(path));
    cu_debug_assert(cuos_is_path_str(path));
    if (lstat(cu_str_to_cstr(path), &st) == -1) {
	cu_errf("%s", strerror(errno));
	return cu_false;
    }
    if (S_ISDIR(st.st_mode)) {
	DIR *dir = opendir(cu_str_to_cstr(path));
	struct dirent *dent;
	if (!dir) {
	    cu_errf("%s", strerror(errno));
	    return cu_false;
	}
	while ((dent = readdir(dir))) {
	    if (strcmp(dent->d_name, ".") == 0 ||
		strcmp(dent->d_name, "..") == 0)
		continue;
	    if (!cuos_remove_rec(
		    cuos_path_join(path,
				      cu_str_new_cstr(dent->d_name)))) {
		closedir(dir);
		return cu_false;
	    }
	}
	closedir(dir);
    }
    if (remove(cu_str_to_cstr(path)) == -1) {
	cu_errf("%s", strerror(errno));
	return cu_false;
    }
    return cu_true;
}


/* Generic Iteration
 * ----------------- */

cu_bool_t
cuos_dirrec_conj_files(cu_str_t dname,
			  cu_clop(cb, cu_bool_t, cu_str_t))
{
    struct stat st;
    if (stat(cu_str_to_cstr(dname), &st) == -1)
	return cu_true;
    if (S_ISDIR(st.st_mode)) {
	struct dirent *ent;
	DIR *dir = opendir(cu_str_to_cstr(dname));
	if (!dir)
	    return cu_true;
	while ((ent = readdir(dir)) != NULL) {
	    cu_str_t subname;
	    if (ent->d_name[0] == '.'
		&& (ent->d_name[1] == 0
		    || (ent->d_name[1] == '.'
			&& ent->d_name[2] == 0)))
		continue;
	    subname = cuos_path_join(dname, cu_str_new_cstr(ent->d_name));
	    if (!cuos_dirrec_conj_files(subname, cb))
		return cu_false;
	}
    }
    else if (S_ISREG(st.st_mode)) {
	if (!cu_call(cb, dname))
	    return cu_false;
    }
    return cu_true;
}

cu_clos_def(sys_dirreccoll_conj_files_cb,
	    cu_prot(cu_bool_t, void *sub_bname),
	    ( cu_clop(cb, cu_bool_t, cu_str_t);
	      cu_str_t dname; ) )
{
    cu_clos_self(sys_dirreccoll_conj_files_cb);
    cu_str_t subname = cuos_path_join(self->dname, sub_bname);
    return cuos_dirreccoll_conj_files(subname, self->cb);
}
cu_bool_t
cuos_dirreccoll_conj_files(cu_str_t dname,
			      cu_clop(cb, cu_bool_t, cu_str_t))
{
    struct stat st;
    if (stat(cu_str_to_cstr(dname), &st) == -1)
	return cu_true;
    if (S_ISDIR(st.st_mode)) {
	sys_dirreccoll_conj_files_cb_t subcb;
	cucon_rbtree_t entries = cucon_rbtree_new();
	struct dirent *ent;
	DIR *dir = opendir(cu_str_to_cstr(dname));
	if (!dir)
	    return cu_true;
	while ((ent = readdir(dir)) != NULL) {
	    cu_str_t sub_bname;
	    if (ent->d_name[0] == '.'
		&& (ent->d_name[1] == 0
		    || (ent->d_name[1] == '.'
			&& ent->d_name[2] == 0)))
		continue;
	    sub_bname = cu_str_new_cstr(ent->d_name);
	    cucon_rbtree_insert2p_ptr(entries,
		    (cu_clop(, int, void *, void *))cu_str_coll_clop,
		    &sub_bname);
	}
	subcb.cb = cb;
	subcb.dname = dname;
	return cucon_rbtree_conj_ptr(
		    entries, sys_dirreccoll_conj_files_cb_prep(&subcb));
    }
    else if (S_ISREG(st.st_mode)) {
	if (!cu_call(cb, dname))
	    return cu_false;
    }
    return cu_true;
}


/* Prefix Searches
 * --------------- */

cu_bool_t
cuos_prefixsearch_conj(cucon_list_t pfx_l, cu_str_t name,
			  cu_clop(cb, cu_bool_t, cu_str_t result))
{
    cucon_listnode_t it;
    for (it = cucon_list_begin(pfx_l);
	 it != cucon_list_end(pfx_l); it = cucon_listnode_next(it)) {
	struct stat st;
	cu_str_t path = cu_str_new_copy(cucon_listnode_ptr(it));
	cu_str_append_char(path, '/');
	cu_str_append_str(path, name);
	if (stat(cu_str_to_cstr(path), &st) == 0)
	    if (!cu_call(cb, path))
		return cu_false;
    }
    return cu_true;
}

cu_str_t
cuos_prefixsearch_first(cucon_list_t pfx_l, cu_str_t name)
{
    cucon_listnode_t it;
    for (it = cucon_list_begin(pfx_l);
	 it != cucon_list_end(pfx_l); it = cucon_listnode_next(it)) {
	struct stat st;
	cu_str_t path = cu_str_new_copy(cucon_listnode_ptr(it));
	cu_str_append_char(path, '/');
	cu_str_append_str(path, name);
	if (stat(cu_str_to_cstr(path), &st) == 0)
	    return path;
    }
    return NULL;
}

void
cuos_prefixsearch_append_all(cucon_list_t pfx_l, cu_str_t name,
				cucon_list_t res)
{
    cucon_listnode_t it;
    for (it = cucon_list_begin(pfx_l);
	 it != cucon_list_end(pfx_l); it = cucon_listnode_next(it)) {
	struct stat st;
	cu_str_t path = cu_str_new_copy(cucon_listnode_ptr(it));
	cu_str_append_char(path, '/');
	cu_str_append_str(path, name);
	if (stat(cu_str_to_cstr(path), &st) == 0)
	    cucon_list_append_ptr(res, path);
    }
    return;
}


/* Query
 * ----- */

cuos_dentry_type_t
cuos_dentry_type(cu_str_t path)
{
    struct stat st;
    if (lstat(cu_str_to_cstr(path), &st) < 0) {
	switch (errno) {
	case ENOENT:
	case ENOTDIR:
	    return cuos_dentry_type_none;
	default:
	    cu_errf("Failed to stat %s: %s", cu_str_to_cstr(path),
		     strerror(errno));
	    return cuos_dentry_type_unknown;
	}
    }
    if (S_ISREG(st.st_mode))
	return cuos_dentry_type_file;
    if (S_ISDIR(st.st_mode))
	return cuos_dentry_type_dir;
    if (S_ISCHR(st.st_mode))
	return cuos_dentry_type_char_dev;
    if (S_ISBLK(st.st_mode))
	return cuos_dentry_type_block_dev;
    if (S_ISFIFO(st.st_mode))
	return cuos_dentry_type_fifo;
    if (S_ISLNK(st.st_mode))
	return cuos_dentry_type_symlink;
    if (S_ISSOCK(st.st_mode))
	return cuos_dentry_type_socket;
    return cuos_dentry_type_unknown;
}

time_t
cuos_mtime(cu_str_t path)
{
    struct stat st;
    if (stat(cu_str_to_cstr(path), &st) < 0) {
	switch (errno) {
	case ENOENT:
	case ENOTDIR:
	    return 0;
	default:
	    cu_errf("Failed to stat %s: %s", cu_str_to_cstr(path),
		     strerror(errno));
	    return 0;
	}
    }
    return st.st_mtime;
}


/* cuos_tmp_dir
 * ------------ */

static pthread_mutex_t _tmp_dir_mutex = CU_MUTEX_INITIALISER;
static AO_t /* cu_str_t */ _tmp_dir = NULL;

static void _tmp_dir_clean()
{
    cu_debug_assert(_tmp_dir);
    cuos_remove_rec(AO_load(&_tmp_dir));
}

cu_str_t
cuos_tmp_dir()
{
    cu_str_t dir = (cu_str_t)AO_load_acquire_read(&_tmp_dir);
    if (dir)
	return dir;

    cu_mutex_lock(&_tmp_dir_mutex);
    if (!_tmp_dir) {
	char const *env_tmpdir = getenv("TMPDIR");
	if (env_tmpdir)
	    _tmp_dir = cu_str_new_2cstr(env_tmpdir, "/culibs.XXXXXX");
	else
	    _tmp_dir = cu_str_new_cstr("/tmp/culibs.XXXXXX");
	if (!mkdtemp((char *)cu_str_to_cstr(_tmp_dir))) {
	    cu_errf("Could not create temporary directory for process.");
	    exit(71); /* cf sysexits.h */
	}
	atexit(_tmp_dir_clean);
    }
    cu_mutex_unlock(&_tmp_dir_mutex);
    return _tmp_dir;
}

/* cuos_session_dir
 * ---------------- */

static pthread_mutex_t session_dir_mutex = CU_MUTEX_INITIALISER;
static AO_t /* cu_str_t */ session_dir = 0;

cu_str_t
cuos_session_dir(mode_t mode)
{
    cu_str_t dir = (cu_str_t)AO_load_acquire_read(&session_dir);
    if (dir)
	return dir;

    cu_mutex_lock(&session_dir_mutex);
    if (session_dir)
	dir = (cu_str_t)session_dir;
    else {
	char const *home_dir = getenv("HOME");
	char const *app_name = cu_application_name_cstr();
	struct stat st;
	if (!home_dir) {
	    cu_errf("$HOME is not defined!");
	    cu_mutex_unlock(&session_dir_mutex);
	    return NULL;
	}
	dir = cu_str_new_3charr(home_dir, strlen(home_dir), "/.", 2,
				app_name, strlen(app_name));
	if (stat(cu_str_to_cstr(dir), &st)) {
	    if (errno == ENOENT && mode) {
		cu_verbf(1, "Creating %S for session data.", dir);
		if (!cuos_mkdir_rec(dir, mode)) {
		    cu_errf("Failed to create %S for session data.", dir);
		    dir = NULL;
		}
	    }
	    else {
		cu_errf("Failed to stat %S: %s", dir, strerror(errno));
		dir = NULL;
	    }
	}
	else if (!S_ISDIR(st.st_mode)) {
	    cu_errf("Non-directory %S is in the way of session data.", dir);
	    dir = NULL;
	}
	AO_store_release_write(&session_dir, (AO_t)dir);
    }
    cu_mutex_unlock(&session_dir_mutex);
    return dir;
}
