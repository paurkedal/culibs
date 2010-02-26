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
#include <cufo/tag.h>
#include <cufo/tagdefs.h>
#include <cufo/attrdefs.h>
#include <cuoo/intf.h>
#include <cuoo/type.h>
#include <cucon/hzmap.h>
#include <cu/debug.h>
#include <cu/diag.h>
#include <cu/va_ref.h>
#include <cu/str.h>
#include <cu/wstring.h>
#include <cu/logging.h>
#include <cu/conf.h>
#include <string.h>
#include <ctype.h>

#define BUFFER(fos) cu_to(cu_buffer, fos)
#define MAX2(i, j) (i < j? i : j)
#define MAX3(i, j, k) MAX2(i, MAX2(j, k))

#define CUFO_PRILENGTH_CHAR 1
#define CUFO_PRILENGTH_SHORT 2
#define CUFO_PRILENGTH_UNSPECIFIED 3
#define CUFO_PRILENGTH_LONG 4
#define CUFO_PRILENGTH_LONG_LONG 5
#define CUFO_PRILENGTH_INTMAX 6
#define CUFO_PRILENGTH_SIZE 7
#define CUFO_PRILENGTH_PTRDIFF 8
#define CUFO_PRILENGTH_LONG_DOUBLE 9

#define FORMAT_MAP_KEYSIZE 16
#define FORMAT_MAP_KEYSIZEW \
    ((FORMAT_MAP_KEYSIZE + sizeof(cu_word_t) - 1)/sizeof(cu_word_t))

typedef struct cufoP_tag_stack_s *cufoP_tag_stack_t;
struct cufoP_tag_stack_s
{
    cufoP_tag_stack_t next;
    cufo_tag_t tag;
};

typedef struct _format_node *_format_node_t;
struct _format_node
{
    cu_inherit (cucon_hzmap_node);
    cu_word_t wkey[FORMAT_MAP_KEYSIZEW];
    cu_bool_t is_ptr;
    union {
	cufo_print_ptr_fn_t print_ptr;
	cufo_print_fn_t print_va;
    } u0;
};

static struct cucon_hzmap format_ptr_map;
static struct cucon_hzmap format_va_map;

static _format_node_t
insert_format(char const *key)
{
    cucon_hzmap_node_t node;
    cu_word_t wkey[FORMAT_MAP_KEYSIZEW];
    size_t len = strlen(key);
    if (len > FORMAT_MAP_KEYSIZE)
	cu_bugf("Format key '%s' is too long, max length is %d",
		key, FORMAT_MAP_KEYSIZE);
    memset(wkey, 0, sizeof(wkey));
    memcpy(wkey, key, len);
    if (!cucon_hzmap_insert(&format_ptr_map, wkey,
			    sizeof(struct _format_node), &node))
	cu_bugf("Format key '%s' has already been registered.", key);
    return cu_from(_format_node, cucon_hzmap_node, node);
}

void
cufo_register_ptr_format(char const *key, cufo_print_ptr_fn_t handler)
{
    _format_node_t node = insert_format(key);
    node->is_ptr = cu_true;
    node->u0.print_ptr = handler;
}

void
cufo_register_va_format(char const *key, cufo_print_fn_t handler)
{
    _format_node_t node = insert_format(key);
    node->is_ptr = cu_false;
    node->u0.print_va = handler;
}

static _format_node_t
lookup_format(char const *key, size_t key_len)
{
    cucon_hzmap_node_t node;
    cu_word_t wkey[FORMAT_MAP_KEYSIZEW];
    if (key_len > FORMAT_MAP_KEYSIZE)
	return NULL;
    memset(wkey, 0, sizeof(wkey));
    memcpy(wkey, key, key_len);
    node = cucon_hzmap_find(&format_ptr_map, wkey);
    if (node)
	return cu_from(_format_node, cucon_hzmap_node, node);
    else
	return NULL;
}

