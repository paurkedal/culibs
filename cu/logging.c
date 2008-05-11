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

#include <cu/logging.h>
#include <cu/sref.h>
#include <stdio.h>

cu_clop_def(cuP_default_vlogf, void,
	    cu_log_facility_t facility, char const *fmt, va_list va)
{
    cu_bool_t have_loc = fmt[0] == '%' && fmt[1] == ':';
#ifdef CUCONF_COMPACT_LOG
    char s0, s1;
    switch (facility->origin) {
	case CU_LOG_LOGIC:	s0 = 'l'; break;
	case CU_LOG_SYSTEM:	s0 = 's'; break;
	case CU_LOG_USER:	s0 = 'u'; break;
	default:		s0 = '?'; break;
    }
    switch (facility->severity) {
	case CU_LOG_DEBUG:	s1 = 'd'; break;
	case CU_LOG_INFO:	s1 = 'i'; break;
	case CU_LOG_NOTICE:	s1 = 'n'; break;
	case CU_LOG_WARNING:	s1 = 'w'; break;
	case CU_LOG_ERROR:	s1 = 'e'; break;
	case CU_LOG_FAILURE:	s1 = 'f'; break;
	default:		s1 = '?'; break;
    }
    fprintf(stderr, "[%c/%c] ", s0, s1);
#else
    char *s0, *s1;
    switch (facility->origin) {
	case CU_LOG_LOGIC:	s0 = "logic"; break;
	case CU_LOG_SYSTEM:	s0 = "sys"; break;
	case CU_LOG_USER:	s0 = NULL; break;
	default:		s0 = "?"; break;
    }
    switch (facility->severity) {
	case CU_LOG_DEBUG:	s1 = "debug"; break;
	case CU_LOG_INFO:	s1 = "info"; break;
	case CU_LOG_NOTICE:	s1 = "notice"; break;
	case CU_LOG_WARNING:	s1 = "warning"; break;
	case CU_LOG_ERROR:	s1 = have_loc? NULL : "error"; break;
	case CU_LOG_FAILURE:	s1 = "failure"; break;
	default:		s1 = "?"; break;
    }
    if (s0) {
	if (s1)
	    fprintf(stderr, "%s %s: ", s0, s1);
	else
	    fprintf(stderr, "%s: ", s0);
    } else if (s1)
	fprintf(stderr, "%s: ", s1);
#endif

    if (have_loc) {
	cu_sref_t loc = va_arg(va, cu_sref_t);
	cu_sref_fprint(loc, stderr);
	fputs(": ", stderr);
	fmt += 2;
    }

    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
}

cu_clop_def(cuP_default_log_binder, cu_bool_t, cu_log_facility_t facility)
{
    facility->vlogf = cuP_default_vlogf;
    return cu_true;
}

static cu_log_binder_t cuP_log_binder = cu_clop_ref(cuP_default_log_binder);
static cu_log_facility_t cuP_log_facility_chain = NULL;

cu_log_binder_t
cu_register_log_binder(cu_log_binder_t log_binder)
{
    cu_log_facility_t facility;
    cu_log_binder_t old_log_binder = cuP_log_binder;
    cuP_log_binder = log_binder;
    for (facility = cuP_log_facility_chain; facility;
	 facility = facility->next)
	cu_call(cuP_log_binder, facility);
    return old_log_binder;
}

void
cu_register_permanent_log(cu_log_facility_t facility)
{
    facility->next = cuP_log_facility_chain;
    cuP_log_facility_chain = facility;
    cu_call(cuP_log_binder, facility);
}

void
cu_register_transient_log(cu_log_facility_t facility)
{
    cu_call(cuP_log_binder, facility);
}
