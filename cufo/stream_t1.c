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

#define ARRSIZE(a) (sizeof(a)/sizeof((a)[0]))

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
_test_on_off_0(cufo_stream_t fos)
{
    cufo_printf(fos,
		"→%<Lλ%>%<Rρ%>:%<%<Lλ%>%>%<%<Rρ%>%>:%<%<I%>O%>:%<O%<I%>%>\n",
		cufoT_type, cufoT_italic,
		cufoT_type, cufoT_italic, cufoT_bold, cufoT_underline,
		cufoT_type, cufoT_bold, cufoT_type, cufoT_bold);
}

static void
_test_on_off(cufo_stream_t fos, int N)
{
    char indicator[] = "nu U";
    int j = 0, n;
    for (n = 0; n < N; ++n) {
	int i = n + 1 == N? 0 : lrand48() & 3;
	if (!(i & 1) && (j & 1))
	    cufo_leave(fos, cufoT_underline);
	if ((i & 1) && !(j & 1))
	    cufo_enter(fos, cufoT_underline);
	cufo_putc(fos, indicator[i]);
	j = i;
    }
    cufo_newline(fos);
}

static void
_show_tags(cufo_stream_t fos, cufo_tag_t *tags, size_t n)
{
    int i;
    for (i = 0; i < n; ++i) {
	cufo_space(fos);
	cufo_enter(fos, tags[i]);
	cufo_puts(fos, cufo_tag_name(tags[i]));
	cufo_leave(fos, tags[i]);
    }
}

static void
_show_all_tags(cufo_stream_t fos)
{
    cufo_tag_t inline_tags[] = {
	cufoT_emph, cufoT_italic, cufoT_bold, cufoT_underline, cufoT_invalid,
    };
    cufo_tag_t code_tags[] = {
	cufoT_comment, cufoT_keyword, cufoT_label, cufoT_operator,
	cufoT_variable, cufoT_type, cufoT_literal, cufoT_special,
    };
    cufo_puts(fos, "Inline general:");
    _show_tags(fos, inline_tags, ARRSIZE(inline_tags));
    cufo_newline(fos);
    cufo_puts(fos, "Inline code:");
    _show_tags(fos, code_tags, ARRSIZE(code_tags));
}

static void
_test_term_target(void)
{
    cufo_stream_t fos = cufo_open_auto_fd(1, cu_false);
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
    _test_on_off_0(fos);
    _test_on_off(fos, 680);
    _show_all_tags(fos);
    cufo_close(fos);
}

int
main()
{
    cufo_init();
    _test_term_target();
    return 2*!!cu_test_bug_count();
}
