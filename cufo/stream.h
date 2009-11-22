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
#include <cutext/fwd.h>
#include <cu/va_ref.h>
#include <cu/inherit.h>
#include <cu/buffer.h>
#include <cu/wchar.h>
#include <cu/thread.h>
#include <cuoo/fwd.h>
#include <cucon/hzmap.h>
#include <iconv.h>

CU_BEGIN_DECLARATIONS

#define CUFOP_FLUSH_MUST_CLEAR 1
#define CUFOP_FLUSH_PROPAGATE 2

void cufoP_set_wide(cufo_stream_t fos, cu_bool_t be_wide);
void cufoP_flush(cufo_stream_t fos, unsigned int flags);
void *cufoP_stream_produce(cufo_stream_t fos, size_t len);

/** \defgroup cufo_stream_h cufo/stream.h: Formatted Ouput Streams
 ** @{ \ingroup cufo_mod */

/** \name Stream API Details
 ** @{ */

#define CUFO_PRIFLAG_PLUS	 1	/* explicit plus-sign for positive */
#define CUFO_PRIFLAG_MINUS	 2	/* left justify */
#define CUFO_PRIFLAG_SPACE	 4	/* pad sign position for positive */
#define CUFO_PRIFLAG_HASH	 8	/* alternate form */
#define CUFO_PRIFLAG_ZERO	16	/* zero padded */

struct cufo_prispec
{
    unsigned int flags;
    int width;		/* -1 if unspecified */
    int precision;	/* -1 if unspecified */
};
typedef void (*cufo_print_fn_t)(cufo_stream_t fos, cufo_prispec_t spec,
				cu_va_ref_t va_ref);

/** The type of a \ref CUOO_INTF_FOPRINT_FN callback used to print dynamic
 ** objects to a formatted stream. */
typedef void (*cufo_print_ptr_fn_t)(cufo_stream_t fos, cufo_prispec_t spec,
				    void *ptr);

#define CUFO_SFLAG_SHOW_TYPE_IF_UNPRINTABLE	(1u << 0)
#define CUFO_SFLAG_HAVE_ERROR			(1u << 16)

struct cufo_convinfo
{
    iconv_t cd;
    unsigned int wr_scale;
};

struct cufo_stream
{
    cu_inherit (cu_buffer);
    cutext_sink_t target;
    cu_bool_least_t is_wide;
    char lastchar;
    unsigned int flags;
    struct cufo_convinfo convinfo[2]; /* = { multibyte_iconv, wide_iconv } */
#ifdef CUCONF_DEBUG_CLIENT
    struct cufoP_tag_stack *tag_stack;
#endif
    struct cucon_hzmap clientstate_map;
    cu_mutex_t mutex;
};

cu_bool_t cufo_stream_init(cufo_stream_t fos, char const *encoding,
			   cutext_sink_t target);

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

/** @}
 ** \name Stream Construction and Cleanup
 ** @{ */

/** A stream tied to file descriptor 1 (stdandard output). */
extern cufo_stream_t cufo_stdout;

/** A stream tied to file descriptor 2 (standard error). */
extern cufo_stream_t cufo_stderr;

cufo_stream_t cufo_open_sink(cutext_sink_t sink);

cufo_stream_t cufo_open_strip_sink(cutext_sink_t sink);

cufo_stream_t cufo_open_strip_fd(char const *encoding,
				 int fd, cu_bool_t close_fd);

cufo_stream_t cufo_open_strip_file(char const *encoding, char const *path);

cufo_stream_t cufo_open_strip_str(void);

cufo_stream_t cufo_open_strip_wstring(void);

/** Returns a text stream writing to \a sink. */
cufo_stream_t cufo_open_text_sink(cufo_textstyle_t style, cutext_sink_t sink);

/** Returns a text stream which convert (if necessary) to the character
 ** encoding \a encoding, and writes the result to \a fd.  If \a close_fd is
 ** true, then \a fd will be closed when the returned stream is closed. */
cufo_stream_t cufo_open_text_fd(char const *encoding, cufo_textstyle_t style,
				int fd, cu_bool_t close_fd);

cufo_stream_t cufo_open_text_file(char const *encoding, cufo_textstyle_t style,
				  char const *path);

