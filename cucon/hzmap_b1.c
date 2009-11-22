/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/hzmap.h>
#include <cu/test.h>

void
profile(int N)
{
    int i;
    struct cucon_hzmap map;
    cucon_hzmap_init(&map, 1);
    for (i = 0; i < N; ++i) {
	cu_word_t key = i*i%N;
	cucon_hzmap_insert_void(&map, &key);
    }
    cucon_hzmap_dump_stats(&map, stdout);
}

int
main()
{
    int j;
    cu_init();
    for (j = 0; j < 22; ++j)
	profile(1 << j);
    return 0;
}
