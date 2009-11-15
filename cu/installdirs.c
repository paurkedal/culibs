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

#include <cu/memory.h>
#include <cu/installdirs.h>
#include <cu/diag.h>
#include <string.h>
#include <ctype.h>

void
cu_installdirs_set(cu_installdirs_t installdirs, cu_installdir_key_t key,
		   char const *dir)
{
    installdirs[key].prefix_key = CU_INSTALLDIR_NONE;
    installdirs[key].suffix = dir;
}

cu_bool_t
cu_installdirs_set_byname(cu_installdirs_t installdirs, char const *name,
			  char const *dir)
{
    cu_installdir_t installdir = installdirs;
    while (installdir->name) {
	if (strcmp(installdir->name, name) == 0) {
	    installdir->suffix = dir;
	    return cu_true;
	}
	++installdir;
    }
    return cu_false;
}

void
cu_installdirs_set_byenv(cu_installdirs_t installdirs, char const *prefix)
{
    cu_installdir_t installdir = installdirs;
    size_t prefix_len = strlen(prefix);
    while (installdir->name) {
	char const *envval, *s;
	char *sp;
	char *envvar = cu_salloc(prefix_len + strlen(installdir->name) + 1);
	strcpy(envvar, prefix);
	sp = envvar + prefix_len;
	if (islower(*prefix))
	    strcpy(sp, installdir->name);
	else {
	    for (s = installdir->name; *s; ++s)
		*sp++ = toupper(*s);
	    *sp = '\0';
	}
	envval = getenv(envvar);
	if (envval)
	    installdir->dir = envval;
	++installdir;
    }
}

static void
_installdir_finish(cu_installdirs_t installdirs, cu_installdir_t installdir)
{
    if (installdir->dir == (char const *)-1)
	cu_bugf("Cyclic installdir definitions.");
    else if (installdir->prefix_key == CU_INSTALLDIR_NONE)
	installdir->dir = installdir->suffix;
    else {
	size_t prefix_len;
	char *dir;
	cu_installdir_t ref_installdir = &installdirs[installdir->prefix_key];
	installdir->dir = (char const *)-1;
	if (!ref_installdir->dir)
	    _installdir_finish(installdirs, ref_installdir);
	prefix_len = strlen(ref_installdir->dir);
	dir = cu_ualloc_atomic(prefix_len + strlen(installdir->suffix) + 1);
	strcpy(dir, ref_installdir->dir);
	strcpy(dir + prefix_len, installdir->suffix);
	installdir->dir = dir;
    }
}

void
cu_installdirs_finish(cu_installdirs_t installdirs)
{
    cu_installdir_t installdir = installdirs;
    while (installdir->name) {
	if (!installdir->dir)
	    _installdir_finish(installdirs, installdir);
	++installdir;
    }
}

void
cu_installdirs_dump(cu_installdirs_t installdirs)
{
    cu_installdir_t installdir;
    for (installdir = installdirs; installdir->name; ++installdir)
	cu_verbf(0, "%s=%s", installdir->name, installdir->dir);
}