cufo_stream_t cufo_open_text_str(cufo_textstyle_t style);

cufo_stream_t cufo_open_text_wstring(cufo_textstyle_t style);

cufo_stream_t cufo_open_xml(cutext_sink_t target_sink);

/** Opens a stream with terminal capabilities using default text and terminal
 ** styles.  The \a encoding must be multibyte or byte compatible due to the
 ** terminfo escape sequences.  If the culibs was configured without terminal
 ** support, this falls back to \ref cufo_open_text_fd. */
cufo_stream_t cufo_open_term_fd(char const *encoding, char const *term,
				int fd, cu_bool_t close_fd);

/** Makes a conservative guess whether \a fd is a terminal and calls either
 ** \ref cufo_open_text_fd or \ref cufo_open_term_fd as appropriate.  It also
 ** determines the character encoding from the current locale. */
cufo_stream_t cufo_open_auto_fd(int fd, cu_bool_t close_fd);

/** Flushes \a fos as frees up associated resources.  If it's connected to a
 ** sink which produces data (such as a string), then the data will be
 ** returned.  It's up to the callee to know the correct type to unbox.  */
cu_box_t cufo_close(cufo_stream_t fos);

/** Frees up associated resources of \a fos without flushing or returning
 ** anything.  \see cufo_close */
void cufo_close_discard(cufo_stream_t fos);

/** @}
 ** \name Stream Control
 ** @{ */

/** Acquires an exclusive lock on \a fos in case it's shared by multiple
 ** threads.  Pair it off with \ref cufo_unlock. */
CU_SINLINE void cufo_lock(cufo_stream_t fos)
{ cu_mutex_lock(&fos->mutex); }

/** Unlocks \a fos. */
CU_SINLINE void cufo_unlock(cufo_stream_t fos)
{ cu_mutex_unlock(&fos->mutex); }

/** Flushes buffered data down the stack of sinks. */
CU_SINLINE void cufo_flush(cufo_stream_t fos)
{ cufoP_flush(fos, CUFOP_FLUSH_PROPAGATE); }

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

/** @}
 ** \name Basic Printing
 ** @{ */

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

/** Writes the ASCII character \a ch to \a fos.  Use \ref cufo_puts or \ref
 ** cufo_print_charr to write UTF-8 byte sequences. */
void cufo_putc(cufo_stream_t fos, char ch);

/** Writes the wide character \a wc to \a fos. */
void cufo_putwc(cufo_stream_t fos, cu_wchar_t wc);

/** Writes \a n repetitions of the ASCII character \a ch to \a fos. */
void cufo_fillc(cufo_stream_t fos, char ch, int n);

/** Writes \a n repetitions of the wide character \a wc to \a fos. */
void cufo_fillwc(cufo_stream_t fos, cu_wchar_t wc, int n);

/** Writes the null-terminated string \a cs to \a fos. */
void cufo_puts(cufo_stream_t fos, char const *cs);

/** Writes the null-terminated wide string \a wcs to \a fos. */
void cufo_putws(cufo_stream_t fos, cu_wchar_t *wcs);

/** Writes a newline to \a fos unless the last character was a newline. */
void cufo_newline(cufo_stream_t fos);

/** Writes a space to \a fos unless the last character was an ASCII white
 ** space.  Non-ASCII white space may also be considered in the future. */
void cufo_space(cufo_stream_t fos);

/** Write \a cs_len bytes from \a cs to \a fos, interpreted as UTF-8. */
void cufo_print_charr(cufo_stream_t fos, char const *cs, size_t cs_len);

/** Write \a len wide characters from \a arr to \a fos. */
void cufo_print_wcarr(cufo_stream_t fos, cu_wchar_t const *arr, size_t len);

/** Write the wide string \a ws to \a fos. */
void cufo_print_wstring(cufo_stream_t fos, cu_wstring_t ws);

/** Write the UTF-8 string \a str to \a fos. */
void cufo_print_str(cufo_stream_t fos, cu_str_t str);

/** Format a source reference to \a fos. */
void cufo_print_sref(cufo_stream_t fos, cu_sref_t srf);

