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

#include <cufo/stream.h>
#include <cufo/tagdefs.h>

static void
print_wstring(cufo_stream_t fos, cufo_prispec_t spec, void *p)
{
    cufo_print_wstring(fos, p);
}

static void
print_str(cufo_stream_t fos, cufo_prispec_t spec, void *p)
{
    cufo_print_str(fos, p);
}

static void
print_script(cufo_stream_t fos, cufo_prispec_t spec, long x, cufo_tag_t tag,
	     char const **digits, char const *minus)
{
    if (cufo_enter(fos, tag))
	cufo_printf(fos, "%ld", x);
    else {
	char buf[sizeof(long)*3 + 2], *s = buf;
	if (x < 0) {
	    cufo_puts(fos, minus);
	    x = -x;
	}
	sprintf(buf, "%ld", x);
	while (*s) {
	    char ch = *s++;
	    cu_debug_assert('0' <= ch && ch <= '9');
	    cufo_puts(fos, digits[ch - '0']);
	}
    }
    cufo_leave(fos, tag);
}

static void
print_d_sub(cufo_stream_t fos, cufo_prispec_t spec, cu_va_ref_t va_ref)
{
    int x = cu_va_ref_arg(va_ref, int);
    static char const *digits[] = {
	"₀", "₁", "₂", "₃", "₄", "₅", "₆", "₇", "₈", "₉"
    };
    print_script(fos, spec, x, cufoT_subscript, digits, "₋");
}

static void
print_d_sup(cufo_stream_t fos, cufo_prispec_t spec, cu_va_ref_t va_ref)
{
    int x = cu_va_ref_arg(va_ref, int);
    static char const *digits[] = {
	"⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹"
    };
    print_script(fos, spec, x, cufoT_superscript, digits, "⁻");
}

static void
print_ld_sub(cufo_stream_t fos, cufo_prispec_t spec, cu_va_ref_t va_ref)
{
    long x = cu_va_ref_arg(va_ref, long);
    static char const *digits[] = {
	"₀", "₁", "₂", "₃", "₄", "₅", "₆", "₇", "₈", "₉"
    };
    print_script(fos, spec, x, cufoT_subscript, digits, "₋");
}

static void
print_ld_sup(cufo_stream_t fos, cufo_prispec_t spec, cu_va_ref_t va_ref)
{
    long x = cu_va_ref_arg(va_ref, long);
    static char const *digits[] = {
	"⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹"
    };
    print_script(fos, spec, x, cufoT_superscript, digits, "⁻");
}

void
cufoP_init_formats()
{
    cufo_register_ptr_format("wstring", print_wstring);
    cufo_register_ptr_format("str", print_str);
    cufo_register_va_format("d/sub", print_d_sub);
    cufo_register_va_format("d/sup", print_d_sup);
    cufo_register_va_format("ld/sub", print_ld_sub);
    cufo_register_va_format("ld/sup", print_ld_sup);
}
