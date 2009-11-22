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

#include <cu/dsink.h>
#include <cu/inherit.h>
#include <cu/memory.h>
#include <cu/buffer.h>
#include <cu/test.h>
#include <cu/size.h>
#include <string.h>

#define TEST_SIZE 4096

typedef struct _testsink *_testsink_t;
struct _testsink
{
    cu_inherit (cu_dsink);
    char buf[TEST_SIZE];
    size_t size;
};

#define TESTSINK(sink) cu_from(_testsink, cu_dsink, sink)

static size_t
_testsink_write(cu_dsink_t sink, void const *data, size_t size)
{
    size_t wr_size;
    cu_test_assert(TESTSINK(sink)->size + size <= TEST_SIZE);
    if (TESTSINK(sink)->size + size == TEST_SIZE)
	wr_size = size;
    else
	wr_size = lrand48() % size + 1;
    memcpy(TESTSINK(sink)->buf + TESTSINK(sink)->size, data, wr_size);
    TESTSINK(sink)->size += wr_size;
    return wr_size;
}

static cu_word_t
_testsink_control(cu_dsink_t sink, int fc, va_list va)
{
    switch (fc) {
	default:
	    return CU_DSINK_ST_UNIMPL;
    }
}

_testsink_t
_testsink_new(void)
{
    _testsink_t sink = cu_gnew(struct _testsink);
    cu_dsink_init(cu_to(cu_dsink, sink), _testsink_control, _testsink_write);
    sink->size = 0;
    return sink;
}

void
test_dsink_buffer(int i_run)
{
    char buf[TEST_SIZE];
    int i;
    _testsink_t testsink = _testsink_new();
    cu_dsink_t sink = cu_dsink_stack_buffer(cu_to(cu_dsink, testsink));

    for (i = 0; i < TEST_SIZE; ++i)
	buf[i] = lrand48();
    i = 0;
    while (i < TEST_SIZE) {
	size_t n = cu_size_min(lrand48() % 128 + 1, TEST_SIZE - i);
	size_t n_w = cu_dsink_write(sink, buf + i, n);
	cu_test_assert(n_w == n);
	i += n;
    }
    cu_dsink_flush(sink);
    if (i_run < 20)
	cu_dsink_control(sink, CU_DSINK_FN_DEBUG_DUMP);
    cu_test_assert(testsink->size == TEST_SIZE);
    cu_test_assert(memcmp(buf, testsink->buf, TEST_SIZE) == 0);
}

int
main()
{
    int i;
    cu_init();
    for (i = 0; i < 2000; ++i)
	test_dsink_buffer(i);
    return 2*!!cu_test_bug_count();
}
