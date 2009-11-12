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

#include <cuoo/halloc.h>
#include <cu/test.h>
#include <errno.h>

#define N_ALLOC 10000000
#define N_ALLOC_TH 3

static cuoo_type_t _obj_type;
static AO_t pending = N_ALLOC_TH;

static void *
_alloc_proc(void *arg)
{
    int i;
    for (i = 0; i < N_ALLOC; ++i) {
	cu_word_t tpl[1], *obj;
	tpl[0] = (cu_word_t)arg;
	obj = cu_ptr_add(cuoo_halloc(_obj_type, sizeof(cu_word_t), tpl),
			 CUOO_HCOBJ_SHIFT);
	cu_test_assert(*obj == (cu_word_t)arg);
    }
    AO_fetch_and_add(&pending, -1);
    return NULL;
}

static void *
_gcollect_proc(void *arg)
{
    while (AO_load(&pending))
	GC_gcollect();
    return NULL;
}

static void
_fail(int err, char const *func_name)
{
    fprintf(stderr, "%s failed: %s", func_name, strerror(err));
    exit(2);
}

int
main()
{
    pthread_t alloc_pth[N_ALLOC_TH], gcollect_pth;
    int i, err;

    cuoo_init();
    _obj_type = cuoo_type_new_opaque_hcs(NULL, sizeof(cu_word_t));

    err = GC_pthread_create(&gcollect_pth, NULL, _gcollect_proc, NULL);
    if (err) _fail(err, "GC_pthread_create");

    for (i = 0; i < N_ALLOC_TH; ++i) {
	err = GC_pthread_create(&alloc_pth[i], NULL, _alloc_proc, &i + i);
	if (err) _fail(err, "GC_pthread_create");
    }
    for (i = 0; i < N_ALLOC_TH; ++i) {
	err = GC_pthread_join(alloc_pth[i], NULL);
	if (err) _fail(err, "GC_pthread_join");
    }

    err = GC_pthread_join(gcollect_pth, NULL);
    if (err) _fail(err, "GC_pthread_join");

    return 2*!!cu_test_bug_count();
}
