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

#include <cu/hook.h>
#include <cu/test.h>

#define N_FUNC 22

static int _g_i;

cu_clos_def(_test_iter_func, cu_prot0(void), (int i;))
{
    cu_clos_self(_test_iter_func);
    cu_test_assert_int_eq(self->i, _g_i);
    ++_g_i;
}

void
_test_iter()
{
    int i, iS, iE;
    struct cu_iter_hook_s hook;
    _test_iter_func_t f[N_FUNC];

    cu_iter_hook_init(&hook);
    iS = iE = 0;
    for (i = 0; i < N_FUNC; ++i) {
	if (lrand48() % 2) {
	    f[i].i = --iS;
	    cu_iter_hook_prepend(&hook, _test_iter_func_prep(&f[i]));
	}
	else {
	    f[i].i = iE++;
	    cu_iter_hook_append(&hook, _test_iter_func_prep(&f[i]));
	}
    }
    _g_i = iS;
    cu_iter_hook_call(&hook);
    cu_test_assert_int_eq(_g_i, iE);
}

cu_clos_def(_test_iterA_ptr_func, cu_prot(cu_bool_t, void *arg), (int i;))
{
    cu_clos_self(_test_iterA_ptr_func);
    cu_test_assert_int_eq(self->i, ((int *)arg)[0]);
    ++((int *)arg)[0];
    return ((int *)arg)[0] <= ((int *)arg)[1];
}

void
_test_iterA_ptr()
{
    int i, iS, iE, arg[2];
    struct cu_iterA_ptr_hook_s hook;
    _test_iterA_ptr_func_t f[N_FUNC];

    cu_iterA_ptr_hook_init(&hook);
    iS = iE = 0;
    for (i = 0; i < N_FUNC; ++i) {
	if (lrand48() % 2) {
	    f[i].i = --iS;
	    cu_iterA_ptr_hook_prepend(&hook, _test_iterA_ptr_func_prep(&f[i]));
	}
	else {
	    f[i].i = iE++;
	    cu_iterA_ptr_hook_append(&hook, _test_iterA_ptr_func_prep(&f[i]));
	}
    }
    arg[0] = iS;
    arg[1] = iE;
    cu_test_assert(cu_iterA_ptr_hook_call(&hook, arg));
    arg[0] = iS;
    arg[1] = iE - 1;
    cu_test_assert(!cu_iterA_ptr_hook_call(&hook, arg));
}

int
main()
{
    int i;
    cu_init();
    for (i = 0; i < 1024; ++i) {
	_test_iter();
	_test_iterA_ptr();
    }
    return 2*!!cu_test_bug_count();
}
