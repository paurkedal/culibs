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

#include <cufo/stream.h>
#include <cufo/tagdefs.h>
#include <cu/test.h>

struct _attrperm {
    cufo_tag_t tag;
    char const *name;
};

static void
_attrperm_swap(struct _attrperm *a0, struct _attrperm *a1)
{
    cufo_tag_t tag;
    char const *name;
    tag = a1->tag;
    name = a1->name;
    a1->tag = a0->tag;
    a1->name = a0->name;
    a0->tag = tag;
    a0->name = name;
}

static void
_test_attrperm(cufo_stream_t fos, struct _attrperm *a, int n)
{
    int i;
#if 0
    cufo_printf(fos, "(%<%s (%<%s (%<%s (%<%s%>) %s%>) %s%>) %s%>)\n",
		a[0].tag, a[0].name, a[1].tag, a[1].name,
		a[2].tag, a[2].name, a[3].tag, a[3].name,
		a[2].name, a[1].name, a[0].name);
#endif
    for (i = 0; i < n; ++i) {
	cufo_puts(fos, "(");
	cufo_enter(fos, a[i].tag);
	cufo_puts(fos, a[i].name);
	cufo_puts(fos, " ");
    }
    for (i = n - 1; i >= 0; --i) {
	cufo_puts(fos, " ");
	cufo_puts(fos, a[i].name);
	cufo_leave(fos, a[i].tag);
	cufo_puts(fos, ")");
    }
    cufo_puts(fos, "  normal\n");
}

static void
_test_attrperms(cufo_stream_t fos, struct _attrperm *a, int i, int n)
{
    if (i < n) {
	int j;
	_test_attrperms(fos, a, i + 1, n);
	for (j = i + 1; j < n; ++j) {
	    _attrperm_swap(&a[i], &a[j]);
	    _test_attrperms(fos, a, i + 1, n);
	    _attrperm_swap(&a[i], &a[j]);
	}
    }
    else
	_test_attrperm(fos, a, n);
}

static void
_test_term_target(void)
{
    cufo_stream_t fos = cufo_open_term_fd("UTF-8", NULL, 1, cu_false);
    struct _attrperm a[] = {
	{cufoT_italic, "italic"},
	{cufoT_bold, "bold"},
	{cufoT_underline, "underline"},
	{cufoT_type, "type"},
    };
    cufo_enter(fos, cufoT_title);
    cufo_printf(fos, "\n%<Terminal Test%>\n\n", cufoT_title);
    cufo_leaveln(fos, cufoT_title);
    _test_attrperms(fos, a, 0, 4);
    cufo_close(fos);
}

int
main()
{
    cufo_init();
    _test_term_target();
    return 2*!!cu_test_bug_count();
}
