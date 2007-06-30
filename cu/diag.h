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
/*!\defgroup cu_diag cu/diag.h: Printing and Error Reporting
 * @{ \ingroup cu_mod */

typedef cu_clop(cu_diag_format_fn_t, void,
		cu_str_t fmt, cu_va_ref_t va, FILE *out);

/*!Define a new format key for \ref cu_fprintf by mapping \a key to the
 * handler \a fn.  The handler will be passed the format specifier along
 * with reference to the current \c va_list state and the output file. */
void cu_diag_define_format_key(char key, cu_diag_format_fn_t fn);

/*!A \c fprintf work-alike which supports registering new keys through \ref
 * cu_diag_define_format_key.  \note The standard format key support is
 * lacking, esp it does not support widths and precision. */
void cu_fprintf(FILE *, char const *, ...);

/*!As \ref cu_fprintf, but passing arguments as a \c va_list. */
void cu_vfprintf(FILE *, char const *, va_list va);


/*!A specialised \ref cu_fprintf for printing error messages. */
void cu_errf(char const *fmt, ...);

/*!A specialised \ref cu_fprintf for printing error messages with reference
 * to source code. */
void cu_errf_at(cu_sref_t, char const *fmt, ...);

/*!As \a cu_errf, but passing arguments as a \c va_list. */
void cu_verrf(char const *msg, va_list va);

/*!As \a cu_errf_at, but passing arguments as a \c va_list. */
void cu_verrf_at(cu_sref_t srf, char const *msg, va_list va);


/*!A specialised \ref cu_fprintf for printing warnings. */
void cu_warnf(char const *fmt, ...);

/*!A specialised \ref cu_fprintf for printing warnings with reference
 * to source code. */
void cu_warnf_at(cu_sref_t, char const *fmt, ...);

/*!As \a cu_warnf, but passing arguments as a \c va_list. */
void cu_vwarnf(char const *msg, va_list va);

/*!As \a cu_warnf_at, but passing arguments as a \c va_list. */
void cu_vwarnf_at(cu_sref_t srf, char const *msg, va_list va);


extern int cuP_verbosity;

/*!Prints an informative message to if verbosity level is at least
 * \a level. */
void cu_verbf(int level, char const *fmt, ...);

/*!Prints an informative message with reference to source code if verbosity
 * level is at least \a level. */
void cu_verbf_at(int level, cu_sref_t, char const *fmt, ...);

/*!The current verbosity level. */
CU_SINLINE int cu_verbosity() { return cuP_verbosity; }

/*!Sets the verbosity level for \ref cu_verbf and \ref cu_verbf_at calls. */
void cu_set_verbosity(int verbosity);


/*!Prints an internal error and aborts. */
void cu_bugf(char const *fmt, ...) CU_ATTR_NORETURN;

/*!Prints an internal error with reference to source code and aborts. */
void cu_bugf_at(cu_sref_t, char const *fmt, ...) CU_ATTR_NORETURN;

/*!Prints an internal error with reference to source code and aborts. */
void cu_bugf_fl(char const *file, int line,
		char const *msg, ...) CU_ATTR_NORETURN;

void cu_raise_out_of_memory(size_t size);

CU_SINLINE void
cu_check_out_of_memory(void *ptr, size_t size)
{
    if (!ptr)
	cu_raise_out_of_memory(size);
}

/*!@}*/
CU_END_DECLARATIONS

#endif
