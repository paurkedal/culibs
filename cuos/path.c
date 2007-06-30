/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuos/path.h>
#include <cu/str.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

cu_bool_fast_t
cuos_is_path_dname(cu_str_t name)
{
    return cu_str_size(name) == 0
	|| cu_str_at(name, cu_str_size(name) - 1) == '/';
}

cu_str_t
cuos_path_as_dname(cu_str_t path)
{
    if (cu_str_size(path) == 0)
	return cu_str_new_cstr_static("/");
    else if (cu_str_size(path) == 0 && cu_str_at(path, 0) == '.')
	return cu_str_new();
    else if (cu_str_at(path, cu_str_size(path)) == '/')
	return path;
    else
	return cu_str_new_str_char(path, '/');
}

cu_str_t
cuos_dname_as_path(cu_str_t dname)
{
    if (cu_str_size(dname) == 0)
	return cu_str_new_cstr_static(".");
    else if (cu_str_at(dname, cu_str_size(dname) - 1) == '/') {
	if (cu_str_size(dname) == 1)
	    return dname;
	else
	    return cu_str_substr(dname, 0, cu_str_size(dname) - 1);
    }
    else
	return dname;
}

cu_bool_fast_t
cuos_is_path_charr(char const *s, size_t len)
{
    char const *se = s + len;
    if (len == 0)
	return cu_false;
    if (len == 1 && (*s == '.' || *s == '/'))
	return cu_true;
    if (se[-1] == '/')
	return cu_false;
    while (s + 2 < se && s[0] == '.' && s[1] == '.' && s[2] == '/')
	s += 3;
    if (s + 2 == se && s[0] == '.' && s[1] == '.')
	return cu_true;
    while (s < se) {
	if (*s == '.') {
	    ++s;
	    if (s == se || *s == '/')
		return cu_false;
	    if (*s == '.') {
		++s;
		if (s == se || *s == '/')
		    return cu_false;
	    }
	}
	while (*s != '/') {
	    ++s;
	    if (s == se)
		return cu_true;
	}
	++s;
	assert(s != se);
	if (*s == '/')
	    return cu_false;
    }
    return cu_true;
}

cu_bool_fast_t
cuos_is_path_cstr(char const *s)
{
    return cuos_is_path_charr(s, strlen(s));
}

cu_bool_fast_t
cuos_is_path_str(cu_str_t str)
{
    return cuos_is_path_charr(cu_str_charr(str), cu_str_size(str));
}

cu_str_t
cuos_path_from_charr(char const *s, size_t len)
{
    cu_str_t path;
    size_t i;
    int n_comp;
    struct chib_stringrange_s {
	char const *beg;
	char const *end;
    } *stack, *sp;
    int top_up = 0;
    cu_bool_fast_t is_abs;
    char const *se = s + len;
    char const *s0;

    if (s == se)
	return NULL;
    if (s + 1 == se && *s == '/')
	return cu_str_new_charr("/", 1);
    if (s[0] == '/') {
	is_abs = cu_true;
	++s;
    }
    else
	is_abs = cu_false;

    /* Strip leading "/" and "./" */
    while (s < se) {
	if (*s == '/')
	    ++s;
	else if (s + 1 < se && s[0] == '.' && s[1] == '/')
	    s += 2;
	else
	    break;
    }

    /* Strip trailing "/" and "/." */
    for (;;) {
	if (s == se)
	    return cu_str_new_charr(is_abs? "/" : ".", 1);
	else if (se[-1] == '/')
	    --se;
	else if (s + 1 < se && s[-1] == '.' && s[-2] == '/')
	    se -= 2;
	else
	    break;
    }

    /* Determine the maximum number of components */
    n_comp = 2;
    for (s0 = s; s0 < se; ++s0)
	if (*s0 == '/')
	    ++n_comp;
    stack = cu_salloc(n_comp*sizeof(struct chib_stringrange_s));

    /* Extract irreducible components */
    sp = stack;
    i = 0;
    while (1) {
	assert(sp < stack + n_comp);
	assert(s != se);
	assert(*s != '/');
	if (s[0] == '.') {
	    if (s + 1 == se)
		break;
	    else if (s[1] == '/') {
		s += 2;
		while (*s == '/') { ++s; assert(s != se); }
		continue;
	    }
	    else if (s[1] == '.') {
		if (s + 2 == se) {
		    if (sp > stack) {
			--sp;
			break;
		    }
		    else if (is_abs)
			return NULL;
		    else {
			++top_up;
			break;
		    }
		}
		else if (s[2] == '/') {
		    s += 3;
		    while (*s == '/') { ++s; assert(s != se); }
		    if (sp > stack) {
			--sp;
			continue;
		    }
		    else if (is_abs)
			return NULL;
		    else {
			++top_up;
			continue;
		    }
		}
		/* else fallthrough */
	    }
	    /* else fallthrough */
	}
	sp->beg = s;
	while (*s != '/') {
	    ++s;
	    if (s == se) {
		sp->end = s;
		++sp;
		goto break_0;
	    }
	}
	sp->end = s;
	++sp;
	do {
	    ++s;
	    assert(s != se);
	} while (*s == '/');
    }
break_0:
    sp->beg = NULL;

    /* Compose new normalised file name. */
    if (is_abs) {
	path = cu_str_new_charr("/", 1);
	assert(top_up == 0);
    }
    else if (stack->beg == NULL) {
	if (top_up) {
	    path = cu_str_new_charr("..", 2);
	    while (--top_up)
		cu_str_append_charr(path, "/..", 3);
	    return path;
	}
	else
	    return cu_str_new_charr(".", 1);
    }
    else {
	path = cu_str_new();
	while (top_up--)
	    cu_str_append_charr(path, "../", 3);
    }
    if (stack->beg == NULL)
	return path;
    cu_str_append_charr(path, stack->beg, stack->end - stack->beg);
    ++stack;
    while (stack->beg) {
	cu_str_append_char(path, '/');
	cu_str_append_charr(path, stack->beg, stack->end - stack->beg);
	++stack;
    }
    return path;
}

