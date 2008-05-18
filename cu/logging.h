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

typedef cu_clop(cu_vlogf_t, void,
		cu_log_facility_t facility, cu_sref_t loc,
		char const *fmt, va_list va);

typedef cu_clop(cu_log_binder_t, cu_bool_t, cu_log_facility_t new_facility);

#define CU_LOG_FLAG_PERMANENT 1
#define CU_LOG_FLAG_TRANSIENT 2
#define CU_LOG_FLAG_DEBUG_FACILITY 4

struct cu_log_facility_s
{
    cu_log_severity_t severity : 8;
    cu_log_origin_t origin : 8;
    unsigned int flags : 16;
    char const **keys;
    cu_log_facility_t next;
    cu_vlogf_t vlogf;
};

/*!Register a new log-binder.  A log-binder will be called with all existing
 * logs and logs created in the future.  It's purpose is to assign a suitable
 * vlogf implementation to log facilities based on the various meta-information
 * stored in the facility.  E.g. it may decide to direct error messages to a
 * system log and to mail critical errors to an email account.
 *
 * The vlogf function assigned by \a binder must be thread-safe. */
cu_log_binder_t cu_register_log_binder(cu_log_binder_t binder);

/*!Register a log facility which will persist throughout the life-time of the
 * process.  If \ref cu_register_log_binder is called later, the new binder
 * will process \a facility, allowing it to change the vlogf implementation. */
void cu_register_permanent_log(cu_log_facility_t facility);

/*!Register a transient logging facility.  As opposed to \ref
 * cu_register_permanent_log, this facility will be passed only to the current
 * log-binder, and not any future ones. */
void cu_register_transient_log(cu_log_facility_t facility);

/*!Run the vlogf implementation of \a facility. */
void cu_vlogf(cu_log_facility_t facility, char const *fmt, va_list va);

/*!Run the vlogf implementation of \a facility. */
void cu_logf(cu_log_facility_t facility, char const *fmt, ...);

/*!Run the vlogf implementation of \a facility with a source location. */
void cu_vlogf_at(cu_log_facility_t facility, cu_sref_t loc,
		 char const *fmt, va_list va);

/*!@}*/
CU_END_DECLARATIONS

#endif
