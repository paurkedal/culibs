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

#ifndef CUOS_FS_H
#define CUOS_FS_H

#include <cucon/fwd.h>
#include <cu/clos.h>
#include <sys/types.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cuos_fs cuos/fs.h: File System
 ** @{ \ingroup cuos_mod
 **
 ** These functions probes or manipulates the file system and related metadata.
 ** \see cuos_path_h "cuos/path.h" for manipulating file paths.
 ** \see cuos_file_h "cuos/file.h" for accessing regular files.
 **/

/*!Catogorisation of files, directories, special files. */
typedef enum {
    cuos_dentry_type_none,	/*!< a null value */
    cuos_dentry_type_file,	/*!< a normal file */
    cuos_dentry_type_dir,	/*!< a directory */
    cuos_dentry_type_char_dev,	/*!< a character device special file */
    cuos_dentry_type_block_dev,	/*!< a block device special file */
    cuos_dentry_type_fifo,	/*!< a first-in first-out pipe */
    cuos_dentry_type_symlink,	/*!< a symbolic link */
    cuos_dentry_type_socket,	/*!< a socket special file */
    cuos_dentry_type_unknown	/*!< undetectable or unknown file type */
} cuos_dentry_type_t;

/** The directory entry type of \a path as on the file system. */
cuos_dentry_type_t cuos_dentry_type(cu_str_t path);

/** True iff \a path names a detectable directory entry on the local file
 ** system, and it is not a broken symbolic link. */
cu_bool_t cuos_have_dentry(cu_str_t path);

/** True iff \a path names a detectable directory or link to such on the local
 ** file system. */
cu_bool_t cuos_have_dir(cu_str_t path);

/** True iff \a path names a detectable regular file or link to such on the
 ** local file system. */
cu_bool_t cuos_have_file(cu_str_t path);

/** True iff \a path names a detectable symbolic link on the local file system.
 ** Returns true also for broken symbolic links. */
cu_bool_t cuos_have_link(cu_str_t path);

/** Return the modification time of \a path, or 0 if \a path does not exist.
 ** On other errors, write an error message and return 0. */
time_t cuos_mtime(cu_str_t path);

/** Create all missing components of \a path and \a path itself as a
 ** directory. */
cu_bool_t cuos_mkdir_rec(cu_str_t path, mode_t mode);

/*!Removes \a path, and if it is a directory, descends and removes all
 * files and subdirectories recursively.
 * \warning This may wipe out all your files if used carelessly.  Make
 * sure you carefully control the argument passed to this function. */
cu_bool_t cuos_remove_rec(cu_str_t path);

/*!Run \code cu_call(cb, path)\endcode on each \e path which is \a dname or
 * a subdirectory or file under \a dname, using depth-first with no specific
 * ordering within each directory. */
cu_bool_t cuos_dirrec_conj_files(cu_str_t dname,
				 cu_clop(cb, cu_bool_t, cu_str_t));

/*!Run \code cu_call(cb, path)\endcode on each \e path which is \a dname or
 * a subdirectory or file under \a dname, using a depth-first with collating
 * order of file names within each directory. */
cu_bool_t cuos_dirreccoll_conj_files(cu_str_t dname,
				     cu_clop(cb, cu_bool_t, cu_str_t));

/** \deprecated Use \ref cuos_dirpile. */
cu_bool_t
cuos_prefixsearch_conj(cucon_list_t prefixlist, cu_str_t suffix,
		       cu_clop(cb, cu_bool_t, cu_str_t result));

/** \deprecated Use \ref cuos_dirpile. */
cu_str_t cuos_prefixsearch_first(cucon_list_t prefixlist, cu_str_t suffix);

/** \deprecated Use \ref cuos_dirpile. */
void cuos_prefixsearch_append_all(cucon_list_t prefixlist, cu_str_t suffix,
				  cucon_list_t result);

/** Returns a temporary directory for the current process, which will be erased
 ** at exit. */
cu_str_t cuos_tmp_dir(void);

/*!A directory to load and store the state of an interactive application.
 * Before using this function, you must set the application name, e.g. by
 * calling \ref cu_set_application_name.  The session directory will typically
 * be <tt>$HOME/.\e appname</tt>.  If mode is zero and the directory does not
 * exist, \c NULL is returned.  If mode is non-zero and the directory does not
 * exist, the directory will be created with that mode (permissions). */
cu_str_t cuos_session_dir(mode_t mode);

/** @} */
CU_END_DECLARATIONS

#endif
