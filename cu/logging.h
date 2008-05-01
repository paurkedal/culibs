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

#ifndef CU_LOGGING_H
#define CU_LOGGING_H

#include <cu/clos.h>
#include <stdarg.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_logging_h cu/logging.h: Logging (experimental)
 *@{\ingroup cu_mod
 * \note This is experimental and may be changed or removed.
 */

typedef enum {
    CU_LOG_DEBUG,
    CU_LOG_INFO,
    CU_LOG_NOTICE,
    CU_LOG_WARNING,
    CU_LOG_ERROR,
    CU_LOG_FAILURE,
} cu_log_severity_t;

typedef enum {
    CU_LOG_LOGIC,
    CU_LOG_SYSTEM,
    CU_LOG_USER,
} cu_log_origin_t;

typedef cu_clop(cu_log_printer_t, void,
		cu_log_facility_t facility, char const *fmt, va_list va);

typedef cu_clop(cu_log_binder_t, cu_bool_t, cu_log_facility_t new_facility);

struct cu_log_facility_s
{
    cu_log_severity_t severity;
    cu_log_origin_t origin;
    char const **keys;
    cu_log_facility_t next;
    cu_log_printer_t vlogf;
};

cu_log_binder_t cu_register_log_binder(cu_log_binder_t binder);

void cu_register_permanent_log(cu_log_facility_t facility);

void cu_register_transient_log(cu_log_facility_t facility);

CU_SINLINE void
cu_vlogf(cu_log_facility_t facility, char const *fmt, va_list va)
{ cu_call(facility->vlogf, facility, fmt, va); }

/*!@}*/
CU_END_DECLARATIONS

#endif
