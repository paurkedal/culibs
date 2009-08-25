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

#ifndef CU_DIAG_H
#define CU_DIAG_H

#include <cu/fwd.h>
#include <cu/clos.h>
#include <stdio.h>
#include <stdarg.h>
#include <cu/va_ref.h>

CU_BEGIN_DECLARATIONS

extern int cuP_verbosity;

/** \defgroup cu_diag_h cu/diag.h: Printing and Error Reporting
 ** @{ \ingroup cu_base_mod */

/** A specialised \ref cu_fprintf for printing error messages. */
void cu_errf(char const *fmt, ...);

/** As \a cu_errf, but passing arguments as a \c va_list. */
void cu_verrf(char const *msg, va_list va);

/** A specialised \ref cu_fprintf for printing warnings. */
void cu_warnf(char const *fmt, ...);

/** As \a cu_warnf, but passing arguments as a \c va_list. */
void cu_vwarnf(char const *msg, va_list va);

/** Prints an informative message to if verbosity level is at least
 ** \a level. */
void cu_verbf(int level, char const *fmt, ...);

/** The current verbosity level. */
CU_SINLINE int cu_verbosity() { return cuP_verbosity; }

/** Sets the verbosity level for \ref cu_verbf and \ref cu_verbf_at calls. */
void cu_set_verbosity(int verbosity);

/** Prints an internal error without location and aborts.  Usually \ref cu_bugf
 ** is more useful. */
void cu_bugf_n(char const *fmt, ...) CU_ATTR_NORETURN;

/** Prints an internal error with reference to source code and aborts. */
void cu_bugf_at(cu_sref_t, char const *fmt, ...) CU_ATTR_NORETURN;

/** Prints an internal error with reference to source code and aborts. */
void cu_bugf_fl(char const *file, int line,
		char const *msg, ...) CU_ATTR_NORETURN;

/** Prints an internal error with the location of the callee and aborts.  The
 ** arguments are the same as \ref cu_bugf_n. */
#define cu_bugf(...) cu_bugf_fl(__FILE__, __LINE__, __VA_ARGS__)

/** Asserts that the code point of the macro expansion is unreachable.  For a
 ** version conditioned on \c CU_NDEBUG, see \ref cu_debug_unreachable. */
#define cu_bug_unreachable() \
    cu_bugf_fl(__FILE__, __LINE__, \
	       "This point should not have been reached.")

/** Temporary replacement for unfinished code, which aborts with an error
 ** message. */
#define cu_bug_unfinished() \
    cu_bugf_fl(__FILE__, __LINE__, \
	       "Reached a code point which is not yet written.")

/** Similar to \ref cu_bug_unfinished, but also accepts an argument describing
 ** the unfinished work. */
#define cu_bug_todo(descr) cu_bugf_fl(__FILE__, __LINE__, "TODO: "descr)

void cu_handle_syserror(int err_code, char const *proc_name);

void cu_raise_out_of_memory(size_t size);

CU_SINLINE void
cu_check_out_of_memory(void *ptr, size_t size)
{
    if (!ptr)
	cu_raise_out_of_memory(size);
}



/* --- The rest are deprecated. --- */

typedef cu_clop(cu_diag_format_fn_t, void,
		cu_str_t fmt, cu_va_ref_t va, FILE *out);

/*!Define a new format key for \ref cu_fprintf by mapping \a key to the
 * handler \a fn.  The handler will be passed the format specifier along
 * with reference to the current \c va_list state and the output file.
 * \deprecated This is deprecated, use \ref cufo_mod "libcufo" for extended
 * printing support. */
void cu_diag_define_format_key(char key, cu_diag_format_fn_t fn);

/*!A \c fprintf work-alike which supports registering new keys through \ref
 * cu_diag_define_format_key.  \note The standard format key support is
 * lacking, esp it does not support widths and precision.
 * \deprecated This is deprecated, use \ref cufo_mod "libcufo" for extended
 * printing support. */
void cu_fprintf(FILE *, char const *, ...);

/*!As \ref cu_fprintf, but passing arguments as a \c va_list.
 * \deprecated This is deprecated, use \ref cufo_mod "libcufo" for extended
 * printing support. */
void cu_vfprintf(FILE *, char const *, va_list va);

/*!A specialised \ref cu_fprintf for printing error messages with reference
 * to source code.
 * \deprecated This function is deprecated and does not use the new \ref
 * cu_logging_h "logging" framework.  Use \ref cu_errf with the "%:" format
 * specifier instead. */
void cu_errf_at(cu_sref_t, char const *fmt, ...);

/*!As \a cu_errf_at, but passing arguments as a \c va_list.
 * \deprecated This function is deprecated and does not use the new \ref
 * cu_logging_h "logging" framework.  Use \ref cu_verrf with the "%:" format
 * specifier instead. */
void cu_verrf_at(cu_sref_t srf, char const *msg, va_list va);

/*!A specialised \ref cu_fprintf for printing warnings with reference
 * to source code.
 * \deprecated This function is deprecated and does not use the new \ref
 * cu_logging_h "logging" framework.  Use \ref cu_warnf with the "%:" format
 * specifier instead. */
void cu_warnf_at(cu_sref_t, char const *fmt, ...);

/*!As \a cu_warnf_at, but passing arguments as a \c va_list.
 * \deprecated This function is deprecated and does not use the new \ref
 * cu_logging_h "logging" framework.  Use \ref cu_vwarnf with the "%:" format
 * specifier instead. */
void cu_vwarnf_at(cu_sref_t srf, char const *msg, va_list va);

/*!Prints an informative message with reference to source code if verbosity
 * level is at least \a level.
 * \deprecated This function is deprecated and does not use the new \ref
 * cu_logging_h "logging" framework.  Use \ref cu_verbf with the "%:" format
 * specifier instead. */
void cu_verbf_at(int level, cu_sref_t, char const *fmt, ...);


/** @} */
CU_END_DECLARATIONS

#endif
