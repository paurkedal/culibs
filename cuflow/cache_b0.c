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

#include <cuflow/cache.h>
#include <cuflow/cacheconf.h>
#include <cuflow/cache_b0_tab.h>
#include <cuflow/time.h>

cuflow_cached_edcl(fn1,
    ( int i; ),
    ( int j; ));

cuflow_cached_edef(fn1)
{
    fn1_obj_t *obj = cuflow_cached_new(fn1, 0);
    obj->j = key->i;
    cuflow_cacheobj_set_gain((cuflow_cacheobj_t)obj, 1.0);
    return obj;
}

#define CALL_CNT 4000000

void test()
{
    int i;
    clock_t t = -clock();
    for (i = 0; i < CALL_CNT; ++i) {
	struct fn1_key key;
	fn1_obj_t *obj;
	key.i = i;
	obj = testcache_call(fn1, &key);
    }
    t += clock();
    printf("%lg s per uncached call\n", t/((double)CLOCKS_PER_SEC*CALL_CNT));
    t = -clock();
    for (i = 0; i < CALL_CNT; ++i) {
	struct fn1_key key;
	fn1_obj_t *obj;
	key.i = 0;
	obj = testcache_call(fn1, &key);
    }
    t += clock();
    printf("%lg s per cached call\n", t/((double)CLOCKS_PER_SEC*CALL_CNT));
}

int main()
{
    cuflow_init();
    testcache_init(cuflow_default_cacheconf());
    test();
    return 0;
}
