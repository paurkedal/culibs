/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <urkedal@nbi.dk>
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

#include <cutext/src.h>
#include <cu/wchar.h>
#include <cu/memory.h>
#include <cu/test.h>

#define EMIT_COUNT 100000
#define BUF_SIZE 128

cu_clos_def(_produce, cu_prot(cutext_status_t, void **buf, size_t size),
    ( int i; ))
{
    cu_clos_self(_produce);

    if (self->i > EMIT_COUNT)
	return cutext_status_eos;
    else if (size >= 10) {
	sprintf(*buf, "%08x\n", self->i++);
	*buf = cu_ptr_add(*buf, 9);
	return cutext_status_ok;
    }
    else
	return cutext_status_buffer_too_small;
}

int
main()
{
    _produce_t prod0;
    cutext_producer_t prod1;
    cutext_src_t src;

    cutext_init();

    prod0.i = 0;
    src = cutext_src_new(_produce_prep(&prod0));
    prod1 = cutext_producer_new_iconv(src, CUTEXT_ENCODING_UTF8,
				      CUTEXT_ENCODING_UCS4HOST);

    for (;;) {
	cu_wchar_t buf[BUF_SIZE + 1];
	cu_wchar_t *p = buf;
	cutext_status_t st;
	st = cu_call(prod1, (void **)&p, BUF_SIZE*sizeof(cu_wchar_t));
	if (st == cutext_status_eos)
	    break;
	cu_debug_assert(p - buf <= BUF_SIZE);
	cu_debug_assert(st == cutext_status_ok);
	cu_galloc(1);
    }

    return 2*!!cu_test_bug_count();
}
