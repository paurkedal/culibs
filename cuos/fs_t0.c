/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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
#include <cu/str.h>
#include <stdio.h>
#include <string.h>

int
main(int argc, char **argv)
{
    char buf[5];
    cu_init();
    if (argc != 2) {
	fprintf(stderr, "Usage: %s DENTRY_TO_REMOVE_RECURSIVELY\n", argv[0]);
	return 1;
    }
    printf("!!! Remove %s recursively? (type yes to confirm) !!!", argv[1]);
    fgets(buf, 5, stdin);
    if (strcmp(buf, "yes\n") == 0) {
	if (!cuos_remove_rec(cu_str_new_cstr(argv[1])))
	    return 1;
    }
    return 0;
}
