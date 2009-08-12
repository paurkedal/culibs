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

#include <cufo/termstyle.h>
#include <cufo/tag.h>
#include <cuos/fs.h>
#include <cuos/dirpile.h>
#include <cuos/path.h>
#include <cucon/list.h>
#include <cu/memory.h>
#include <cu/thread.h>
#include <cu/sref.h>
#include <cu/str.h>
#include <string.h>
#include <ctype.h>

/* USE OF THE DEBUG LOG HERE.  The style loading code may run before we
 * register our log binder, so don't use extended format specifiers here. */
cu_dlog_def(_file, "dtag=cufo.termstyle");

static struct cuos_dirpile_s _termstyle_dirpile;

static struct { char const *name; unsigned int mask; } _booleans[] = {
    {"italic",    CUFO_TERMFACE_ITALIC},
    {"underline", CUFO_TERMFACE_UNDERLINE},
    {"bold",      CUFO_TERMFACE_BOLD},
    {"reverse",   CUFO_TERMFACE_REVERSE},
};

#define BOOL_ATTR_COUNT (sizeof(_booleans)/sizeof(_booleans[0]))

void
cufo_termface_init(cufo_termface_t face)
{
    memset(face, 0, sizeof(struct cufo_termface_s));
}

cufo_termface_t
cufo_termface_new()
{
    cufo_termface_t face = cu_gnew(struct cufo_termface_s);
    cufo_termface_init(face);
    return face;
}

void
cufo_termface_set_fgcolour(cufo_termface_t face, int col)
{
    face->fgcolour = col;
    face->enables |= CUFO_TERMFACE_FGCOLOUR;
}

void
cufo_termface_set_bgcolour(cufo_termface_t face, int col)
{
    face->bgcolour = col;
    face->enables |= CUFO_TERMFACE_BGCOLOUR;
}

static int
_parse_colour(char const *colour)
{
    long i = strtol(colour, (char **)&colour, 0);
    if (*colour || !(0 <= i && i < 256))
	return -1;
    return i;
}

cu_bool_t
cufo_termface_set_fgcolour_cstr(cufo_termface_t face, char const *colour)
{
    int i = _parse_colour(colour);
    if (i < 0)
	return cu_false;
    cufo_termface_set_fgcolour(face, i);
    return cu_true;
}

cu_bool_t
cufo_termface_set_bgcolour_cstr(cufo_termface_t face, char const *colour)
{
    int i = _parse_colour(colour);
    if (i < 0)
	return cu_false;
    cufo_termface_set_bgcolour(face, i);
    return cu_true;
}

void
cufo_termface_set_bool(cufo_termface_t face, unsigned int attr, cu_bool_t val)
{
    if (val)
	face->booleans |= attr;
    else
	face->booleans &= ~attr;
    face->enables |= attr;
}

void
cufo_termstyle_init(cufo_termstyle_t style)
{
    cucon_pmap_init(&style->face_map);
}

cufo_termstyle_t
cufo_termstyle_new(void)
{
    cufo_termstyle_t style = cu_gnew(struct cufo_termstyle_s);
    cufo_termstyle_init(style);
    return style;
}

cufo_termface_t
cufo_termstyle_get(cufo_termstyle_t style, cufo_tag_t tag)
{
    return cucon_pmap_find_mem(&style->face_map, tag);
}

cufo_termface_t
cufo_termstyle_ref(cufo_termstyle_t style, cufo_tag_t tag)
{
    cufo_termface_t face;
    if (cucon_pmap_insert_mem(&style->face_map, tag,
			      sizeof(struct cufo_termstyle_s), &face))
	cufo_termface_init(face);
    return face;
}

