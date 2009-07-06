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
#include <cuos/path.h>
#include <cu/str.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

cuos_dirpile_t
cuos_dirpile_new(void)
{
    cuos_dirpile_t pile = cu_gnew(struct cuos_dirpile_s);
    cuos_dirpile_init(pile);
    return pile;
}

cu_bool_t
cuos_dirpile_insert(cuos_dirpile_t pile, cu_str_t path, cu_bool_t on_top)
{
    struct stat st;
    if (stat(cu_str_to_cstr(path), &st) == 0) {
	if (S_ISDIR(st.st_mode)) {
	    if (on_top)
		cucon_list_prepend_ptr(&pile->top_dirs, path);
	    else
		cucon_list_append_ptr(&pile->top_dirs, path);
	    return cu_true;
	}
	else
	    cu_warnf("Ignoring non-directory %s for search path.",
		     cu_str_to_cstr(path));
    }
    return cu_false;
}

int
cuos_dirpile_insert_envvar(cuos_dirpile_t pile, char const *envvar,
			   cu_bool_t on_top)
{
    char const *dirs = getenv(envvar);
    int insert_count = 0;
    if (dirs == NULL)
	return -1;
    do {
	char const *next_dirs = strchr(dirs, ':');
	cu_str_t dir;
	if (next_dirs == NULL) {
	    dir = cu_str_new_cstr(dirs);
	    dirs = NULL;
	}
	else {
	    dir = cu_str_new_charr(dirs, next_dirs - dirs);
	    dirs = next_dirs + 1;
	}
	if (cu_str_size(dir) > 0)
	    if (cuos_dirpile_insert(pile, dir, on_top))
		++insert_count;
    } while (dirs);
    return insert_count;
}

cu_bool_t
cuos_dirpile_iterA_top(cuos_dirpile_t pile,
		       cu_clop(f, cu_bool_t, cu_str_t))
{
    cucon_listnode_t node;
    for (node = cucon_list_begin(&pile->top_dirs);
	 node != cucon_list_end(&pile->top_dirs);
	 node = cucon_listnode_next(node))
	if (!cu_call(f, cucon_listnode_ptr(node)))
	    return cu_false;
    return cu_true;
}

cu_str_t
cuos_dirpile_first_match(cuos_dirpile_t pile, cu_str_t rel_path)
{
    cucon_listnode_t node;
    for (node = cucon_list_begin(&pile->top_dirs);
	 node != cucon_list_end(&pile->top_dirs);
	 node = cucon_listnode_next(node)) {
	struct stat st;
	cu_str_t path = cuos_path_join(cucon_listnode_ptr(node), rel_path);
	if (stat(cu_str_to_cstr(path), &st) == 0)
	    return path;
    }
    return NULL;
}

cu_bool_t
cuos_dirpile_iterA_matches(cuos_dirpile_t pile,
			   cu_clop(f, cu_bool_t, cu_str_t),
			   cu_str_t rel_path)
{
    cucon_listnode_t node;
    for (node = cucon_list_begin(&pile->top_dirs);
	 node != cucon_list_end(&pile->top_dirs);
	 node = cucon_listnode_next(node)) {
	struct stat st;
	cu_str_t path = cuos_path_join(cucon_listnode_ptr(node), rel_path);
	if (stat(cu_str_to_cstr(path), &st) == 0)
	    if (!cu_call(f, path))
		return cu_false;
    }
    return cu_true;
}
