/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#ifndef CUTEXT_SOURCE_H
#define CUTEXT_SOURCE_H

#include <cutext/fwd.h>
#include <cu/box.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cutext_source_h cutext/source.h: Text Source
 ** @{ \ingroup cutext_mod */

/** \name Source API
 ** @{ */

typedef enum {
    CUTEXT_SOURCE_INFO_ENCODING = 1,
    CUTEXT_SOURCE_INFO_TABSTOP = 3,
} cutext_source_info_key_t;

CU_SINLINE cu_bool_t
cutext_source_info_key_inherits(cutext_source_info_key_t key)
{ return key & 1; }

/** The type of the CUTEXT_SOURCE_INFO_ENCODING property.  This trivial
 ** definition is provided to help keep type casts correct for boxing and
 ** unboxing operations. */
typedef char const *cutext_source_info_encoding_t;

/** The callbacks of a \ref cutext_source implementation.  For compatibility
 ** with future versions, the structure should be zeroed before assignment of
 ** the call-backs.  Alternatively, it may be declared static with C99
 ** initializers, as in <code>{.read = _my_read, ...}</code>. */
struct cutext_source_descriptor
{
    size_t (*read)(cutext_source_t, void *, size_t);
    void const *(*look)(cutext_source_t, size_t, size_t *);
    void (*close)(cutext_source_t);
    cutext_source_t (*subsource)(cutext_source_t);
    cu_box_t (*info)(cutext_source_t, cutext_source_info_key_t);
};

/** The type of a text source.  The implementation will typically embed it
 ** within a structure containing additional fields. */
struct cutext_source
{
    cutext_source_descriptor_t descriptor;
};

CU_SINLINE cutext_source_descriptor_t
cutext_source_descriptor(cutext_source_t src)
{
    return src->descriptor;
}

/** Initialize the base \a src of a source implementation with callbacks
 ** provided by \a descriptor. */
CU_SINLINE void
cutext_source_init(cutext_source_t src,
		   cutext_source_descriptor_t descriptor)
{
    src->descriptor = descriptor;
}

/** Read at most \a max_size bytes into \a buf, returning the actual number
 ** read or <code>(size_t)-1</code> on error.  A return of 0 indicates the end
 ** of the stream, any other successful call reads at least one byte. */
CU_SINLINE size_t
cutext_source_read(cutext_source_t src, void *buf, size_t max_size)
{ return (*src->descriptor->read)(src, buf, max_size); }

/** Skips over at most \a max_size bytes, returning the actual number skipped,
 ** or <code>(size_t)-1</code> on error.  A return of 0 indicates the end of
 ** the stream.  If a previous call to \ref cutext_source_look has returned a
 ** size of \a max_size or larger since the last read or skip, then this call
 ** succeeds with the full count. */
CU_SINLINE size_t
cutext_source_skip(cutext_source_t src, size_t max_size)
{ return (*src->descriptor->read)(src, NULL, max_size); }

/** True iff \a src provides the \ref cutext_source_look method. */
CU_SINLINE cu_bool_t
cutext_source_can_look(cutext_source_t src)
{ return src->descriptor->look != NULL; }

/** Request a lookahead of \a size bytes of upcoming data from \a src.  If
 ** successful the data is returned and its actual size is assigned to
 ** <code>*\a size_out</code>.  The actual size may be larger than \a size, and
 ** only on the end-of-stream or in case of error may it be smaller.
 **
 ** This method is only provided by some source implementations, as reported by
 ** \ref cutext_source_can_look.  If needed, \ref cutext_source_stack_buffer
 ** stacks a buffer onto any source to provide lookahead. */
CU_SINLINE void const *
cutext_source_look(cutext_source_t src, size_t size, size_t *size_out)
{
    return (*src->descriptor->look)(src, size, size_out);
}

/** Close \a src, which in turn should close its subsources. */
CU_SINLINE void
cutext_source_close(cutext_source_t src)
{
    (*src->descriptor->close)(src);
}

CU_SINLINE cu_box_t
cutext_source_info(cutext_source_t src, cutext_source_info_key_t key)
{ return (*src->descriptor->info)(src, key); }

/** Assist the source implementation \a src with a subsource \a subsrc in
 ** providing a suitable default value for \a key. */
cu_box_t cutext_source_info_inherit(cutext_source_t src,
				    cutext_source_info_key_t key,
				    cutext_source_t subsrc);

/** The name of the character encoding of \a src, or \c NULL if unknown. */
CU_SINLINE char const *
cutext_source_encoding(cutext_source_t src)
{ return cu_unbox_ptr(cutext_source_info_encoding_t,
		      cutext_source_info(src, CUTEXT_SOURCE_INFO_ENCODING)); }


/** @}
 ** \name Generic Callbacks
 ** {@ */

/** Always returns 0. */
size_t cutext_source_null_read(cutext_source_t, void *, size_t);

/** The trivial close operation. */
void cutext_source_noop_close(cutext_source_t);

/** Returns \c NULL. */
cutext_source_t cutext_source_no_subsource(cutext_source_t);

/** Returns \a NULL for encoding, and 8 for tabstop. */
cu_box_t cutext_source_default_info(cutext_source_t, cutext_source_info_key_t);


/** @}
 ** \name Source Implementations
 ** @{ */

/** Return a source over a \a size bytes of data stored from \a data and
 ** considered to be encoded as \a enc. */
cutext_source_t cutext_source_new_mem(char const *enc,
				      void const *data, size_t size);

/** Return a source over the 0-terminated UTF-8 string \a cstr. */
cutext_source_t cutext_source_new_cstr(char const *cstr);

/** Return a source over the UTF-8 string \a str. */
cutext_source_t cutext_source_new_str(cu_str_t str);

/** Return a source over the wide string \a wstr. */
cutext_source_t cutext_source_new_wstring(cu_wstring_t wstr);

/** Return a source over the contents read from \a fd encoded as \a enc.  If \a
 ** close_fd is true, then close \a fd when \ref cutext_close is called on the
 ** returned source. */
cutext_source_t cutext_source_fdopen(char const *enc,
				     int fd, cu_bool_t close_fd);

/** Return a source over the contenst of the file at \a path encoded as \a
 ** encoding. */
cutext_source_t cutext_source_fopen(char const *encoding, char const *path);

/** Stack a buffer on top of \a subsrc to provide lookahead.  The \ref
 ** cutext_look method is guaranteed to be available on the returned source. */
cutext_source_t cutext_source_stack_buffer(cutext_source_t subsrc);

/** Stack an iconv conversion filter on \a subsrc, encoding to \a newenc.  */
cutext_source_t cutext_source_stack_iconv(char const *newenc,
					  cutext_source_t subsrc);

/** @}
 ** \name Algorithms
 ** @{ */

/** Drain \a src and return the number of bytes which were left. */
size_t cutext_source_count(cutext_source_t src);

/** @}
 ** @} */
CU_END_DECLARATIONS

#endif
