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

#ifndef CU_DSOURCE_H
#define CU_DSOURCE_H

#include <cu/fwd.h>
#include <stdarg.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_dsource_h cu/dsource.h: Generic Data Source
 *@{\ingroup cu_seq_mod */

#define CU_DSOURCE_FN_CLOSE 1
#define CU_DSOURCE_ST_SUCCESS 0
#define CU_DSOURCE_ST_UNIMPL ((cu_word_t)-1)

struct cu_dsource_s
{
    cu_word_t (*control)(cu_dsource_t source, int op, va_list);
    size_t (*read)(cu_dsource_t source, void *buf, size_t max_size);
};

CU_SINLINE void
cu_dsource_init(cu_dsource_t source,
		cu_word_t (*control)(cu_dsource_t, int, va_list),
		size_t (*read)(cu_dsource_t, void *, size_t))
{
    source->read = read;
    source->control = control;
}

CU_SINLINE size_t
cu_dsource_read(cu_dsource_t source, void *buf, size_t max_size)
{
    return (*source->read)(source, buf, max_size);
}

CU_SINLINE cu_word_t
cu_dsource_control_va(cu_dsource_t source, int op, va_list va)
{
    return (*source->control)(source, op, va);
}

cu_word_t cu_dsource_control(cu_dsource_t source, int op, ...);

/*!@}*/
CU_END_DECLARATIONS

#endif
