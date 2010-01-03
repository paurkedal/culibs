/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2009  Petter Urkedal <urkedal@nbi.dk>
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


#ifndef CU_DEBUG_H
#define CU_DEBUG_H

#include <cu/fwd.h>
#include <cu/conf.h>
#include <cu/inherit.h>
#include <cu/logging.h>
#include <cu/diag.h>
#include <stdio.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cu_debug_h cu/debug.h: Utilities for Debugging
 ** @{ \ingroup cu_util_mod */

#if !defined(CU_NDEBUG) || defined(CU_IN_DOXYGEN)
#  ifndef CU_IN_DOXYGEN
#    define cuP_dlog_def(storage, name, ...)				\
	storage char const *name##_debug_keys[] = {__VA_ARGS__, NULL};	\
	storage struct cu_log_facility name##_debug_log =		\
	    CU_LOG_FACILITY_INITIALISER(				\
		CU_LOG_DEBUG, CU_LOG_LOGIC, name##_debug_keys,		\
		CU_LOG_FLAG_DEBUG_FACILITY | CU_LOG_FLAG_PERMANENT)
#    define cuP_dlogf(name, fmt, ...) \
	cu_logf(&name##_debug_log, fmt, __FILE__, __LINE__, __VA_ARGS__)
#  endif

/** Emits a definition of a debug logger identified by \a name using static
 ** linkage.  The additional arguments are strings which are used to enable the
 ** logger at program startup.  The default log binder looks for strings of the
 ** form \c "dtag=FOO" where \e FOO by convention is a period-separated
 ** qualified name used to identify the current source file or topic for
 ** debugging.  Logging is only enabled for dtags listed in the file pointed to
 ** by \ref culibs_environ_page "\c $CU_DTAGS_PATH". */
#  define cu_dlog_def(name, ...) cuP_dlog_def(static, name, __VA_ARGS__)

/** Emits a definition of a debug logger identified by \a name using extern
 ** linkage.  See \ref cu_dlog_def for details. */
#  define cu_dlog_edef(name, ...) cuP_dlog_def(, name, __VA_ARGS__)

/** Emits an extern declaration corresponding to a \ref cu_dlog_edef. */
#  define cu_dlog_edec(name) extern struct cu_log_facility name##_debug_log

/** Logs a message to the debug logger identified by \a name.  The variadic
 ** arguments are similar to \c printf. */
#  define cu_dlogf(name, ...) cuP_dlogf(name, __VA_ARGS__, NULL)

/** If debugging is enabled, prints an error and aborts.  To exit with an error
 ** independent of \c CU_NDEBUG, use \ref cu_bugf. */
#define cu_debug_error(...) cu_bugf_fl(__FILE__, __LINE__, __VA_ARGS__)

/** If debugging is enabled, evaluates \a test and aborts if zero. */
#define cu_debug_assert(test) \
    ((test)? (void)0 : (void)cu_debug_error("assertion '%s' failed", #test))

/** If a lexical instance of this statement is called more than once during the
 ** program lifetime, it aborts with an error message. */
#define cu_debug_assert_once()						\
    do {								\
	static int cuL_cnt = 0;						\
	if (cuL_cnt++ != 0)						\
	    cu_debug_error("This point should only have been reached once."); \
    } while (0)

/** Asserts that the current line is unreachable.  This is the same as \ref
 ** cu_bug_unreachable except for being disabled if \c CU_NDEBUG is defined. */
#define cu_debug_unreachable() cu_bug_unreachable()

#else
#  define cu_dlog_def(name, ...)	CU_END_BOILERPLATE
#  define cu_dlog_edef(name, ...)	CU_END_BOILERPLATE
#  define cu_dlog_edec(name)		CU_END_BOILERPLATE
#  define cu_dlogf(...)			((void)0)
#  define cu_debug_error(...)		((void)0)
#  define cu_debug_assert(test)		((void)0)
#  define cu_debug_assert_once()	((void)0)
#  define cu_debug_unreachable()	((void)0)
#endif

/** Enable the debug tag \a dtag.  This must be called at program
 ** initialisation, before the log facilities which triggers on \a dtag are
 ** used.  It highly preferable to to let cu_init() load the tags automatically
 ** as described in \ref cu_dlog_def. */
void cu_dtag_enable(char const *dtag);

/** Disable the debug tag \a dtag.  The same notice as for \ref cu_dtag_enable
 ** applies. */
void cu_dtag_disable(char const *dtag);

/** Queries whether the debug tag \a dtag is enabled.  Also condition the code
 ** on \#ifndef CU_NDEBUG for efficiency. */
cu_bool_t cu_dtag_get(char const *dtag);


/*
 *  Debugging Selection
 */
typedef enum {
    cu_debug_flag_info		= 1 << 0,
    cu_debug_show_alloc_flag	= 1 << 1,
    cu_debug_soft_exit_flag	= 1 << 2,
    cu_debug_show_collect_flag	= 1 << 3,
    cu_debug_expensive_typing	= 1 << 4
} cu_debug_flags_t;

cu_debug_flags_t cu_debug_enable(cu_debug_flags_t);
cu_debug_flags_t cu_debug_disable(cu_debug_flags_t);
cu_debug_flags_t cu_debug_select(cu_debug_flags_t);

/*
 *  Debug Messages
 */
extern void (*cuP_debug_bug_report)(const char *, int, const char *, ...);

void cu_findent(FILE *fp, int i);
void cu_debug_abort(void) CU_ATTR_NORETURN;
void cu_debug_trap(void);


/* Client Debug
 * ============ */

#ifdef CUCONF_DEBUG_CLIENT
#  define cu_check_arg(argnum, argname, test)				\
	((test)? ((void)0)						\
	       : cu_bugf_domain(argnum, #argname, "must fulfil "#test))
#else
#  define cu_check_arg(argnum, argname, test) ((void)0)
#endif

/** @} */

/* Deprecated 2009-08-25 (down to cu_debug_key). */
void cuP_dprintf(char const *, int, char const *, char const *, ...)
    CU_ATTR_DEPRECATED;
void cuP_debug_msg(const char *, int, const char *, int, const char *, ...)
    CU_ATTR_DEPRECATED;
extern cu_bool_t cuP_debug_is_enabled;
#ifndef CU_NDEBUG
#  define cu_dprintf(...) cuP_dprintf(__FILE__, __LINE__, __VA_ARGS__)
#  define cu_debug_inform(...) \
	cuP_debug_msg(__FILE__,__LINE__,"debug",cu_debug_flag_info,__VA_ARGS__)
#  define cu_debug_warning(...) \
	cuP_debug_msg(__FILE__,__LINE__,"warning", 0, __VA_ARGS__)
#  define cu_debug_here() cu_debug_inform("here")
cu_bool_t cu_debug_is_enabled(cu_debug_flags_t);
void cu_debug_indent() CU_ATTR_DEPRECATED;
void cu_debug_unindent() CU_ATTR_DEPRECATED;
#else
#  define cu_dprintf(...) ((void)0)
#  define cu_debug_inform(...) ((void)0)
#  define cu_debug_warning(...) ((void)0)
#  define cu_debug_here() ((void)0)
#  define cu_debug_is_enabled(flags) cu_false
#  define cu_debug_indent() ((void)0)
#  define cu_debug_unindent() ((void)0)
#endif
#define cu_unreachable() cu_debug_unreachable()
#define cu_debug_key cu_dtag_get

CU_END_DECLARATIONS
#endif
