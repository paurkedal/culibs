/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUOS_PROCESS_H
#define CUOS_PROCESS_H

#include <stdio.h>
#include <sys/types.h>
#include <cucon/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuos_process cuos/process.h: Running external processes
 * @{ \ingroup cuos_mod */

/*!Fork the current process and call \code execvp(prg, argv)\endcode in the
 * subprocess.  If \a fd_arr is not NULL, redirect file descriptor \e i
 * to \a fd_arr[i] iff \a fd_arr[i] ≠ -1.  The given file
 * descriptors will be closed by this function.  The process id is
 * returned.
 *
 * \arg fd_arr[0] file descriptor for \c stdin in the subprocess, or -1
 * \arg fd_arr[1] file descriptor for \c stdout in the subprocess, or -1
 * \arg fd_arr[2] file descriptor for \c stderr in the subprocess, or -1 */
pid_t cuos_start_prog_fd(char const *prg, char const *const *argv,
			 char *const *envp, int *fd_arr);

/*!Fork the current process and call \code execvp(prg, argv)\endcode in the
 * subprocess.  For each \a in, \a out, \a err which is non-\c NULL, return
 * an open file to the \c stdin, \c stdout, \c stderr of the subprocess.
 * Note that \c stdin is opened for writing and the others are opened for
 * reading. */
pid_t cuos_start_prog_io(char const *prg, char const *const *argv,
			 char *const *envp,
			 FILE **in, FILE **out, FILE **err);

/*!Shortcut for \ref cuos_start_prog_fd when no redirections are needed. */
CU_SINLINE pid_t cuos_start_prog(char const *prg, char const *const *argv,
				 char *const *envp)
{ return cuos_start_prog_fd(prg, argv, envp, NULL); }

/*!Wait for the process with process id \a pid as retured by
 * \ref cuos_start_prog_fd, \ref cuos_start_prog_io, or
 * \ref cuos_start_prog.  \a prg should be the same as the first argument
 * of the corresponding start-function. */
int cuos_wait_for_prog(char const *prg, pid_t pid);

/*!Start in a sub-process and wait for the program \a prg with arguments
 * \a argv and environment \a envp.  See \ref cuos_start_prog for details. */
int cuos_call_prog(char const *prg, char const *const *argv,
		   char *const *envp);

/*!Start in a sub-process and wait for the program \a prg with arguments
 * \a argv, environment \a envp and redirections \a fd_arr. See
 * \ref cuos_start_prog_fd for details. */
int cuos_call_prog_fd(char const *prg, char const *const *argv,
		      char *const *envp, int *fd_arr);

/*!Construct a \c NULL terminated argv from a \c cucon_list_t of
 * \c cu_str_t.  \a argv must have at least
 * <tt>\ref cucon_list_count (strlist) + 1</tt>
 * elements. */
void cuos_argv_cct_strlist(char const **argv, cucon_list_t strlist);

/*!Returns 'cstr' with shell special chars (including space) escaped.
 * May quote more than strictly needed. */
cu_str_t cuos_shell_escape_cstr(char const *cstr);

/*!Print out \a argv to \a out. */
void cuos_argv_fprint(char const **argv, FILE *out);

/* @} */
CU_END_DECLARATIONS

#endif
