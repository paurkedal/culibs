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

#ifndef CUTEXT_SINK_H
#define CUTEXT_SINK_H

#include <cutext/fwd.h>
#include <cu/buffer.h>
#include <cu/box.h>
#include <stdarg.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cutext_sink_h cutext/sink.h: Text Sink
 ** @{ \ingroup cutext_mod
 **
 ** This header provides a data structure with callbacks for defining <i>data
 ** sinks</i> which are consumers of arbitrary byte streams.
 **
 ** A data sink is said to be <i>clog-free</i> if it's write-callback is
 ** guaranteed to always cosume all data provided to it.  Data sinks derived
 ** form \ref cutext_sink may or may not be clog-free.  This property may be
 ** essential to some callers, and since this is not checked at compile time,
 ** it's recommendable that
 ** <ul>
 **   <li>It's documentet whether a returned sink is guaranteed to be clog-free.
 **   <li>It's documentet whether a function sink-parameter is required to be
 **     clog-free.
 **   <li>When a clog-free stream is expected, this is verified at run-time by a
 **     call to \ref cutext_sink_assert_clogfree.
 ** </ul>
 ** Any buffer can be turned into a clog-free buffer by stacking a buffer on
 ** top of it, as done by \ref cutext_sink_stack_buffer.
 */

/** \name Sink API
 ** @{ */

typedef enum {
    CUTEXT_SINK_INFO_ENCODING = 1,
    CUTEXT_SINK_INFO_NCOLUMNS = 3,
    CUTEXT_SINK_INFO_DEBUG_STATE = 0x70
} cutext_sink_info_key_t;

typedef char const *cutext_sink_info_encoding_t;
typedef cu_str_t cutext_sink_info_debug_state_t;

CU_SINLINE cu_bool_t
cutext_sink_info_key_inherits(cutext_sink_info_key_t key)
{ return key & 1; }

struct cufo_tag;
struct cufo_attrbind;

typedef struct cutext_sink_descriptor_s *cutext_sink_descriptor_t;
struct cutext_sink_descriptor_s
{
    unsigned int flags;

    size_t (*write)(cutext_sink_t, void const *, size_t);
    cu_bool_t (*flush)(cutext_sink_t);
    cu_box_t (*finish)(cutext_sink_t);
    void (*discard)(cutext_sink_t);
    cu_bool_t (*iterA_subsinks)(cutext_sink_t, cu_clop(f, cu_bool_t, cutext_sink_t));
    cu_box_t (*info)(cutext_sink_t, cutext_sink_info_key_t);
    cu_bool_t (*enter)(cutext_sink_t, struct cufo_tag *,
		       struct cufo_attrbind *);
    void (*leave)(cutext_sink_t, struct cufo_tag *);
};

#define CUTEXT_SINK_FLAG_CLOGFREE 1

#define CUTEXT_SINK_DESCRIPTOR_DEFAULTS \
    .flags = 0, \
    .write = cutext_sink_noop_write, \
    .flush = cutext_sink_noop_flush, \
    .finish = cutext_sink_noop_finish, \
    .discard = cutext_sink_noop_discard, \
    .iterA_subsinks = cutext_sink_empty_iterA_subsinks, \
    .info = cutext_sink_default_info, \
    .enter = cutext_sink_noop_enter, \
    .leave = cutext_sink_noop_leave

/** Base struct for data sinks. */
struct cutext_sink
{
    cutext_sink_descriptor_t descriptor;
};

CU_SINLINE cutext_sink_descriptor_t
cutext_sink_descriptor(cutext_sink_t sink)
{ return sink->descriptor; }

/** Initialise \a sink with callbacks from \a descriptor. */
CU_SINLINE void
cutext_sink_init(cutext_sink_t sink, cutext_sink_descriptor_t descriptor)
{
    sink->descriptor = descriptor;
}

/** Special return value for \a cutext_sink_write to indicate failure. */
#define CU_DSINK_WRITE_ERROR ((size_t)-1)

/** Tries to write up to \a max_size bytes starting at \a buf to the sink, and
 ** returns the number of bytes actually consumed.  If the sink is clog-free,
 ** \a max_size is returned. A special return value \ref CU_DSINK_WRITE_ERROR
 ** indicates failure. */
CU_SINLINE size_t
cutext_sink_write(cutext_sink_t sink, void const *buf, size_t max_size)
{
    return (*sink->descriptor->write)(sink, buf, max_size);
}

CU_SINLINE unsigned int
cutext_sink_flags(cutext_sink_t sink)
{ return sink->descriptor->flags; }

/** True if \a sink is clog-free, as reported by the \ref
 ** CU_DSINK_FN_IS_CLOGFREE response. */
CU_SINLINE cu_bool_t cutext_sink_is_clogfree(cutext_sink_t sink)
{ return sink->descriptor->flags & CUTEXT_SINK_FLAG_CLOGFREE; }

/** Control function to check that \a sink is clog-free.  It is recommended to
 ** call this once before starting to write to a sink where a clog-free sink is
 ** required for correct operation. */
void cutext_sink_assert_clogfree(cutext_sink_t sink);

CU_SINLINE cu_box_t
cutext_sink_info(cutext_sink_t sink, cutext_sink_info_key_t key)
{ return (*sink->descriptor->info)(sink, key); }

/** Returns a suitable value for the default case of the info implementation of
 ** \a sink if it passes its input to \a subsink. */
cu_box_t cutext_sink_info_inherit(cutext_sink_t sink,
				  cutext_sink_info_key_t key,
				  cutext_sink_t subsink);

CU_SINLINE char const *
cutext_sink_encoding(cutext_sink_t sink)
{ return cu_unbox_ptr(cutext_sink_info_encoding_t,
		      cutext_sink_info(sink, CUTEXT_SINK_INFO_ENCODING)); }

/** Attempt to write any buffered data and pass down the flush request to
 ** sub-sinks. */
CU_SINLINE cu_bool_t
cutext_sink_flush(cutext_sink_t sink)
{
    return (*sink->descriptor->flush)(sink);
}

/** Inform \a sink that no more data will be written.  This implies flushing
 ** any buffered data and closing resources.  Some sinks may return constructed
 ** data. */
CU_SINLINE cu_box_t
cutext_sink_finish(cutext_sink_t sink)
{
    return (*sink->descriptor->finish)(sink);
}

/** Inform \a sink that no more data will be written, and that the operation is
 ** considered failed.  This means any buffered data may be discarded and
 ** resources closed.  Operations on external resources such as file creation
 ** or database updates, may or may not be reverted. */
CU_SINLINE void
cutext_sink_discard(cutext_sink_t sink)
{
    (*sink->descriptor->discard)(sink);
}

/** Iterates over the immediate subsinks in case \a sink is a filter. */
CU_SINLINE cu_bool_t
cutext_sink_iterA_subsinks(cutext_sink_t sink, cu_clop(f, cu_bool_t, cutext_sink_t))
{
    return (*sink->descriptor->iterA_subsinks)(sink, f);
}

void cutext_sink_debug_dump(cutext_sink_t sink);

/** @}
 ** \name Generic callbacks
 ** @{ */
size_t cutext_sink_noop_write(cutext_sink_t, void const *, size_t);
cu_bool_t cutext_sink_noop_flush(cutext_sink_t);
cu_box_t cutext_sink_noop_finish(cutext_sink_t);
void cutext_sink_noop_discard(cutext_sink_t);
size_t cutext_sink_subsinks_write(cutext_sink_t, void const *, size_t);
cu_bool_t cutext_sink_subsinks_flush(cutext_sink_t);
cu_bool_t cutext_sink_empty_iterA_subsinks(cutext_sink_t,
					cu_clop(f, cu_bool_t, cutext_sink_t));
cu_box_t cutext_sink_default_info(cutext_sink_t, cutext_sink_info_key_t);
cu_bool_t cutext_sink_noop_enter(cutext_sink_t, struct cufo_tag *,
				 struct cufo_attrbind *);
void cutext_sink_noop_leave(cutext_sink_t, struct cufo_tag *);

/** @} */
/** \name Sink Implementations
 ** @{ */

/** Returns a sink on which the call to \ref cutext_sink_finish returns a \ref
 ** cu_str_h "cu_str_t" of the written data.  No conversion is done.  Writing
 ** UTF-8 encoded data gives a text string. */
cutext_sink_t cutext_sink_new_str(void);

/** Returns a sink on which the call to \ref cutext_sink_finish returns a \ref
 ** cu_wstring_h "cu_wstring_t" of the written data.  No conversion is done.
 ** Writing \ref cu_wchar_encoding encoded characters gives a text string. */
cutext_sink_t cutext_sink_new_wstring(void);

cutext_sink_t cutext_sink_fdopen(char const *encoding, int fd,
				 cu_bool_t close_fd);

cutext_sink_t cutext_sink_fopen(char const *encoding, char const *path);

/** Creates a clog-free sink of \a subsink by buffering data. */
cutext_sink_t cutext_sink_stack_buffer(cutext_sink_t subsink);

/** Modifies the character encoding of \a subsink to \a new_encoding by passing
 ** the data through \c iconv. */
cutext_sink_t cutext_sink_stack_iconv(char const *new_encoding,
				      cutext_sink_t subsink);

/** A sink which counts the number of bytes written. */
struct cutext_countsink
{
    cu_inherit (cutext_sink);
    size_t count;
};

CU_SINLINE cutext_sink_t
cutext_countsink_to_sink(cutext_countsink_t sink)
{ return cu_to(cutext_sink, sink); }

/** Initialise \a sink. */
void cutext_countsink_init(cutext_countsink_t sink);

/** Returns the number of bytes written to \a sink. */
CU_SINLINE size_t
cutext_countsink_count(cutext_countsink_t sink)
{ return sink->count; }

/** A sink which copy the data written into a buffer. */
struct cutext_buffersink
{
    cu_inherit (cutext_sink);
    struct cu_buffer buffer;
};

CU_SINLINE cutext_sink_t
cutext_buffersink_to_sink(cutext_buffersink_t sink)
{ return cu_to(cutext_sink, sink); }

/** Initialise the buffer sink \a sink. */
void cutext_buffersink_init(cutext_buffersink_t sink);

/** The underlying buffer of \a sink. */
CU_SINLINE cu_buffer_t
cutext_buffersink_buffer(cutext_buffersink_t sink)
{ return &sink->buffer; }

/** @}
 ** @} */
CU_END_DECLARATIONS

#endif
