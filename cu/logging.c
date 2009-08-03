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
#include <cu/diag.h>
#include <cu/memory.h>
#include <stdio.h>
#include <string.h>

static void
_log_also(cu_sref_t sref, char const *skind)
{
    if (!sref)
	return;
    while ((sref = cu_sref_chain_tail(sref))) {
	cu_sref_fprint(sref, stderr);
	if (skind) {
	    fputs(skind, stderr);
	    fputs(": ", stderr);
	}
	fputs("As above.\n", stderr);
    }
}

cu_clop_def(_noop_vlogf, void,
	    cu_log_facility_t facility, cu_sref_t loc,
	    char const *fmt, va_list va)
{
}

cu_clop_def(_default_vlogf, void,
	    cu_log_facility_t facility, cu_sref_t loc,
	    char const *fmt, va_list va)
{
    cu_bool_t have_debug_loc, have_fmt_loc, have_any_loc;
#ifdef CUCONF_COMPACT_LOG
    char s0, s1;
#else
    char *skind, *s0, *s1;
#endif
    have_fmt_loc = fmt[0] == '%' && fmt[1] == ':';
    have_debug_loc = facility->flags & CU_LOG_FLAG_DEBUG_FACILITY;
    have_any_loc = have_fmt_loc || have_debug_loc || loc;
#ifdef CUCONF_COMPACT_LOG
    switch (cu_log_facility_origin(facility)) {
	case CU_LOG_LOGIC:	s0 = 'l'; break;
	case CU_LOG_SYSTEM:	s0 = 's'; break;
	case CU_LOG_USER:	s0 = 'u'; break;
	default:		s0 = '?'; break;
    }
    switch (cu_log_facility_severity(facility)) {
	case CU_LOG_DEBUG:	s1 = 'd'; break;
	case CU_LOG_INFO:	s1 = 'i'; break;
	case CU_LOG_NOTICE:	s1 = 'n'; break;
	case CU_LOG_WARNING:	s1 = 'w'; break;
	case CU_LOG_ERROR:	s1 = 'e'; break;
	case CU_LOG_FAILURE:	s1 = 'f'; break;
	default:		s1 = '?'; break;
    }
    skind = cu_salloc(6);
    sprintf(skind, "[%c/%c] ", s0, s1);
    fprintf(stderr, "[%c/%c] ", s0, s1);
#else
    switch (cu_log_facility_origin(facility)) {
	case CU_LOG_LOGIC:	s0 = "logic"; break;
	case CU_LOG_SYSTEM:	s0 = "sys"; break;
	case CU_LOG_USER:	s0 = NULL; break;
	default:		s0 = "?"; break;
    }
    switch (cu_log_facility_severity(facility)) {
	case CU_LOG_DEBUG:	s1 = "debug"; break;
	case CU_LOG_INFO:	s1 = "info"; break;
	case CU_LOG_NOTICE:	s1 = "notice"; break;
	case CU_LOG_WARNING:	s1 = "warning"; break;
	case CU_LOG_ERROR:	s1 = have_any_loc? NULL : "error"; break;
	case CU_LOG_FAILURE:	s1 = "failure"; break;
	default:		s1 = "?"; break;
    }
#endif

    if (have_debug_loc) {
	char const *file = va_arg(va, char const *);
	int line = va_arg(va, int);
	fprintf(stderr, "%s:%d: ", file, line);
    }
    if (loc) {
	cu_sref_fprint(loc, stderr);
	fputs(": ", stderr);
    }
    if (have_fmt_loc) {
	cu_sref_t loc = va_arg(va, cu_sref_t);
	cu_sref_fprint(loc, stderr);
	fputs(": ", stderr);
	fmt += 2;
    }

#ifndef CUCONF_COMPACT_LOG
    if (s0) {
	if (s1) {
	    int len0 = strlen(s0);
	    skind = cu_salloc(len0 + strlen(s1) + 2);
	    strcpy(skind, s0);
	    skind[len0] = ' ';
	    strcpy(skind + len0 + 1, s1);
	} else
	    skind = s0;
    } else
	skind = s1;
    if (skind)
	fprintf(stderr, "%s: ", s1);
#endif

    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);

    /* Print the also-lines when loc is passed to an _at function for backwards
     * compatibility.  Pending solution for the new %: format. */
    if (loc)
	_log_also(loc, skind);
}

cu_clop_def(_default_log_binder, cu_bool_t, cu_log_facility_t facility)
{
    facility->vlogf = _default_vlogf;
    return cu_true;
}

static cu_log_binder_t _current_log_binder = cu_clop_ref(_default_log_binder);
static cu_log_facility_t _permanent_facility_chain = NULL;

cu_bool_t cuP_debug_facility_enabled(cu_log_facility_t facility);

cu_log_binder_t
cu_register_log_binder(cu_log_binder_t log_binder)
{
    cu_log_facility_t facility;
    cu_log_binder_t old_log_binder = _current_log_binder;
    _current_log_binder = log_binder;
    for (facility = _permanent_facility_chain; facility;
	 facility = facility->next)
	if (!(facility->flags & CU_LOG_FLAG_DEBUG_FACILITY) ||
	    cuP_debug_facility_enabled(facility))
	    cu_call(_current_log_binder, facility);
	else
	    facility->vlogf = cu_clop_ref(_noop_vlogf);
    return old_log_binder;
}

void
cu_register_permanent_log(cu_log_facility_t facility)
{
    facility->next = _permanent_facility_chain;
    _permanent_facility_chain = facility;
    if (!(facility->flags & CU_LOG_FLAG_DEBUG_FACILITY) ||
	cuP_debug_facility_enabled(facility))
	cu_call(_current_log_binder, facility);
    else
	facility->vlogf = cu_clop_ref(_noop_vlogf);
}

void
cu_register_transient_log(cu_log_facility_t facility)
{
    if (!(facility->flags & CU_LOG_FLAG_DEBUG_FACILITY) ||
	cuP_debug_facility_enabled(facility))
	cu_call(_current_log_binder, facility);
    else
	facility->vlogf = cu_clop_ref(_noop_vlogf);
}

static void
_prepare_facility(cu_log_facility_t facility)
{
    if (facility->flags & CU_LOG_FLAG_PERMANENT)
	cu_register_permanent_log(facility);
    else if (facility->flags & CU_LOG_FLAG_TRANSIENT)
	cu_register_transient_log(facility);
    else
	cu_bugf("Uninitialised log facility.");
}

void
cu_vlogf(cu_log_facility_t facility, char const *fmt, va_list va)
{
    if (!facility->vlogf)
	_prepare_facility(facility);
    cu_call(facility->vlogf, facility, NULL, fmt, va);
}

void
cu_vlogf_at(cu_log_facility_t facility, cu_sref_t loc,
	    char const *fmt, va_list va)
{
    if (!facility->vlogf)
	_prepare_facility(facility);
    cu_call(facility->vlogf, facility, loc, fmt, va);
}

void
cu_logf(cu_log_facility_t facility, char const *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    cu_vlogf(facility, fmt, va);
    va_end(va);
}

void
cu_logf_at(cu_log_facility_t facility, cu_sref_t loc, char const *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    cu_vlogf_at(facility, loc, fmt, va);
    va_end(va);
}
