/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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
#include <stdio.h>
#include <signal.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_debug cu/debug.h: Utilities for Debugging
 * @{\ingroup cu_util_mod */

#ifdef CUCONF_DEBUG_CLIENT
#  define cuP_dlog_def(storage, name, ...)				\
    storage char const *name##_debug_keys[] = {__VA_ARGS__, NULL};	\
    storage struct cu_log_facility_s name##_debug_log = {		\
	.severity = CU_LOG_DEBUG, .origin = CU_LOG_LOGIC,		\
	.keys = name##_debug_keys,					\
	.flags = CU_LOG_FLAG_DEBUG_FACILITY | CU_LOG_FLAG_PERMANENT,	\
    }
#  define cu_dlog_def(...) cuP_dlog_def(static, __VA_ARGS__)
#  define cu_dlog_edef(...) cuP_dlog_def(, __VA_ARGS__)
#  define cu_dlog_edec(name) extern struct cu_log_facility_s name##_debug_log
#  define cuP_dlogf(name, fmt, ...) \
    cu_logf(&name##_debug_log, fmt, __FILE__, __LINE__, __VA_ARGS__)
#  define cu_dlogf(...) cuP_dlogf(__VA_ARGS__, NULL)
#else
#  define cu_dlog_def(name) typedef int name##_nodebug_t
#  define cu_dlog_edef(name) typedef int name##_nodebug_t
#  define cu_dlog_edec(name) typedef int name##_nodebug_t
#  define cu_dlogf(...) ((void)0)
#endif

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
void cu_debug_enable_backtrace_on_signal(void);

/*
 *  Debug Messages
 */
void cuP_debug_msg(const char *, int, const char *, int, const char *, ...);
extern void (*cuP_debug_bug_report)(const char *, int, const char *, ...);

void cu_findent(FILE *fp, int i);
void cuP_debug_prep_abort(void);
void cu_debug_abort(void);
void cu_debug_trap(void);
#ifndef CU_NDEBUG
   /* These macros shadow the above functions.  They provide a slight
    * convenience while debugging, as there is one less stack frame in the
    * resulting core dump. */
#  define cu_debug_abort() (cuP_debug_prep_abort(), raise(SIGABRT))
#  define cu_debug_trap(void) raise(SIGTRAP)
#endif

#ifndef CU_NDEBUG

extern cu_bool_t cuP_debug_is_enabled;

cu_bool_t cu_debug_is_enabled(cu_debug_flags_t);
#define cu_debug_inform(args...) \
    cuP_debug_msg(__FILE__, __LINE__, "debug", cu_debug_flag_info, args)
#define cu_debug_warning(args...) \
    cuP_debug_msg(__FILE__, __LINE__, "warning", 0, args)
#define cu_debug_error(args...) \
    (cuP_debug_msg(__FILE__, __LINE__, "error", 0, args), cu_debug_abort())
#define cu_debug_assert(test) \
    ((test)? (void)0 : (void)cu_debug_error("assertion '%s' failed", #test))
void cu_debug_indent();
void cu_debug_unindent();
#define cu_debug_here() cu_debug_inform("here")
#define cu_debug_assert_once()						\
    do {								\
	static int cuL_cnt = 0;						\
	if (cuL_cnt++ != 0)						\
	    cu_debug_error("This point should only have been reached once."); \
    } while (0)

#else

#define cu_debug_is_enabled(flags) cu_false
#define cu_debug_inform(args...) ((void)0)
#define cu_debug_warning(args...) ((void)0)
#define cu_debug_error(args...)						\
    (cuP_debug_bug_report(__FILE__, __LINE__, args), cu_debug_abort())
#define cu_debug_assert(test) ((void)0)
#define cu_debug_indent() ((void)0)
#define cu_debug_unindent() ((void)0)
#define cu_debug_here() ((void)0)
#define cu_debug_assert_once() ((void)0)

#endif

#define cu_debug_unreachable() \
	cu_debug_error("This point should not have been reached.")
#define cu_unreachable() cu_debug_unreachable()

#ifndef CU_NDEBUG
void cuP_dprintf(char const *, int, char const *, char const *, ...);
#define cu_dprintf(args...) cuP_dprintf(__FILE__, __LINE__, args)
cu_bool_t cu_debug_key(char const *key);
void cu_debug_enable_key(char const *key);
void cu_debug_disable_key(char const *key);
#else
#define cu_dprintf(args...) ((void)0)
#define cu_debug_key(key) cu_false
#define cu_debug_enable_key(key) ((void)0)
#define cu_debug_disable_key(key) ((void)0)
#endif


/* Client Debug
 * ============ */

void cu_err_invalid_arg(char const *function, int argnum, char const *argname,
			char const *msg);
#ifdef CUCONF_DEBUG_CLIENT
#define cu_check_arg(argnum, argname, test)				\
    ((test)? ((void)0)							\
	   : cu_err_invalid_arg(__FUNCTION__, argnum, #argname,		\
				"must fulfil "#test))
#else
#define cu_check_arg(argnum, argname, test) ((void)0)
#endif

/*!@}*/
CU_END_DECLARATIONS

#endif
