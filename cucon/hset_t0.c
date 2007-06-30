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

#include <cucon/fwd.h>
#include <cucon/hset.h>
#include <stdlib.h>
#include <assert.h>

cu_clop_def(eq, cu_bool_t, void *p0, void *p1)
{
    return p0 == p1;
}

cu_clop_def(hash, cu_hash_t, void *p)
{
    return (cu_hash_t)p >> 1;
}

int
main()
{
    size_t const n_ins = 100000;
    size_t i;
    cucon_hset_t hs;
    cu_init();

    hs = cucon_hset_new(eq, hash);
    for (i = 0; i < n_ins; ++i)
	if (cucon_hset_insert(hs, (void*)i) != NULL)
	    assert(!"unexpected degeneracy");
    for (i = 0; i < n_ins; i += 2)
	if (cucon_hset_erase(hs, (void*)i) != (void*)i)
	    assert(!"lost value");
    for (i = 0; i < n_ins; ++i) {
	void *key = cucon_hset_find(hs, (void*)i);
	if (i % 2)
	    assert(key == (void*)i);
	else
	    assert(key == NULL);
    }
    return 0;
}