static void
_termstyle_loadinto(cufo_termstyle_t style, cu_str_t path, FILE *in)
{
    struct cu_sref_s loc;
    cu_sref_init(&loc, path, 0, -1);
    for (;;) {
	char buf[160];
	char *s;
	char const *tag_name;
	cufo_tag_t tag;
	cufo_termface_t face;

	cu_sref_newline(&loc);
	if (!fgets(buf, sizeof(buf), in))
	    break;
	s = buf;
	while (*s && isspace(*s)) ++s;
	if (!*s || *s == '#')
	    continue;

	/* Scan the tag name. */
	tag_name = s;
	while (*s && !isspace(*s)) ++s;
	if (!*s)
	    continue;
	*s++ = 0;
	tag = cufo_tag(cufo_culibs_namespace(), tag_name);
	face = cufo_termstyle_ref(style, tag);

	/* Scan a sequence of "bold", "!bold", "fg=X", etc. */
	do {
	    char const *var;

	    while (*s && isspace(*s)) ++s;
	    if (!*s)
		break;

	    var = s;
	    while (*s && !isspace(*s) && *s != '=') ++s;

	    if (*s == '=') {
		char const *val;
		*s++ = 0;
		val = s;
		while (*s && !isspace(*s)) ++s;
		if (*s != 0)
		    *s++ = 0;
		if (strcmp(var, "fg") == 0) {
		    cu_dlogf(_file, "Setting %s fgcolour %s",
			     cufo_tag_name(tag), val);
		    if (!cufo_termface_set_fgcolour_cstr(face, val))
			cu_warnf_at(&loc, "Invalid foreground colour %s.", val);
		}
		else if (strcmp(var, "bg") == 0) {
		    cu_dlogf(_file, "Setting %s bgcolour %s",
			     cufo_tag_name(tag), val);
		    if (!cufo_termface_set_bgcolour_cstr(face, val))
			cu_warnf_at(&loc, "Ivalid background colour %s.", val);
		}
	    }
	    else {
		cu_bool_t value;
		int i;

		if (*s)
		    *s++ = 0;

		if (*var == '!') {
		    ++var;
		    value = cu_false;
		}
		else
		    value = cu_true;
		for (i = 0; i < BOOL_ATTR_COUNT; ++i)
		    if (strcmp(var, _booleans[i].name) == 0) {
			cu_dlogf(_file, "Setting %s attr %s=%s",
				 cufo_tag_name(tag), var,
				 value? "true" : "false");
			cufo_termface_set_bool(face, _booleans[i].mask, value);
			break;
		    }
		if (i == BOOL_ATTR_COUNT)
		    cu_warnf_at(&loc, "Unrecognised text attribute %s.", var);
	    }
	} while (*s);
    }
}

cu_bool_t
cufo_termstyle_loadinto(cufo_termstyle_t style, cu_str_t style_name)
{
    FILE *in;
    cu_str_t fname = cu_str_new_str_cstr(style_name, ".termsty");
    cu_str_t path = cuos_dirpile_first_match(&_termstyle_dirpile, fname);

    if (path) {
	char const *path_cstr = cu_str_to_cstr(path);
	cu_dlogf(_file, "Loading terminal style %s.",
		 cu_str_to_cstr(style_name));
	in = fopen(path_cstr, "r");
	if (!in) {
	    cu_warnf("Failed to open %s.", path_cstr);
	    return cu_false;
	}
	_termstyle_loadinto(style, path, in);
	return cu_true;
    }
    else {
	cu_warnf("Missing terminal style %(str).", style_name);
	return cu_false;
    }
}

cufo_termstyle_t
cufo_termstyle_load(cu_str_t style_name)
{
    cufo_termstyle_t style = cufo_termstyle_new();
    cufo_termstyle_loadinto(style, style_name);
    return style;
}

void
cufoP_termstyle_init()
{
    cu_str_t path;
    char const *homedir = getenv("HOME");

    cuos_dirpile_init(&_termstyle_dirpile);
    cuos_dirpile_insert_envvar(&_termstyle_dirpile, "CU_STYLE_PATH", cu_false);

    if (homedir) {
	path = cuos_path_join_2cstr(homedir, ".config/culibs/style");
	cuos_dirpile_insert(&_termstyle_dirpile, path, cu_false);
    }

    path = cu_str_new_cstr("/etc/culibs/style");
    cuos_dirpile_insert(&_termstyle_dirpile, path, cu_false);

    /* TODO: Look for shipped highlighting in $pkg_datadir. */
}