static void
get_buffer_cap(cufo_stream_t fos, int max_width,
	       char **buf_out, size_t *cap_out)
{
    /* Extend buffer capacity if needed to avoid truncation. */
    cufo_set_wide(fos, cu_false);
    cu_buffer_extend_freecap(cu_to(cu_buffer, fos), max_width + 1);
    *cap_out = cu_buffer_freecap(cu_to(cu_buffer, fos));
    *buf_out = cu_buffer_content_end(cu_to(cu_buffer, fos));
}

void (*cufoP_print_nonobj)(cufo_stream_t, cufo_prispec_t, cuex_t) = NULL;

void
cufo_printsp_ex(cufo_stream_t fos, cufo_prispec_t spec, cuex_t e)
{
    cuex_meta_t e_meta;
    cuoo_type_t e_type;
    e_meta = cuex_meta(e);
    if (cuex_meta_is_type(e_meta)) {
	cufo_print_ptr_fn_t print;
	cu_str_t (*to_str)(void *);
	e_type = cuoo_type_from_meta(e_meta);

	if ((print = cuoo_type_impl_fptr(cufo_print_ptr_fn_t, e_type,
					 CUOO_INTF_FOPRINT_FN))) {
	    (*print)(fos, spec, e);
	    return;
	}

	if ((to_str = cuoo_type_impl_fptr(cuoo_intf_to_str_fn_t, e_type,
					  CUOO_INTF_TO_STR_FN))) {
	    cufo_enter(fos, cufoT_literal);
	    cufo_print_str(fos, (*to_str)(e));
	    cufo_leave(fos, cufoT_literal);
	    return;
	}

	if (fos->flags & CUFO_SFLAG_SHOW_TYPE_IF_UNPRINTABLE) {
	    fos->flags &= ~CUFO_SFLAG_SHOW_TYPE_IF_UNPRINTABLE;
	    cufo_printf(fos, "__obj_%p[: %!]", e, e_type);
	    fos->flags |= CUFO_SFLAG_SHOW_TYPE_IF_UNPRINTABLE;
	}
	else
	    cufo_printf(fos, "__obj_%p", e);
    }
    else if (cufoP_print_nonobj)
	(*cufoP_print_nonobj)(fos, spec, e);
    else
	cufo_printf(fos, "__ex_%p", e);
}

void
cufo_print_ex(cufo_stream_t fos, cuex_t e)
{
    struct cufo_prispec spec;
    spec.flags = 0;
    spec.width = 0;
    spec.precision = -1;
    cufo_printsp_ex(fos, &spec, e);
}

