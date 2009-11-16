/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cu/diag.h>
#include <cu/logging.h>
#include <cu/sref.h>
#include <cu/debug.h>
#include <atomic_ops.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

/* Diagnostic Functions
 * -------------------- */

static AO_t _error_count;
static AO_t _warning_count;

enum {FI_VERB, FI_WARN, FI_ERR, FI_DEBUG, FI_BUG};
static struct cu_log_facility_s _log_facility_arr[] = {
    {CU_LOG_INFO,	CU_LOG_USER},
    {CU_LOG_WARNING,	CU_LOG_USER},
    {CU_LOG_ERROR,	CU_LOG_USER},
    {CU_LOG_INFO,	CU_LOG_LOGIC},
    {CU_LOG_FAILURE,	CU_LOG_LOGIC},
};

void
cu_diag_define_format_key(char key, cu_diag_format_fn_t fn)
{
    fprintf(stderr,
	    "warning: cu_diag_define_format_key is a noop, use libcufo.\n");
}

void (*cuP_vfprintf)(FILE *, char const *, va_list) = NULL;

void
cu_vfprintf(FILE *file, char const* msg, va_list va)
{
    if (cuP_vfprintf)
	(*cuP_vfprintf)(file, msg, va);
    else
	vfprintf(file, msg, va);
}

void
cu_verrf_at(cu_sref_t srf, char const *msg, va_list va)
{
    AO_fetch_and_add1(&_error_count);
    cu_vlogf_at(&_log_facility_arr[FI_ERR], srf, msg, va);
}

void
cu_errf_at(cu_sref_t srf, char const *msg, ...)
{
    va_list va;
    va_start(va, msg);
    cu_verrf_at(srf, msg, va);
    va_end(va);
}

void
cu_verrf(char const *msg, va_list va)
{
    AO_fetch_and_add1(&_error_count);
    cu_vlogf(&_log_facility_arr[FI_ERR], msg, va);
}

void
cu_errf(char const *msg, ...)
{
    va_list va;
    va_start(va, msg);
    cu_verrf(msg, va);
    va_end(va);
}

void
cu_vwarnf_at(cu_sref_t srf, char const *msg, va_list va)
{
    AO_fetch_and_add1(&_warning_count);
    cu_vlogf_at(&_log_facility_arr[FI_WARN], srf, msg, va);
}

void
cu_warnf_at(cu_sref_t srf, char const *msg, ...)
{
    va_list va;
    va_start(va, msg);
    cu_vwarnf_at(srf, msg, va);
    va_end(va);
}

void
cu_vwarnf(char const* msg, va_list va)
{
    AO_fetch_and_add1(&_warning_count);
    cu_vlogf(&_log_facility_arr[FI_WARN], msg, va);
}

void
cu_warnf(char const* msg, ...)
{
    va_list va;
    va_start(va, msg);
    cu_vwarnf(msg, va);
    va_end(va);
}

void
cu_bugf_at(cu_sref_t srf, char const* msg, ...)
{
    va_list va;
    va_start(va, msg);
    cu_vlogf_at(&_log_facility_arr[FI_BUG], srf, msg, va);
    va_end(va);
    cu_debug_abort();
}

void
cu_bugf_fl(char const *file, int line, char const *msg, ...)
{
    va_list va;
    cu_sref_t srf = cu_sref_new_cstr(file, line, -1);
    va_start(va, msg);
    cu_vlogf_at(&_log_facility_arr[FI_BUG], srf, msg, va);
    va_end(va);
    cu_debug_abort();
}

void
cu_bugf_n(char const* msg, ...)
{
    va_list va;
    va_start(va, msg);
    cu_vlogf(&_log_facility_arr[FI_BUG], msg, va);
    va_end(va);
    cu_debug_abort();
}

int cuP_verbosity = 10;

void
cu_set_verbosity(int level)
{
    cuP_verbosity = level;
}

void
cu_verbf_at(int level, cu_sref_t srf, char const *msg, ...)
{
    va_list va;
    if (level > cuP_verbosity)
	return;
    va_start(va, msg);
    cu_vlogf_at(&_log_facility_arr[FI_VERB], srf, msg, va);
    va_end(va);
}

void
cu_verbf(int level, char const *msg, ...)
{
    va_list va;
    if (level > cuP_verbosity)
	return;
    va_start(va, msg);
    cu_vlogf(&_log_facility_arr[FI_VERB], msg, va);
    va_end(va);
}

void
cu_fprintf(FILE *file, char const *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    cu_vfprintf(file, fmt, va);
    va_end(va);
}

void
cu_handle_syserror(int err_code, char const *proc_name)
{
    char const *msg = strerror(err_code);
    switch (err_code) {
	case 0:
	    cu_bugf("cu_handle_syserror called with err_code = 0.");

	    /* Identify some conditions which should not arise in a valid
	     * program. */
#ifdef EDEADLK
	case EDEADLK:
#endif
#ifdef EDOM
	case EDOM:
#endif
#ifdef EFAULT
	case EFAULT:
#endif
#ifdef EINVAL
	case EINVAL:
#endif
	    cu_bugf("Call to %s failed: %s", proc_name, msg);

	    /* Treat the rest as hard runtime errors. */
	default:
	    cu_errf("Call to %s failed: %s", proc_name, msg);
	    exit(69);
    }
}

void
cuP_diag_init(void)
{
    int i;
    char *s;

    for (i = 0; i < sizeof(_log_facility_arr)/sizeof(_log_facility_arr[0]); ++i)
	cu_register_permanent_log(&_log_facility_arr[i]);

    if ((s = getenv("CU_VERBOSITY"))) {
	int i = atoi(s);
	if (i > 0)
	    cu_set_verbosity(i);
    }
}
