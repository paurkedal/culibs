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

#ifndef CUFO_STREAM_H
#define CUFO_STREAM_H

#include <cufo/fwd.h>
#include <cu/va_ref.h>
#include <cu/inherit.h>
#include <cu/buffer.h>
#include <cu/wchar.h>
#include <cuoo/fwd.h>
#include <cucon/hzmap.h>
#include <iconv.h>

CU_BEGIN_DECLARATIONS

void cufoP_set_wide(cufo_stream_t fos, cu_bool_t be_wide);
void cufoP_flush(cufo_stream_t fos, cu_bool_t must_clear);
void *cufoP_stream_produce(cufo_stream_t fos, size_t len);

/*!\defgroup cufo_stream_h cufo/stream.h: Formatted Ouput Streams
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

/*!The type of a \ref CUOO_INTF_FOPRINT_FN callback used to print dynamic
 * objects to a formatted stream. */
typedef void (*cufo_print_ptr_fn_t)(cufo_stream_t fos, cufo_prispec_t spec,
				    void *ptr);

struct cufo_target_s
{
    size_t (*write)(cufo_stream_t fos, void const *data, size_t size);
    cu_bool_t (*enter)(cufo_stream_t fos, cufo_tag_t tag, cufo_attrbind_t);
    void (*leave)(cufo_stream_t fos, cufo_tag_t);
    void *(*close)(cufo_stream_t fos);
    void (*flush)(cufo_stream_t fos);
};

#define CUFO_SFLAG_SHOW_TYPE_IF_UNPRINTABLE	(1u << 0)
#define CUFO_SFLAG_HAVE_ERROR			(1u << 16)

struct cufo_convinfo_s
{
    iconv_t cd;
    unsigned int wr_scale;
};

struct cufo_stream_s
{
    cu_inherit (cu_buffer_s);
    cufo_target_t target;
    cu_bool_least_t is_wide;
    char lastchar;
    unsigned int flags;
    struct cufo_convinfo_s convinfo[2]; /* = { multibyte_iconv, wide_iconv } */
#ifdef CUCONF_DEBUG_CLIENT
    struct cufoP_tag_stack_s *tag_stack;
#endif
    struct cucon_hzmap_s clientstate_map;
};

cu_bool_t cufo_stream_init(cufo_stream_t fos, char const *encoding,
			   cufo_target_t target);

cu_bool_t cufo_stream_clientstate(cufo_stream_t fos, void const *key,
				  size_t state_size, cu_ptr_ptr_t state_out);

cu_bool_t cufo_stream_drop_clientstate(cufo_stream_t fos, void const *key);

/*!Returns the last character written to \a fos if it was an ASCII character,
 * otherwise returns 0.  This is mainly inteded as a convenient way for a
 * client to check if it has written out spaces or newlines, instead of keeping
 * track by itself. */
char cufo_stream_lastchar(cufo_stream_t fos);

CU_SINLINE void *
cufo_stream_produce(cufo_stream_t fos, size_t len)
{
    if (len > cu_buffer_freecap(cu_to(cu_buffer, fos)))
	return cufoP_stream_produce(fos, len);
    else {
	void *res = cu_buffer_content_end(cu_to(cu_buffer, fos));
	cu_buffer_set_content_end(cu_to(cu_buffer, fos), cu_ptr_add(res, len));
	return res;
    }
}

cufo_stream_t cufo_open_strip_sink(char const *encoding, cu_dsink_t sink);

cufo_stream_t cufo_open_strip_fd(char const *encoding, int fd);

cufo_stream_t cufo_open_strip_file(char const *encoding, char const *path);

cufo_stream_t cufo_open_strip_str(void);

cufo_stream_t cufo_open_strip_wstring(void);

cufo_stream_t cufo_open_text_sink(char const *encoding, cufo_textstyle_t style,
				  cu_dsink_t sink);

cufo_stream_t cufo_open_text_fd(char const *encoding, cufo_textstyle_t style,
				int fd);

cufo_stream_t cufo_open_text_file(char const *encoding, cufo_textstyle_t style,
				  char const *path);

cufo_stream_t cufo_open_text_str(cufo_textstyle_t style);

cufo_stream_t cufo_open_text_wstring(cufo_textstyle_t style);

cufo_stream_t cufo_open_xmldirect(char const *encoding, cu_dsink_t target_sink);

void *cufo_close(cufo_stream_t fos);

void cufo_close_discard(cufo_stream_t fos);

CU_SINLINE void cufo_flush(cufo_stream_t fos) { cufoP_flush(fos, cu_false); }

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
    s = (char *)cufo_stream_produce(fos, 1);
    *s = ch;
}

CU_SINLINE void
cufo_fast_putwc(cufo_stream_t fos, cu_wchar_t wc)
{
    cu_wchar_t *s;
    cu_debug_assert(fos->is_wide);
    s = (cu_wchar_t *)cufo_stream_produce(fos, sizeof(cu_wchar_t));
    *s = wc;
}

void cufo_putc(cufo_stream_t fos, char ch);

void cufo_putwc(cufo_stream_t fos, cu_wchar_t wc);

void cufo_fillc(cufo_stream_t fos, char ch, int repeat);

void cufo_fillwc(cufo_stream_t fos, cu_wchar_t wc, int repeat);

void cufo_puts(cufo_stream_t fos, char const *cs);

void cufo_putws(cufo_stream_t fos, cu_wchar_t *wcs);

void cufo_newline(cufo_stream_t fos);

void cufo_space(cufo_stream_t fos);

void cufo_print_charr(cufo_stream_t fos, char const *cs, size_t cs_len);

void cufo_print_wcarr(cufo_stream_t fos, cu_wchar_t const *arr, size_t len);

void cufo_print_wstring(cufo_stream_t fos, cu_wstring_t ws);

void cufo_print_str(cufo_stream_t fos, cu_str_t str);

void cufo_print_sref(cufo_stream_t fos, cu_sref_t srf);

void cufo_print_ex(cufo_stream_t fos, cuex_t e);

void cufo_printsp_ex(cufo_stream_t fos, cufo_prispec_t spec, cuex_t e);

cu_bool_t cufoP_entera(cufo_stream_t fos, cufo_tag_t tag, ...);

cu_bool_t cufo_entera_va(cufo_stream_t fos, cufo_tag_t tag, va_list va);

#define cufo_enter(fos, tag) cufoP_entera(fos, tag, NULL)
#define cufo_entera(fos, tag, ...) cufoP_entera(fos, tag, __VA_ARGS__, NULL)

void cufo_leave(cufo_stream_t fos, cufo_tag_t tag);

void cufo_leaveln(cufo_stream_t fos, cufo_tag_t tag);

void cufo_empty(cufo_stream_t fos, cufo_tag_t tag, ...);

int cufo_printf(cufo_stream_t fos, char const *fmt, ...);

int cufo_vprintf(cufo_stream_t fos, char const *fmt, va_list va);

int cufo_printfln(cufo_stream_t fos, char const *fmt, ...);

void cufo_logf(cufo_stream_t fos, cu_log_facility_t facility,
	       char const *fmt, ...);

void cufo_vlogf(cufo_stream_t fos, cu_log_facility_t facility,
		char const *fmt, va_list va);

void cufo_vlogf_at(cufo_stream_t fos, cu_log_facility_t facility,
		   cu_sref_t sref, char const *fmt, va_list va);

void cufo_register_ptr_format(char const *key, cufo_print_ptr_fn_t fn);

void cufo_register_va_format(char const *key, cufo_print_fn_t fn);

/*!@}*/
CU_END_DECLARATIONS

#endif