static char const *
handle_format(cufo_stream_t fos, char const *fmt, cu_va_ref_t va_ref,
	      size_t old_size, cufoP_tag_stack_t *tag_stack)
{
    char new_fmt[13] = "%";
    int i_new_fmt = 1;
    unsigned int flags = 0;
    int length, width, prec, max_width;
    char *outbuf;
    size_t outcap;

    /* Flags */
    for (;; ++fmt) {
	switch (*fmt) {
	    case '+': flags |= CUFO_PRIFLAG_PLUS;  break;
	    case '-': flags |= CUFO_PRIFLAG_MINUS; break;
	    case ' ': flags |= CUFO_PRIFLAG_SPACE; break;
	    case '#': flags |= CUFO_PRIFLAG_HASH;  break;
	    case '0': flags |= CUFO_PRIFLAG_ZERO;  break;
	    default: goto break_flags;
	}
	if (i_new_fmt < 6)
	    new_fmt[i_new_fmt++] = *fmt;
    }
break_flags:

    /* Width */
    if (isdigit(*fmt))
	width = strtol(fmt, (char **)&fmt, 10);
    else if (*fmt == '*')
	width = cu_va_ref_arg(va_ref, int);
    else
	width = 0;
    new_fmt[i_new_fmt++] = '*';
    cu_debug_assert(i_new_fmt < 7);

    /* Precision */
    if (*fmt == '.') {
	++fmt;
	if (isdigit(*fmt))
	    prec = strtol(fmt, (char **)&fmt, 10);
	else if (*fmt == '*')
	    prec = cu_va_ref_arg(va_ref, int);
	else
	    prec = 0;
	new_fmt[i_new_fmt++] = '.';
	new_fmt[i_new_fmt++] = '*';
    } else
	prec = -1;
    cu_debug_assert(i_new_fmt < 9);

    /* Length modifier */
    length = CUFO_PRILENGTH_UNSPECIFIED;
    switch (*fmt) {
	case 'h':
	    new_fmt[i_new_fmt++] = 'h';
	    ++fmt;
	    if (*fmt == 'h') {
		new_fmt[i_new_fmt++] = 'h';
		++fmt;
		length = CUFO_PRILENGTH_CHAR;
	    } else
		length = CUFO_PRILENGTH_SHORT;
	    break;
	case 'l':
	    new_fmt[i_new_fmt++] = 'l';
	    ++fmt;
	    if (*fmt == 'l') {
		new_fmt[i_new_fmt++] = 'l';
		++fmt;
		length = CUFO_PRILENGTH_LONG_LONG;
	    } else
		length = CUFO_PRILENGTH_LONG;
	    break;
	case 'j':
	    new_fmt[i_new_fmt++] = 'j';
	    ++fmt;
	    length = CUFO_PRILENGTH_INTMAX;
	    break;
	case 'z':
	    new_fmt[i_new_fmt++] = 'z';
	    ++fmt;
	    length = CUFO_PRILENGTH_SIZE;
	    break;
	case 't':
	    new_fmt[i_new_fmt++] = 't';
	    ++fmt;
	    length = CUFO_PRILENGTH_PTRDIFF;
	    break;
	case 'L':
	    new_fmt[i_new_fmt++] = 'L';
	    ++fmt;
	    length = CUFO_PRILENGTH_LONG_DOUBLE;
	    break;
	default:
	    break;
    }
    cu_debug_assert(i_new_fmt < 11);

    /* Conversion specifier */
    new_fmt[i_new_fmt++] = *fmt;
    new_fmt[i_new_fmt++] = 0;
    cu_debug_assert(i_new_fmt < 13);
    switch (*fmt++) {
	case 'd': case 'i':
	case 'o': case 'u': case 'x': case 'X':
	    max_width = sizeof(long int)*3 + 2; /* -0777 or -999 or -0xff */
	    if (max_width < width)
		max_width = width;
	    if (max_width < prec + 1)
		max_width = prec + 1;
	    get_buffer_cap(fos, max_width, &outbuf, &outcap);
	    switch (length) {
		case CUFO_PRILENGTH_CHAR:
		    if (prec == -1)
			snprintf(outbuf, outcap, new_fmt, width,
				 cu_va_ref_arg(va_ref, int));
		    else
			snprintf(outbuf, outcap, new_fmt, width, prec,
				 cu_va_ref_arg(va_ref, int));
		    break;
		case CUFO_PRILENGTH_SHORT:
		    if (prec == -1)
			snprintf(outbuf, outcap, new_fmt, width,
				 cu_va_ref_arg(va_ref, int));
		    else
			snprintf(outbuf, outcap, new_fmt, width, prec,
				 cu_va_ref_arg(va_ref, int));
		    break;
		case CUFO_PRILENGTH_UNSPECIFIED:
		    if (prec == -1)
			snprintf(outbuf, outcap, new_fmt, width,
				 cu_va_ref_arg(va_ref, int));
		    else
			snprintf(outbuf, outcap, new_fmt, width, prec,
				 cu_va_ref_arg(va_ref, int));
		    break;
		case CUFO_PRILENGTH_LONG:
		    if (prec == -1)
			snprintf(outbuf, outcap, new_fmt, width,
				 cu_va_ref_arg(va_ref, long));
		    else
			snprintf(outbuf, outcap, new_fmt, width, prec,
				 cu_va_ref_arg(va_ref, long));
		    break;
		case CUFO_PRILENGTH_LONG_LONG:
		    if (prec == -1)
			snprintf(outbuf, outcap, new_fmt, width,
				 cu_va_ref_arg(va_ref, long long));
		    else
			snprintf(outbuf, outcap, new_fmt, width, prec,
				 cu_va_ref_arg(va_ref, long long));
		    break;
		case CUFO_PRILENGTH_INTMAX:
		    if (prec == -1)
			snprintf(outbuf, outcap, new_fmt, width,
				 cu_va_ref_arg(va_ref, intmax_t));
		    else
			snprintf(outbuf, outcap, new_fmt, width, prec,
				 cu_va_ref_arg(va_ref, intmax_t));
		    break;
		case CUFO_PRILENGTH_SIZE:
		    if (prec == -1)
			snprintf(outbuf, outcap, new_fmt, width,
				 cu_va_ref_arg(va_ref, size_t));
		    else
			snprintf(outbuf, outcap, new_fmt, width, prec,
				 cu_va_ref_arg(va_ref, size_t));
		    break;
		case CUFO_PRILENGTH_PTRDIFF:
		    if (prec == -1)
			snprintf(outbuf, outcap, new_fmt, width, prec,
				 cu_va_ref_arg(va_ref, ptrdiff_t));
		    else
			snprintf(outbuf, outcap, new_fmt, width, prec,
				 cu_va_ref_arg(va_ref, ptrdiff_t));
		    break;
		default:
		    cu_bugf("Invalid length modifier for integer format.");
	    }
	    break;
	case 'f': case 'F':
	case 'e': case 'E':
	case 'g': case 'G':
	case 'a': case 'A':
	    max_width = sizeof(long double)*3 + 9; /* -0.0009999e-99 */
	    if (max_width < width)
		max_width = width;
	    if (max_width < prec + 7)
		max_width = prec + 7;
	    get_buffer_cap(fos, max_width, &outbuf, &outcap);
	    switch (length) {
		case CUFO_PRILENGTH_UNSPECIFIED:
		case CUFO_PRILENGTH_LONG:
		    if (prec == -1)
			snprintf(outbuf, outcap, new_fmt, width,
				 cu_va_ref_arg(va_ref, double));
		    else
			snprintf(outbuf, outcap, new_fmt, width, prec,
				 cu_va_ref_arg(va_ref, double));
		    break;
		case CUFO_PRILENGTH_LONG_DOUBLE:
		    if (prec == -1)
			snprintf(outbuf, outcap, new_fmt, width,
				 cu_va_ref_arg(va_ref, long double));
		    else
			snprintf(outbuf, outcap, new_fmt, width, prec,
				 cu_va_ref_arg(va_ref, long double));
		    break;
		default:
		    cu_bugf("Invalid length modifier for float format.");
	    }
	    break;
	case 'p':
	    max_width = sizeof(void *)*2 + 2;
	    if (max_width < width)
		max_width = width;
	    get_buffer_cap(fos, max_width, &outbuf, &outcap);
	    switch (length) {
		case CUFO_PRILENGTH_UNSPECIFIED:
		    if (prec == -1)
			snprintf(outbuf, outcap, new_fmt, width,
				 cu_va_ref_arg(va_ref, void *));
		    else
			cu_bugf("Invalid precision on pointer format "
				"specifier.");
		    break;
		default:
		    cu_bugf("Invalid length modifier for pointer format.");
	    }
	    break;
	case 'c':
	    switch (length) {
		case CUFO_PRILENGTH_UNSPECIFIED: {
		    char ch = cu_va_ref_arg(va_ref, int);
		    if (width <= 1)
			cufo_putc(fos, ch);
		    else if (flags & CUFO_PRIFLAG_MINUS) {
			cufo_putc(fos, ch);
			cufo_fillc(fos, ' ', width - 1);
		    } else {
			cufo_fillc(fos, ' ', width - 1);
			cufo_putc(fos, ch);
		    }
		    break;
		}
		case CUFO_PRILENGTH_LONG:
		    /* TODO. If cu_wint_t is different from wint_t either in
		     * size or enumeration, deal with wint_t here. */
		case CUFO_PRILENGTH_INTMAX: {
		    cu_wchar_t wc = cu_va_ref_arg(va_ref, cu_wint_t);
		    if (width <= 1)
			cufo_putwc(fos, wc);
		    else if (flags & CUFO_PRIFLAG_MINUS) {
			cufo_putwc(fos, wc);
			cufo_fillc(fos, ' ', width - 1);
		    } else {
			cufo_fillc(fos, ' ', width - 1);
			cufo_putwc(fos, wc);
		    }
		    break;
		}
		default:
		    cu_bugf("Invalid length modifier for character format.");
	    }
	    return fmt;
	case 's':
	    switch (length) {
		case CUFO_PRILENGTH_UNSPECIFIED: {
		    char *arr = cu_va_ref_arg(va_ref, char *);
		    size_t len = strlen(arr);
		    if (len == 0 && width == 0 && (flags & CUFO_PRIFLAG_SPACE))
			width = 1;
		    if (prec != -1 && len > prec)
			len = prec;
		    if (len >= width)
			cufo_print_charr(fos, arr, len);
		    else if (flags & CUFO_PRIFLAG_MINUS) {
			cufo_print_charr(fos, arr, len);
			cufo_fillc(fos, ' ', width - len);
		    }
		    else {
			cufo_fillc(fos, ' ', width - len);
			cufo_print_charr(fos, arr, len);
		    }
		    break;
		}
		case CUFO_PRILENGTH_LONG:
		    /* TODO. If cu_wint_t is different from wint_t either in
		     * size or enumeration, deal with wint_t here. */
		case CUFO_PRILENGTH_INTMAX: {
		    cu_wchar_t *arr = cu_va_ref_arg(va_ref, cu_wchar_t *);
		    size_t len = cu_wcslen(arr);
		    if (len == 0 && width == 0 && (flags & CUFO_PRIFLAG_SPACE))
			width = 1;
		    if (prec != -1 && len > prec)
			len = prec;
		    if (len >= width)
			cufo_print_wcarr(fos, arr, len);
		    else if (flags & CUFO_PRIFLAG_MINUS) {
			cufo_print_wcarr(fos, arr, len);
			cufo_fillwc(fos, ' ', width - len);
		    }
		    else {
			cufo_fillwc(fos, ' ', width - len);
			cufo_print_wcarr(fos, arr, len);
		    }
		    break;
		}
		default:
		    cu_bugf("Invalid length modifier for string format.");
	    }
	    return fmt;
	case 'n': {
	    size_t pos;
	    pos = cu_buffer_content_size(cu_to(cu_buffer, fos)) - old_size;
	    if (flags != 0 || width != -1 ||
		prec != -1)
		cu_bugf("Flags, width, and precisions is invalid for position "
			"write-back specifier.");
	    switch (length) {
		case CUFO_PRILENGTH_CHAR:
		    *cu_va_ref_arg(va_ref, signed char *) = pos;
		    break;
		case CUFO_PRILENGTH_SHORT:
		    *cu_va_ref_arg(va_ref, short *) = pos;
		    break;
		case CUFO_PRILENGTH_UNSPECIFIED:
		    *cu_va_ref_arg(va_ref, int *) = pos;
		    break;
		case CUFO_PRILENGTH_LONG:
		    *cu_va_ref_arg(va_ref, long *) = pos;
		    break;
		case CUFO_PRILENGTH_LONG_LONG:
		    *cu_va_ref_arg(va_ref, long long *) = pos;
		    break;
		default:
		    cu_bugf("Invalid length modifier for position write-back.");
	    }
	    return fmt;
	}
	case '&': {
	    cufo_print_fn_t f = cu_va_ref_arg(va_ref, cufo_print_fn_t);
	    struct cufo_prispec spec;
	    spec.flags = flags;
	    spec.width = width;
	    spec.precision = prec;
	    (*f)(fos, &spec, va_ref);
	    return fmt;
	}
	case '!': {
	    struct cufo_prispec spec;
	    cuex_t e = cu_va_ref_arg(va_ref, cuex_t);
	    spec.flags = flags;
	    spec.width = width;
	    spec.precision = prec;
	    if (e == NULL) {
		cufo_enter(fos, cufoT_literal);
		cufo_puts(fos, "NULL");
		cufo_leave(fos, cufoT_literal);
	    }
	    else
		cufo_printsp_ex(fos, &spec, e);
	    return fmt;
	}
	case '(': {
	    char const *s = fmt;
	    _format_node_t _format_node;
	    while (*fmt && *fmt != ')') ++fmt;
	    _format_node = lookup_format(s, fmt - s);
	    if (_format_node) {
		struct cufo_prispec spec;
		spec.flags = flags;
		spec.width = width;
		spec.precision = prec;
		if (_format_node->is_ptr) {
		    void *ptr = cu_va_ref_arg(va_ref, void *);
		    (*_format_node->u0.print_ptr)(fos, &spec, ptr);
		} else
		    (*_format_node->u0.print_va)(fos, &spec, va_ref);
	    }
	    else {
		char *sp = cu_salloc(fmt - s + 1);
		strncpy(sp, s, fmt - s + 1); sp[fmt - s] = 0;
		cu_bugf("Unknown format specifier %s.", sp);
	    }
	    return ++fmt;
	}
	case '<': {
	    cufoP_tag_stack_t tag_node;
	    cufo_tag_t tag = cu_va_ref_arg(va_ref, cufo_tag_t);
	    cufo_enter(fos, tag);
	    tag_node = cu_gnew(struct cufoP_tag_stack_s);
	    tag_node->next = *tag_stack;
	    tag_node->tag = tag;
	    *tag_stack = tag_node;
	    return fmt;
	}
	case '>': {
	    if (!*tag_stack)
		cu_bugf("Missing start tag to match '%%>' in format.");
	    cufo_leave(fos, (*tag_stack)->tag);
	    *tag_stack = (*tag_stack)->next;
	    return fmt;
	}
	case '%':
	    cufo_putc(fos, '%');
	    return fmt;
	default:
	    cu_bugf("Invalid format specifier %%%c.", fmt[-1]);
	    return fmt;
    }
    /* Only formats using outbuf shall reach here. */
    cu_buffer_set_content_end(BUFFER(fos), outbuf + strlen(outbuf));
    return fmt;
}

