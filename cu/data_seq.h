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

#ifndef CU_DATA_SEQ_H
#define CU_DATA_SEQ_H

#include <cu/fwd.h>
#include <stdarg.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_data_seq_h cu/data_seq.h: Data Sources and Sinks
 *@{\ingroup cu_mod */

#define CU_DATA_CONTROL_FLUSH 1
#define CU_DATA_CONTROL_DISCARD 2
#define CU_DATA_CONTROL_FINISH 3
#define CU_DATA_STATUS_UNIMPL ((cu_word_t)-1)

struct cu_data_source_s
{
    size_t (*read)(cu_data_source_t source, void *buf, size_t max_size);
    cu_word_t (*control)(cu_data_source_t source, int op, va_list);
};

CU_SINLINE void
cu_data_source_init(cu_data_source_t source,
		    size_t (*read)(cu_data_source_t, void *, size_t),
		    cu_word_t (*control)(cu_data_source_t, int, va_list))
{
    source->read = read;
    source->control = control;
}

CU_SINLINE size_t
cu_data_source_read(cu_data_source_t source, void *buf, size_t max_size)
{
    return (*source->read)(source, buf, max_size);
}

CU_SINLINE cu_word_t
cu_data_source_control_va(cu_data_source_t source, int op, va_list va)
{
    return (*source->control)(source, op, va);
}

cu_word_t cu_data_source_control(cu_data_source_t source, int op, ...);


struct cu_data_sink_s
{
    size_t (*write)(cu_data_sink_t sink, void const *buf, size_t max_size);
    cu_word_t (*control)(cu_data_sink_t source, int op, va_list);
};

CU_SINLINE void
cu_data_sink_init(cu_data_sink_t sink,
		  size_t (*write)(cu_data_sink_t, void const *, size_t),
		  cu_word_t (*control)(cu_data_sink_t, int, va_list))
{
    sink->write = write;
    sink->control = control;
}

CU_SINLINE size_t
cu_data_sink_write(cu_data_sink_t sink, void const *buf, size_t max_size)
{
    return (*sink->write)(sink, buf, max_size);
}

CU_SINLINE cu_word_t
cu_data_sink_control_va(cu_data_sink_t sink, int op, va_list va)
{
    return (*sink->control)(sink, op, va);
}

cu_word_t cu_data_sink_control(cu_data_sink_t sink, int op, ...);

CU_SINLINE void
cu_data_sink_flush(cu_data_sink_t sink)
{ cu_data_sink_control(sink, CU_DATA_CONTROL_FLUSH); }

CU_SINLINE void *
cu_data_sink_finish(cu_data_sink_t sink)
{ return (void *)cu_data_sink_control(sink, CU_DATA_CONTROL_FINISH); }

CU_SINLINE void
cu_data_sink_discard(cu_data_sink_t sink)
{ cu_data_sink_control(sink, CU_DATA_CONTROL_DISCARD); }


/*!@}*/
CU_END_DECLARATIONS

#endif
