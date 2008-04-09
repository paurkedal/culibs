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

#ifndef CUFO_FORMATTER_H
#define CUFO_FORMATTER_H

#include <cufo/fwd.h>
#include <cu/va_ref.h>
#include <cu/inherit.h>
#include <cu/buffer.h>
#include <cu/wchar.h>
#include <cuoo/fwd.h>

CU_BEGIN_DECLARATIONS

void cufoP_set_wide(cufo_stream_t fos, cu_bool_t be_wide);

/*!\defgroup cufo_stream_h cufo/stream.h:
 *@{\ingroup cufo_mod */

#define CUFO_PRIFLAG_PLUS	 1	/* explicit plus-sign for positive */
#define CUFO_PRIFLAG_MINUS	 2	/* left justify */
#define CUFO_PRIFLAG_SPACE	 4	/* pad sign position for positive */
#define CUFO_PRIFLAG_HASH	 8	/* alternate form */
#define CUFO_PRIFLAG_ZERO	16	/* zero padded */

struct cufo_prispec_s
{
    unsigned int flags;
    int width;		/* -1 if unspecified */
    int precision;	/* -1 if unspecified */
};
typedef void (*cufo_print_fn_t)(cufo_stream_t fos, cufo_prispec_t spec,
				cu_va_ref_t va_ref);

typedef void (*cufo_print_ptr_fn_t)(cufo_stream_t fos, cufo_prispec_t spec,
				    void *ptr);

struct cufo_target_s
{
    void (*flush)(cufo_stream_t fos, cu_bool_t must_empty);
    void (*enter)(cufo_stream_t fos, cufo_tag_t tag, va_list va);
    void (*leave)(cufo_stream_t fos, cufo_tag_t);
    void *(*close)(cufo_stream_t fos);
};

#define CUFO_SFLAG_SHOW_TYPE_IF_UNPRINTABLE	(1u << 0)
#define CUFO_SFLAG_HAVE_ERROR			(1u << 16)

struct cufo_stream_s
{
    cu_inherit (cu_buffer_s);
    cufo_target_t target;
    cu_bool_t is_wide;
    unsigned int flags;
#ifdef CUCONF_DEBUG_CLIENT
    struct cufoP_tag_stack_s *tag_stack;
#endif
};

void cufo_stream_init(cufo_stream_t fos, cufo_target_t target);

cufo_stream_t cufo_open_fd(char const *encoding, int fd, cu_bool_t do_close);

cufo_stream_t cufo_open_file(char const *encoding, char const *path);

cufo_stream_t cufo_open_sink(char const *encoding, cu_data_sink_t sink);

cufo_stream_t cufo_open_str(void);

cufo_stream_t cufo_open_str_recode(char const *encoding);

cufo_stream_t cufo_open_wstring(void);

cufo_stream_t cufo_open_wstring_recode(char const *encoding);

void *cufo_close(cufo_stream_t fos);

void cufo_close_discard(cufo_stream_t fos);

void cufo_flush(cufo_stream_t fos);

CU_SINLINE void
cufo_set_wide(cufo_stream_t fos, cu_bool_t be_wide)
{
    if (fos->is_wide != be_wide)
	cufoP_set_wide(fos, be_wide);
}

CU_SINLINE void
cufo_flag_error(cufo_stream_t fos)
{ fos->flags |= CUFO_SFLAG_HAVE_ERROR; }

CU_SINLINE cu_bool_t
cufo_have_error(cufo_stream_t fos)
{ return fos->flags & CUFO_SFLAG_HAVE_ERROR; }

CU_SINLINE void
cufo_fast_putc(cufo_stream_t fos, char ch)
{
    char *s;
    cu_debug_assert(!fos->is_wide);
    s = cu_buffer_produce(cu_to(cu_buffer, fos), 1);
    *s = ch;
}

CU_SINLINE void
cufo_fast_putwc(cufo_stream_t fos, cu_wchar_t wc)
{
    cu_wchar_t *s;
    cu_debug_assert(fos->is_wide);
    s = cu_buffer_produce(cu_to(cu_buffer, fos), sizeof(cu_wchar_t));
    *s = wc;
}

void cufo_putc(cufo_stream_t fos, char ch);

void cufo_putwc(cufo_stream_t fos, cu_wchar_t wc);

void cufo_fillc(cufo_stream_t fos, char ch, int repeat);

void cufo_fillwc(cufo_stream_t fos, cu_wchar_t wc, int repeat);

void cufo_puts(cufo_stream_t fos, char const *cs);

void cufo_putws(cufo_stream_t fos, cu_wchar_t *wcs);

void cufo_print_charr(cufo_stream_t fos, char const *cs, size_t cs_len);

void cufo_print_wcarr(cufo_stream_t fos, cu_wchar_t const *arr, size_t len);

void cufo_print_wstring(cufo_stream_t fos, cu_wstring_t ws);

void cufo_print_str(cufo_stream_t fos, cu_str_t str);

void cufo_print_ex(cufo_stream_t fos, cuex_t e);

void cufo_printsp_ex(cufo_stream_t fos, cufo_prispec_t spec, cuex_t e);

void cufo_enter(cufo_stream_t fos, cufo_tag_t tag, ...);

void cufo_enter_va(cufo_stream_t fos, cufo_tag_t tag, va_list va);

void cufo_leave(cufo_stream_t fos, cufo_tag_t tag);

void cufo_leaveln(cufo_stream_t fos, cufo_tag_t tag);

void cufo_empty(cufo_stream_t fos, cufo_tag_t tag, ...);

int cufo_printf(cufo_stream_t fos, char const *fmt, ...);

int cufo_vprintf(cufo_stream_t fos, char const *fmt, va_list va);

/*!@}*/
CU_END_DECLARATIONS

#endif