int
cufo_vprintf(cufo_stream_t fos, char const *fmt, va_list va)
{
    char const *fmt_last = fmt;
    size_t old_size = cu_buffer_content_size(cu_to(cu_buffer, fos));
    int write_count = 0;
    cufoP_tag_stack_t tag_stack = NULL;
    while (*fmt) {
	if (*fmt == '%') {
	    if (fmt != fmt_last) {
		size_t count = cu_ptr_diff(fmt, fmt_last);
		cufo_print_charr(fos, fmt_last, count);
		write_count += count;
	    }
	    ++fmt;
	    fmt = handle_format(fos, fmt, cu_va_ref_of_va_list(va), old_size,
				&tag_stack);
	    fmt_last = fmt;
	}
	else
	    ++fmt;
    }
    if (fmt != fmt_last) {
	size_t count = cu_ptr_diff(fmt, fmt_last);
	cufo_print_charr(fos, fmt_last, count);
	write_count += count;
    }
    if (tag_stack)
	cu_bugf("No closing '%%>' for '%%<' format specifier of tag %s.",
		cufo_tag_name(tag_stack->tag));
    return write_count;
}

int
cufo_printf(cufo_stream_t fos, char const *fmt, ...)
{
    int write_count;
    va_list va;
    va_start(va, fmt);
    write_count = cufo_vprintf(fos, fmt, va);
    va_end(va);
    return write_count;
}

