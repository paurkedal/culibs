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

#ifndef CU_DSINK_H
#define CU_DSINK_H

#include <cu/fwd.h>
#include <stdarg.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_dsink_h cu/dsink.h: Generic Data Sink
 *@{\ingroup cu_seq_mod
 *
 * This header provides a data structure with callbacks for defining <i>data
 * sinks</i> which are consumers of arbitrary byte streams.
 *
 * A data sink is said to be <i>clog-free</i> if it's write-callback is
 * guaranteed to always cosume all data provided to it.  Data sinks derived
 * form \ref cu_dsink_s may or may not be clog-free.  This property may be
 * essential to some callers, and since this is not checked at compile time,
 * it's recommendable that
 * <ul>
 *   <li>It's documentet whether a returned sink is guaranteed to be clog-free.
 *   <li>It's documentet whether a function sink-parameter is required to be
 *     clog-free.
 *   <li>When a clog-free stream is expected, this is verified at run-time by a
 *     call to \ref cu_dsink_assert_clogfree.
 * </ul>
 * Any buffer can be turned into a clog-free buffer by stacking a buffer on top
 * of it, as done by \ref cu_dsink_new_buffer.
 */

/* Use odd function codes where flushing any buffered data is required prior to
 * passing the control down to sub-sinks, even otherwise.  */

/*!\name Function Codes
 * @{*/
/*!Function code for \ref cu_dsink_is_clogfree, also used by \ref
 * cu_dsink_assert_clogfree.  If the sink is guaranteed to be clog-free, the
 * control dispatcher shall indicate this by returning \ref CU_DSINK_ST_SUCCESS
 * as response, otherwise this control need not be implemented. */
#define CU_DSINK_FN_IS_CLOGFREE		0

#define CU_DSINK_FN_FLUSH		1
    /*!< Function code for \ref cu_dsink_flush. */
#define CU_DSINK_FN_DISCARD		2
    /*!< Function code for \ref cu_dsink_discard. */
#define CU_DSINK_FN_FINISH		3
    /*!< Function code for \ref cu_dsink_finish. */
#define CU_DSINK_FN_DEBUG_DUMP		4
/*!@}*/

/*!\name Function Exit Codes
 * @{*/
#define CU_DSINK_ST_UNIMPL	((cu_word_t)-3)
    /*!< Status returned by control dispatcher when the requested function is
     * not implemented. */
#define CU_DSINK_ST_FAILURE	((cu_word_t)-1)
#define CU_DSINK_ST_SUCCESS	0
    /*!< Status which may be returned by a dispatches on successful return when
     * no other value is to be returned. */
/*!@}*/

/*!Base struct for data sinks. */
struct cu_dsink_s
{
    cu_word_t (*control)(cu_dsink_t source, int op, va_list);
    size_t (*write)(cu_dsink_t sink, void const *buf, size_t max_size);
};

/*!Initialise \a sink with callbacks \a control and \a data.  \a control is a C
 * function which integrates a set of control functions, and \a write is
 * callback which receives data written to \a sink. */
CU_SINLINE void
cu_dsink_init(cu_dsink_t sink,
	      cu_word_t (*control)(cu_dsink_t, int, va_list),
	      size_t (*write)(cu_dsink_t, void const *, size_t))
{
    sink->control = control;
    sink->write = write;
}

/*!Tries to write up to \a max_size bytes starting at \a buf to the sink, and
 * returns the number of bytes actually consumed.  If the sink is clog-free, \a
 * max_size is returned. */
CU_SINLINE size_t
cu_dsink_write(cu_dsink_t sink, void const *buf, size_t max_size)
{
    return (*sink->write)(sink, buf, max_size);
}

/*!Call the control function \a fn on \a sink with any additional arguments
 * wrapped in \a va. */
CU_SINLINE cu_word_t
cu_dsink_control_va(cu_dsink_t sink, int fn, va_list va)
{
    return (*sink->control)(sink, fn, va);
}

/*!Call the control function \a fn on \a sink possibly with additional
 * arguments.  See the specific control functions below for a safer
 * interface. */
cu_word_t cu_dsink_control(cu_dsink_t sink, int fn, ...);

/*!True if \a sink is clog-free, as reported by the \ref
 * CU_DSINK_FN_IS_CLOGFREE response. */
cu_bool_t cu_dsink_is_clogfree(cu_dsink_t sink);

/*!Control function to check that \a sink is clog-free.  It is recommended to
 * call this once before starting to write to a sink where a clog-free sink is
 * required for correct operation. */
void cu_dsink_assert_clogfree(cu_dsink_t sink);

/*!Attempt to write any buffered data and pass down the flush request to
 * sub-sinks. */
CU_SINLINE void
cu_dsink_flush(cu_dsink_t sink)
{ cu_dsink_control(sink, CU_DSINK_FN_FLUSH); }

/*!Inform \a sink that no more data will be written.  This implies flushing
 * any buffered data and closing resources.  Some sinks may return constructed
 * data. */
CU_SINLINE void *
cu_dsink_finish(cu_dsink_t sink)
{ return (void *)cu_dsink_control(sink, CU_DSINK_FN_FINISH); }

/*!Inform \a sink that no more data will be written, and that the operation is
 * considered failed.  This means any buffered data may be discarded and
 * resources closed.  Operations on external resources such as file creation or
 * database updates, may or may not be reverted. */
CU_SINLINE void
cu_dsink_discard(cu_dsink_t sink)
{ cu_dsink_control(sink, CU_DSINK_FN_DISCARD); }

/*!Creates a clog-free sink of \a subsink by buffering data. */
cu_dsink_t cu_dsink_new_buffer(cu_dsink_t subsink);

/*!@}*/
CU_END_DECLARATIONS

#endif
