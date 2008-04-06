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

#include <cufo/stream.h>
#include <cufo/tag.h>
#include <cu/wstring.h>
#include <cu/str.h>
#include <string.h>

#ifdef CUCONF_DEBUG_SELF
#  define INIT_BUFFER_CAP 16
#else
#  define INIT_BUFFER_CAP 512
#endif
#define BUFFER(fos) cu_to(cu_buffer, fos)

#ifdef CUCONF_DEBUG_CLIENT
typedef struct cufoP_tag_stack_s *cufoP_tag_stack_t;
struct cufoP_tag_stack_s
{
    cufoP_tag_stack_t next;
    cufo_tag_t tag;
};
#endif


void
cufo_stream_init(cufo_stream_t fos, cufo_target_t target)
{
    cu_buffer_init(BUFFER(fos), INIT_BUFFER_CAP);
    fos->target = target;
    fos->is_wide = cu_false;
    fos->flags = 0;
#ifdef CUCONF_DEBUG_CLIENT
    fos->tag_stack = NULL;
#endif
}

void *
cufo_close(cufo_stream_t fos)
{
#ifdef CUCONF_DEBUG_CLIENT
    if (!cufo_have_error(fos) && fos->tag_stack != NULL)
	cu_bugf("Missing closing tag %s at end of stream.",
		fos->tag_stack->tag);
#endif
    (*fos->target->flush)(fos, cu_true);
    return (*fos->target->close)(fos);
}

void
cufo_close_discard(cufo_stream_t fos)
{
    cufo_flag_error(fos);
    (*fos->target->close)(fos);
}

void
cufo_flush(cufo_stream_t fos)
{
    if (cu_buffer_content_size(BUFFER(fos)) > 0) {
	(*fos->target->flush)(fos, cu_false);
	if (cu_buffer_content_size(BUFFER(fos)) == 0)
	    cu_buffer_clear(BUFFER(fos)); /* Good place to realign content. */
    }
}

void
cufoP_set_wide(cufo_stream_t fos, cu_bool_t is_wide)
{
    if (cu_buffer_content_size(BUFFER(fos)) > 0) {
	(*fos->target->flush)(fos, cu_true);
	if (cu_buffer_content_size(BUFFER(fos)) == 0)
	    cu_buffer_clear(BUFFER(fos)); /* Good place to realign content. */
	else
	    cu_bugf("The cufo target did not fully flush the buffer when "
		    "switching character width.  This may be due to an "
		    "incomplete multibyte sequence preceding a wide "
		    "character in the application, or a bug in the "
		    "cufo_target implementation.");
    }
    fos->is_wide = is_wide;
}

void
cufo_putc(cufo_stream_t fos, char ch)
{
    cufo_set_wide(fos, cu_false);
    cufo_fast_putc(fos, ch);
}

void
cufo_putwc(cufo_stream_t fos, cu_wchar_t wc)
{
    cufo_set_wide(fos, cu_true);
    cufo_fast_putwc(fos, wc);
}

void
cufo_fillc(cufo_stream_t fos, char ch, int repeat)
{
    char *buf;
    cufo_set_wide(fos, cu_false);
    buf = cu_buffer_produce(BUFFER(fos), repeat);
    memset(buf, ch, repeat);
}

void
cufo_fillwc(cufo_stream_t fos, cu_wchar_t wc, int repeat)
{
    cu_wchar_t *buf;
    cufo_set_wide(fos, cu_true);
    buf = cu_buffer_produce(BUFFER(fos), repeat*sizeof(cu_wchar_t));
    while (repeat-- > 0)
	*buf++ = wc;
}

void
cufo_print_charr(cufo_stream_t fos, char const *charr, size_t size)
{
    void *buf;
    cufo_set_wide(fos, cu_false);
    buf = cu_buffer_produce(BUFFER(fos), size);
    memcpy(buf, charr, size);
}

void
cufo_print_wcarr(cufo_stream_t fos, cu_wchar_t const *wcarr, size_t count)
{
    void *buf;
    cufo_set_wide(fos, cu_true);
    buf = cu_buffer_produce(BUFFER(fos), count*sizeof(cu_wchar_t));
    memcpy(buf, wcarr, count*sizeof(cu_wchar_t));
}

void
cufo_puts(cufo_stream_t fos, char const *cs)
{
    cufo_print_charr(fos, cs, strlen(cs));
}

void
cufo_print_wstring(cufo_stream_t fos, cu_wstring_t ws)
{
    cufo_print_wcarr(fos, cu_wstring_array(ws), cu_wstring_length(ws));
}

void
cufo_print_str(cufo_stream_t fos, cu_str_t str)
{
    cufo_print_charr(fos, cu_str_charr(str), cu_str_size(str));
}

void
cufo_enter_va(cufo_stream_t fos, cufo_tag_t tag, va_list va)
{
#ifdef CUCONF_DEBUG_CLIENT
    cufoP_tag_stack_t tag_stack = cu_gnew(struct cufoP_tag_stack_s);
    tag_stack->next = fos->tag_stack;
    tag_stack->tag = tag;
    fos->tag_stack = tag_stack;
#endif
    (*fos->target->enter)(fos, tag, va);
}

void
cufo_enter(cufo_stream_t fos, cufo_tag_t tag, ...)
{
#ifdef CUCONF_DEBUG_CLIENT
    cufoP_tag_stack_t tag_stack = cu_gnew(struct cufoP_tag_stack_s);
    tag_stack->next = fos->tag_stack;
    tag_stack->tag = tag;
    fos->tag_stack = tag_stack;
#endif
    va_list va;
    va_start(va, tag);
    (*fos->target->enter)(fos, tag, va);
    va_end(va);
}

void
cufo_leave(cufo_stream_t fos, cufo_tag_t tag)
{
#ifdef CUCONF_DEBUG_CLIENT
    cufoP_tag_stack_t tag_stack = fos->tag_stack;
    if (tag_stack->tag != tag)
	cu_bugf("Expecting closing tag %s, got %s.",
		cufo_tag_name(tag_stack->tag), cufo_tag_name(tag));
    fos->tag_stack = tag_stack->next;
#endif
    (*fos->target->leave)(fos, tag);
}

void
cufo_leaveln(cufo_stream_t fos, cufo_tag_t tag)
{
#ifdef CUCONF_DEBUG_CLIENT
    cufo_leave(fos, tag);
#else
    (*fos->target->leave)(fos, tag);
#endif
    cufo_print_charr(fos, "\n", 1);
}

void
cufo_empty(cufo_stream_t fos, cufo_tag_t tag, ...)
{
    va_list va;
    va_start(va, tag);
    (*fos->target->enter)(fos, tag, va);
    (*fos->target->leave)(fos, tag);
    va_end(va);
}