cu_str_t
cuos_path_from_str(cu_str_t path)
{
    return cuos_path_from_charr(cu_str_charr(path), cu_str_size(path));
}

cu_str_t
cuos_path_from_cstr(char const *s)
{
    return cuos_path_from_charr(s, strlen(s));
}

cu_bool_fast_t
cuos_path_is_abs(cu_str_t path)
{
    return cu_str_size(path) == 0
	|| cu_str_at(path, 0) == '/'
	|| cu_str_at(path, 0) == '~';
}

cu_bool_t
cuos_is_pathcomp_charr(char const *s, size_t n)
{
    char const *se = s + n;
    if (s == se)
	return cu_false;
    if (s[0] == '.' && (s + 1 == se || (s[1] == '.' && s + 2 == se)))
	return cu_false;
    while (s < se) {
	if ((unsigned char)*s < 0x80 && (*s == '/' || !isprint(*s)))
	    return cu_false;
	++s;
    }
    return cu_true;
}

cu_bool_t
cuos_is_pathcomp_cstr(char const *s)
{
    return cuos_is_pathcomp_charr(s, strlen(s));
}
cu_bool_t
cuos_is_pathcomp_str(cu_str_t str)
{
    return cuos_is_pathcomp_charr(cu_str_charr(str), cu_str_size(str));
}

int
cuos_path_component_count(cu_str_t path)
{
    char *sb = cu_str_charr(path);
    char *se = sb + cu_str_size(path);
    int n;
    if (sb + 1 == se) {
	if (sb[0] == '.')
	    return 0;
	if (sb[0] == '/')
	    return 1;
    }
    n = 1;
    while (sb < se) {
	if (*sb++ == '/')
	    ++n;
    }
    return n;
}

int
cuos_path_depth(cu_str_t path)
{
    char *sb = cu_str_charr(path);
    char *se = sb + cu_str_size(path);
    int n;
    if (sb + 1 == se) {
	if (sb[0] == '.')
	    return 0;
	if (sb[0] == '/')
	    return 1;
    }
    n = 1;
    while (sb + 3 < se && sb[0] == '.' && sb[1] == '.' && sb[2] == '/') {
	--n;
	sb += 3;
    }
    if (sb + 2 == se && sb[0] == '.' && sb[1] == '.')
	return n - 2;
    while (sb < se) {
	if (*sb++ == '/')
	    ++n;
    }
    return n;
}

cu_str_t
cuos_path_join(cu_str_t path0, cu_str_t path1)
{
    char *s1 = cu_str_charr(path1);
    char *s1e = s1 + cu_str_size(path1);
    char *s0 = cu_str_charr(path0);
    char *s0e = s0 + cu_str_size(path0);
    if (s1 == s1e)
	cu_debug_error("Second arg to cuos_path_join must be non-empty.");
    if (cuos_path_is_abs(path1))
	return path1;
    if (s1 + 1 == s1e && s1[0] == '.')
	return path0;
    if (s0 + 1 == s0e) {
	if (s0[0] == '.')
	    return path1;
	if (s0[0] == '/')
	    return cu_str_new_2charr("/", 1, s1, s1e - s1);
    }
    while (s1 + 2 <= s1e && s1[0] == '.' && s1[1] == '.'
	   && (s1 + 2 == s1e || s1[2] == '/')) {
	if (s0 == s0e)
	    return cu_str_new_charr(s1, s1e - s1);
	if (s0 + 2 <= s0e && s0e[-1] == '.' && s0e[-2] == '.'
	    && (s0 + 2 == s0e || s0e[-3] == '/'))
	    return cu_str_new_3charr(s0, s0e - s0, "/", 1, s1, s1e - s1);
	while (s0 < s0e) {
	    if (*--s0e == '/')
		break;
	}
	if (s1 + 2 == s1e) {
	    if (s0 == s0e)
		return cu_str_new_charr(".", 1);
	    else
		return cu_str_new_charr(s0, s0e - s0);
	}
	else
	    s1 += 3;
    }
    assert(s1 <= s1e);
    return cu_str_new_3charr(s0, s0e - s0, "/", 1, s1, s1e - s1);
}

