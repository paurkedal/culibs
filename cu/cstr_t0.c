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

#include <cu/memory.h>
#include <cu/cstr.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int
main()
{
    size_t n_repeat = 1000;
    char const* s;
    size_t i;

    cu_init();

    s = cu_struniq("gjenganger");
    assert(s == cu_struniq("gjenganger"));
    assert(s != cu_struniq("gjest"));
    GC_gcollect();

    for (i = 0; i < n_repeat; ++i) {
	static char str[sizeof(int)*3 + 1];
	sprintf(str, "%ld", lrand48() % n_repeat);
	/*&char const* str_u =*/ cu_struniq(str);
	if (i % (n_repeat/4) == 0)
	    GC_gcollect();
    }
    
    return 0;
}