/** Print an expression to \a fos. */
void cufo_print_ex(cufo_stream_t fos, cuex_t e);

void cufo_printsp_ex(cufo_stream_t fos, cufo_prispec_t spec, cuex_t e);

/** @}
 ** \name Basic Markup
 ** @{ */

#ifndef CU_IN_DOXYGEN
cu_bool_t cufoP_entera(cufo_stream_t fos, cufo_tag_t tag, ...);
#endif

cu_bool_t cufo_entera_va(cufo_stream_t fos, cufo_tag_t tag, va_list va);

/** Starts a fragment marked up with \a tag. */
CU_SINLINE cu_bool_t
cufo_enter(cufo_stream_t fos, cufo_tag_t tag)
{ return cufoP_entera(fos, tag, NULL); }

/** Starts a fragment marked up with \a tag with associated attributes
 ** specified in the variadic part.  Attributes can be passed by using the \c
 ** cufoA_* macros found in \ref cufo_attrdefs_h "cufo/attrdefs.h". */
#define cufo_entera(fos, tag, ...) cufoP_entera(fos, tag, __VA_ARGS__, NULL)

/** Ends a fragment which was started by \ref cufo_enter or \ref cufo_entera.
 ** The \a tag arguments of the matching calls must be the same. */
void cufo_leave(cufo_stream_t fos, cufo_tag_t tag);

void cufo_leaveln(cufo_stream_t fos, cufo_tag_t tag);

void cufo_empty(cufo_stream_t fos, cufo_tag_t tag, ...);

/** A convenience function which wraps \ref cufo_enter and \ref cufo_leave
 ** around \ref cufo_putc. */
void cufo_tagputc(cufo_stream_t fos, cufo_tag_t tag, char ch);

/** A convenience function which wraps \ref cufo_enter and \ref cufo_leave
 ** around \ref cufo_puts. */
void cufo_tagputs(cufo_stream_t fos, cufo_tag_t tag, char const *s);

/** @}
 ** \name Formatted Printing and Markup
 ** @{ */

/** Print \a fmt to \a fos, replacing format specifies with representations of
 ** the corresponding arguments from \a va. */
int cufo_vprintf(cufo_stream_t fos, char const *fmt, va_list va);

/** Print \a fmt to \a fos, replacing format specifies with representations of
 ** the corresponding following arguments. */
int cufo_printf(cufo_stream_t fos, char const *fmt, ...);

/** A variant of \ref cufo_printf which locks the stream while printing. */
int cufo_lprintf(cufo_stream_t fos, char const *fmt, ...);

/** Calls \ref cufo_printf then \ref cufo_flush on \ref cufo_stdout after
 ** acquiring a temporary lock. */
int cufo_oprintf(char const *fmt, ...);

/** Calls \ref cufo_printf then \ref cufo_flush on \ref cufo_stderr after
 ** acquiring a temporary lock. */
int cufo_eprintf(char const *fmt, ...);

int cufo_printfln(cufo_stream_t fos, char const *fmt, ...);

/** This is a low-level function which invokes the \ref cufo_vlogf_at handler
 ** directly, bypassing the handler in \a facility.  Use \ref cu_logging_h
 ** "cu/logging.h" or \ref cu_diag_h "cu/diag.h" instead. */
void cufo_logf(cufo_stream_t fos, cu_log_facility_t facility,
	       char const *fmt, ...);

/** \copydoc cufo_logf */
void cufo_vlogf(cufo_stream_t fos, cu_log_facility_t facility,
		char const *fmt, va_list va);

/** This is the \ref cu_logging_h "cu/logging.h" callback installed by \ref
 ** cufo_init.  Its usually not needed except when defining new \ref
 ** cu_register_log_binder "log binder" logic. */
void cufo_vlogf_at(cufo_stream_t fos, cu_log_facility_t facility,
		   cu_sref_t sref, char const *fmt, va_list va);

/** @}
 ** \name Program Initialisation
 ** @{ */

void cufo_register_ptr_format(char const *key, cufo_print_ptr_fn_t fn);

void cufo_register_va_format(char const *key, cufo_print_fn_t fn);

/** @}
 ** @} */
CU_END_DECLARATIONS

#endif
