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
/*!\defgroup cu_data_seq_h cu/data_seq.h: Generic Data Sink
 *@{\ingroup cu_mod */

#define CU_DSINK_FN_FLUSH 1
#define CU_DSINK_FN_DISCARD 2
#define CU_DSINK_FN_FINISH 3
#define CU_DSINK_ST_SUCCESS 0
#define CU_DSINK_ST_UNIMPL ((cu_word_t)-1)

struct cu_dsink_s
{
    cu_word_t (*control)(cu_dsink_t source, int op, va_list);
    size_t (*write)(cu_dsink_t sink, void const *buf, size_t max_size);
};

CU_SINLINE void
cu_dsink_init(cu_dsink_t sink,
	      cu_word_t (*control)(cu_dsink_t, int, va_list),
	      size_t (*write)(cu_dsink_t, void const *, size_t))
{
    sink->control = control;
    sink->write = write;
}

CU_SINLINE size_t
cu_dsink_write(cu_dsink_t sink, void const *buf, size_t max_size)
{
    return (*sink->write)(sink, buf, max_size);
}

CU_SINLINE cu_word_t
cu_dsink_control_va(cu_dsink_t sink, int op, va_list va)
{
    return (*sink->control)(sink, op, va);
}

cu_word_t cu_dsink_control(cu_dsink_t sink, int op, ...);

CU_SINLINE void
cu_dsink_flush(cu_dsink_t sink)
{ cu_dsink_control(sink, CU_DSINK_FN_FLUSH); }

CU_SINLINE void *
cu_dsink_finish(cu_dsink_t sink)
{ return (void *)cu_dsink_control(sink, CU_DSINK_FN_FINISH); }

CU_SINLINE void
cu_dsink_discard(cu_dsink_t sink)
{ cu_dsink_control(sink, CU_DSINK_FN_DISCARD); }

/*!@}*/
CU_END_DECLARATIONS

#endif
