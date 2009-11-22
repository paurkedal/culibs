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

#include <cufo/stream.h>
#include <cuos/user_dirs.h>
#include <cu/util.h>
#include <cu/logging.h>
#include <cu/thread.h>
#include <cu/memory.h>
#include <cu/installdirs.h>

void cufoP_tag_init(void);
void cufoP_tagdefs_init(void);
void cufoP_attrdefs_init(void);
void cufoP_textstyle_default_init(void);
void cufoP_printf_init(void);
void cufoP_init_formats(void);

extern cu_installdirs_t cuconfP_installdirs;
struct cuos_pkg_user_dirs cufoP_user_dirs
    = CUOS_PKG_USER_DIRS_INITZ("cufo", "CUFO", cuconfP_installdirs);

cufo_stream_t cufo_stderr, cufo_stdout;
cufo_stream_t cufoP_stderr_bug;

cu_clos_def(_default_vlogf,
	    cu_prot(void, cu_log_facility_t facility, cu_sref_t sref,
		    char const *fmt, va_list va),
    ( cufo_stream_t fos; ))
{
    cu_clos_self(_default_vlogf);
    cufo_lock(self->fos);
    cufo_vlogf_at(self->fos, facility, sref, fmt, va);
    cufo_unlock(self->fos);
}

cu_clos_def(_default_vlogf_bug,
	    cu_prot(void, cu_log_facility_t facility, cu_sref_t sref,
		    char const *fmt, va_list va),
    ( cufo_stream_t fos; ))
{
    cu_clos_self(_default_vlogf_bug);
    /* Normally this function is only called once before the program aborts,
     * but
     *   1. It's possible that another error occurs while formatting a bug
     *      report, in which case we have a deadlock.
     *   2. It's also possible, but unlikely that two threads fail
     *      simultaneously, in which case it's unsafe to ignore the lock.
     * The ideal solution may be recursive locks.  For now, just revert to
     * fprintf. */
    if (pthread_mutex_trylock(&self->fos->mutex) == 0) {
	cufo_vlogf_at(self->fos, facility, sref, fmt, va);
	cufo_unlock(self->fos);
    } else {
	fprintf(stderr,
		"** Could not lock mutex for debug log.  This could be a "
		"deadlock or\n"
		"** a simultaneous failure in two threads.\n");
	vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
    }
}

cu_clop_def(_default_log_binder, cu_bool_t, cu_log_facility_t facility)
{
    if (cu_log_facility_severity(facility) == CU_LOG_FAILURE &&
	cu_log_facility_origin(facility) == CU_LOG_LOGIC) {
	_default_vlogf_bug_t *vlogf = cu_gnew(_default_vlogf_bug_t);
	vlogf->fos = cufoP_stderr_bug;
	facility->vlogf = _default_vlogf_bug_prep(vlogf);
    }
    else {
	_default_vlogf_t *vlogf = cu_gnew(_default_vlogf_t);
	vlogf->fos = cufo_stderr;
	facility->vlogf = _default_vlogf_prep(vlogf);
    }
    return cu_true;
}

static void
_cufo_uninit(void)
{
    cufo_close(cufo_stdout);
    cufo_close(cufo_stderr);
    cufo_close(cufoP_stderr_bug);
}

void
cufo_init(void)
{
    CU_RETURN_UNLESS_FIRST_CALL;
    cu_init();

    cufoP_tag_init();
    cufoP_tagdefs_init();
    cufoP_attrdefs_init();
    cufoP_printf_init();

    cufoP_textstyle_default_init();

    cufo_stdout = cufo_open_auto_fd(1, cu_false);
    cufo_stderr = cufo_open_auto_fd(2, cu_false);
    cufoP_stderr_bug = cufo_open_auto_fd(2, cu_false);
    cu_register_log_binder(cu_clop_ref(_default_log_binder));
    atexit(_cufo_uninit);

    cufoP_init_formats();
}