int
cufo_printfln(cufo_stream_t fos, char const *fmt, ...)
{
    int write_count;
    va_list va;
    va_start(va, fmt);
    write_count = cufo_vprintf(fos, fmt, va);
    va_end(va);
    cufo_newline(fos);
    return write_count;
}

int
cufo_lprintf(cufo_stream_t fos, char const *fmt, ...)
{
    int write_count;
    va_list va;
    cufo_lock(fos);
    va_start(va, fmt);
    write_count = cufo_vprintf(fos, fmt, va);
    va_end(va);
    cufo_unlock(fos);
    return write_count;
}

int
cufo_oprintf(char const *fmt, ...)
{
    int write_count;
    va_list va;
    cufo_lock(cufo_stdout);
    va_start(va, fmt);
    write_count = cufo_vprintf(cufo_stdout, fmt, va);
    va_end(va);
    cufo_flush(cufo_stdout);
    cufo_unlock(cufo_stdout);
    return write_count;
}

int
cufo_eprintf(char const *fmt, ...)
{
    int write_count;
    va_list va;
    cufo_lock(cufo_stderr);
    va_start(va, fmt);
    write_count = cufo_vprintf(cufo_stderr, fmt, va);
    va_end(va);
    cufo_flush(cufo_stderr);
    cufo_unlock(cufo_stderr);
    return write_count;
}

