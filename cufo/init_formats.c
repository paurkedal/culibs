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
#include <cutext/wctype.h>
#include <cucon/ucset.h>
#include <cu/wstring.h>
#include <cu/str.h>
#include <ctype.h>
#include <inttypes.h>

static void
_print_wstring(cufo_stream_t fos, cufo_prispec_t spec, void *p)
{
    cufo_print_wstring(fos, p);
}

static void
_print_str(cufo_stream_t fos, cufo_prispec_t spec, void *p)
{
    cufo_print_str(fos, p);
}

static void
_print_bool(cufo_stream_t fos, cufo_prispec_t spec, cu_va_ref_t va_ref)
{
    cu_bool_t x = cu_va_ref_arg(va_ref, cu_bool_t);
    cufo_puts(fos, x? "true" : "false");
}

static void
_print_script(cufo_stream_t fos, cufo_prispec_t spec, long x, cufo_tag_t tag,
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

static char const *_sub_digits[] = {
    "₀", "₁", "₂", "₃", "₄", "₅", "₆", "₇", "₈", "₉"
};

static char const *_sup_digits[] = {
    "⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹"
};

static void
_print_d_sub(cufo_stream_t fos, cufo_prispec_t spec, cu_va_ref_t va_ref)
{
    int x = cu_va_ref_arg(va_ref, int);
    _print_script(fos, spec, x, cufoT_subscript, _sub_digits, "₋");
}

static void
_print_d_sup(cufo_stream_t fos, cufo_prispec_t spec, cu_va_ref_t va_ref)
{
    int x = cu_va_ref_arg(va_ref, int);
    _print_script(fos, spec, x, cufoT_superscript, _sup_digits, "⁻");
}

static void
_print_ld_sub(cufo_stream_t fos, cufo_prispec_t spec, cu_va_ref_t va_ref)
{
    long x = cu_va_ref_arg(va_ref, long);
    _print_script(fos, spec, x, cufoT_subscript, _sub_digits, "₋");
}

static void
_print_ld_sup(cufo_stream_t fos, cufo_prispec_t spec, cu_va_ref_t va_ref)
{
    long x = cu_va_ref_arg(va_ref, long);
    _print_script(fos, spec, x, cufoT_superscript, _sup_digits, "⁻");
}

static void
_wstring_foprint(cufo_stream_t fos, cufo_prispec_t spec, void *ptr)
{
    int i, n = cu_wstring_length(ptr);
    cufo_putwc(fos, 0x75); /* u */
    cufo_putwc(fos, 0x22); /* " */
    for (i = 0; i < n; ++i) {
	cu_wint_t ch = cu_wstring_at(ptr, i);
	if (ch == 0x5c || ch == 0x22) { /* backslash or solidus */
	    cufo_putwc(fos, 0x5c);
	    cufo_putwc(fos, ch);
	}
	else if (!cutext_iswprint(ch))
	    switch (ch) {
		case 0x09: cufo_puts(fos, "\\t"); break;
		case 0x0a: cufo_puts(fos, "\\n"); break;
		case 0x0b: cufo_puts(fos, "\\v"); break;
		default:
		    if (ch < 0x10000)
			cufo_printf(fos, "\\u%04x", ch);
		    else
			cufo_printf(fos, "\\U%06x", ch);
		    break;
	    }
	else
	    cufo_putwc(fos, ch);
    }
    cufo_putwc(fos, 0x22);
}

static void
_str_foprint(cufo_stream_t fos, cufo_prispec_t spec, void *ptr)
{
    int i, n = cu_str_size(ptr);
    cufo_putc(fos, '"');
    for (i = 0; i < n; ++i) {
	char ch = cu_str_at(ptr, i);
	if (ch == '\\' || ch == '"') {
	    cufo_putc(fos, '\\');
	    cufo_putc(fos, ch);
	}
	else if ((unsigned char)ch < 0x80 && !isprint(ch))
	    switch (ch) {
		case '\t': cufo_puts(fos, "\\t"); break;
		case '\n': cufo_puts(fos, "\\n"); break;
		case '\v': cufo_puts(fos, "\\v"); break;
		default:
		    cufo_printf(fos, "\\x%02x", (unsigned char)ch);
		    break;
	    }
	else
	    cufo_putc(fos, ch);
    }
    cufo_putc(fos, '"');
}

cu_clos_def(_ucset_foprint_item, cu_prot(void, uintptr_t key),
    ( cufo_stream_t fos; size_t count; ))
{
    cu_clos_self(_ucset_foprint_item);
    if (self->count++)
	cufo_puts(self->fos, ", ");
    cufo_printf(self->fos, "%"PRIdPTR, key);
}

static void
_ucset_foprint(cufo_stream_t fos, cufo_prispec_t spec, void *ptr)
{
    _ucset_foprint_item_t cb;
    cb.fos = fos;
    cb.count = 0;
    cufo_putc(fos, '{');
    cucon_ucset_iter(ptr, _ucset_foprint_item_prep(&cb));
    cufo_putc(fos, '}');
}

extern cu_box_t cuP_wstring_foprint;
extern cu_box_t cuP_str_foprint;
extern cu_box_t cuconP_ucset_foprint;

void
cufoP_init_formats()
{
    cufo_register_ptr_format("wstring", _print_wstring);
    cufo_register_ptr_format("wstring/r", _wstring_foprint);
    cufo_register_ptr_format("str", _print_str);
    cufo_register_ptr_format("str/r", _str_foprint);
    cufo_register_va_format("bool", _print_bool);
    cufo_register_va_format("d/sub", _print_d_sub);
    cufo_register_va_format("d/sup", _print_d_sup);
    cufo_register_va_format("ld/sub", _print_ld_sub);
    cufo_register_va_format("ld/sup", _print_ld_sup);

    cuP_wstring_foprint = cu_box_fptr(cufo_print_ptr_fn_t, _wstring_foprint);
    cuP_str_foprint = cu_box_fptr(cufo_print_ptr_fn_t, _str_foprint);
    cuconP_ucset_foprint = cu_box_fptr(cufo_print_ptr_fn_t, _ucset_foprint);
}