cu_str_t
cuos_path_join_str_cstr(cu_str_t s0, char const *s1)
{
    return cuos_path_join(s0, cu_str_new_cstr(s1));
}

cu_str_t
cuos_path_join_cstr_str(char const *s0, cu_str_t s1)
{
    return cuos_path_join(cu_str_new_cstr(s0), s1);
}

cu_bool_fast_t
cuos_path_split(cu_str_t path, int pos, cu_str_t *path0, cu_str_t *path1)
{
    char *sb = cu_str_charr(path);
    char *se = sb + cu_str_size(path);
    char *s;
    if (sb + 1 == se) {
	if (sb[0] == '.') {
	    cu_str_t res = cu_str_new_charr(".", 1);
	    if (pos != 0)
		return cu_false;
	    if (path0)
		*path0 = res;
	    if (path1)
		*path1 = res;
	    return cu_true;
	}
    }
    if (pos > 0) {
	s = sb;
	if (s < se && *s == '/')
	    --pos;
	while (pos) {
	    if (s >= se)
		return cu_false;
	    while (++s < se)
		if (*s == '/')
		    break;
	    --pos;
	}
    }
    else if (pos < 0) {
	s = se;
	while (pos) {
	    if (s == sb) {
		if (*s == '/') {
		    if (path0)
			*path0 = cu_str_new_charr(".", 1);
		    if (path1)
			*path1 = path;
		    return cu_true;
		}
		else
		    return cu_false;
	    }
	    while (sb < s) {
		if (*--s == '/')
		    break;
	    }
	    ++pos;
	}
    }
    else {
	if (path0)
	    *path0 = cu_str_new_charr(".", 1);
	if (path1)
	    *path1 = path;
	return cu_true;
    }
    if (path0) {
	if (s == sb)
	    *path0 = cu_str_new_charr(*s == '/'? "/" : ".", 1);
	else
	    *path0 = cu_str_new_charr(sb, s - sb);
    }
    if (path1) {
	if (s < se && *s == '/')
	    ++s;
	if (s == se)
	    *path1 = cu_str_new_charr(".", 1);
	else
	    *path1 = cu_str_new_charr(s, se - s);
    }
    return cu_true;
}

cu_str_t
cuos_path_dir(cu_str_t path)
{
    char *sb = cu_str_charr(path);
    char *se = sb + cu_str_size(path);
    if (sb + 1 == se) {
	if (sb[0] == '.')
	    return path;
	if (sb[0] == '/')
	    return cu_str_new_charr(".", 1);
    }
    while (se > sb)
	if (*--se == '/') {
	    if (sb == se)
		++se;
	    return cu_str_substr(path, 0, se - sb);
	}
    return cu_str_new_charr(".", 1);
}

cu_str_t
cuos_path_base(cu_str_t path)
{
    char *sb = cu_str_charr(path);
    char *se = sb + cu_str_size(path);
    char *s;
    if (sb + 1 == se)
	return path;
    s = se;
    while (s > sb)
	if (*--s == '/') {
	    ++s;
	    return cu_str_substr(path, s - sb, se - s);
	}
    return path;
}

cu_str_t
cuos_path_ext(cu_str_t path)
{
    size_t n;
/*     path = cuos_dname_as_path(path); */
    n = cu_str_rchr(path, '.');
    if (n == cu_str_index_none || n == 0 || cu_str_at(path, n - 1) == '/'
	|| cu_str_chr_in_substr(path, n, cu_str_size(path) - n, '/')
	!= cu_str_index_none)
	return cu_str_new();
    else
	return cu_str_substr(path, n, cu_str_size(path) - n);
}

cu_str_t
cuos_path_sans_ext(cu_str_t path)
{
    size_t n;
/*     path = cuos_dname_as_path(path); */
    n = cu_str_rchr(path, '.');
    if (n == cu_str_index_none || n == 0 || cu_str_at(path, n - 1) == '/'
	|| cu_str_chr_in_substr(path, n, cu_str_size(path) - n, '/')
	!= cu_str_index_none)
	return path;
    else
	return cu_str_substr(path, 0, n);
}

cu_bool_t
cuos_path_ext_eq_charr(cu_str_t path, char const *arr, size_t size)
{
    size_t path_size = cu_str_size(path);
    char const *path_arr;
    if (path_size < size)
	return cu_false;
    path_arr = cu_str_charr(path) + path_size - size;
    return strncmp(path_arr, arr, size) == 0;
}

cu_bool_t
cuos_path_ext_eq(cu_str_t path, cu_str_t ext)
{
    return cuos_path_ext_eq_charr(path,
				  cu_str_charr(ext), cu_str_size(ext));
}

cu_bool_t
cuos_path_ext_eq_cstr(cu_str_t path, char const *ext)
{
    return cuos_path_ext_eq_charr(path, ext, strlen(ext));
}