void
cufo_vlogf_at(cufo_stream_t fos, cu_log_facility_t facility,
	      cu_sref_t loc, char const *fmt, va_list va)
{
    cufo_entera(fos, cufoT_logentry,
		cufoA_logorigin(cu_log_facility_origin(facility)),
		cufoA_logseverity(cu_log_facility_severity(facility)));
    if (facility->flags & CU_LOG_FLAG_DEBUG_FACILITY) {
	char const *file = va_arg(va, char const *);
	int line = va_arg(va, int);
	if (fmt[0] == '%' && fmt[1] == '~' && fmt[2] == ':')
	    fmt += 3;
	else
	    cufo_printf(fos, "%<%s:%d: %>", cufoT_location, file, line);
    }
    if (loc) {
	cufo_enter(fos, cufoT_location);
	cufo_print_sref(fos, loc);
	cufo_puts(fos, ": ");
	cufo_leave(fos, cufoT_location);
    }
    if (fmt[0] == '%' && fmt[1] == ':') {
	fmt += 2;
	while (isspace(*fmt)) ++fmt;
	cufo_enter(fos, cufoT_location);
	cufo_print_sref(fos, va_arg(va, cu_sref_t));
	cufo_puts(fos, ": ");
	cufo_leave(fos, cufoT_location);
    }
    cufo_enter(fos, cufoT_message);
    cufo_vprintf(fos, fmt, va);
    cufo_leave(fos, cufoT_message);
    cufo_leaveln(fos, cufoT_logentry);
    cufo_flush(fos);
}

void
cufo_vlogf(cufo_stream_t fos, cu_log_facility_t facility,
	   char const *fmt, va_list va)
{
    cufo_vlogf_at(fos, facility, NULL, fmt, va);
}

void
cufo_logf(cufo_stream_t fos, cu_log_facility_t facility, char const *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    cufo_vlogf(fos, facility, fmt, va);
    va_end(va);
}

#ifdef CUCONF_HAVE_FILENO
static void
_vfprintf(FILE *file, char const *fmt, va_list vl)
{
    cufo_stream_t fos;
    fflush(file);
    fos = cufo_open_strip_fd(NULL, fileno(file), cu_false);
    cufo_vprintf(fos, fmt, vl);
    cufo_close(fos);
}
extern void (*cuP_vfprintf)(FILE *, char const *, va_list);
#endif

void
cufoP_printf_init(void)
{
    cucon_hzmap_init(&format_ptr_map, FORMAT_MAP_KEYSIZEW);
    cucon_hzmap_init(&format_va_map, FORMAT_MAP_KEYSIZEW);
#ifdef CUCONF_HAVE_FILENO
    cuP_vfprintf = _vfprintf;
#endif
}
