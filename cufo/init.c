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
#include <cu/util.h>
#include <cu/logging.h>
#include <cu/thread.h>
#include <cu/memory.h>

void cufoP_tag_init(void);
void cufoP_tagdefs_init(void);
void cufoP_attrdefs_init(void);
void cufoP_textstyle_default_init(void);
void cufoP_printf_init(void);

cufo_stream_t cufoP_stderr;
cufo_stream_t cufoP_stderr_bug;
cu_mutex_t cufoP_stderr_mutex = CU_MUTEX_INITIALISER;
cu_mutex_t cufoP_stderr_bug_mutex = CU_MUTEX_INITIALISER;

cu_clos_def(default_vlogf,
	    cu_prot(void, cu_log_facility_t facility, cu_sref_t sref,
		    char const *fmt, va_list va),
    ( cufo_stream_t fos;
      cu_mutex_t *fos_mutex; ))
{
    cu_clos_self(default_vlogf);
    cu_mutex_lock(self->fos_mutex);
    cufo_vlogf_at(self->fos, facility, sref, fmt, va);
    cu_mutex_unlock(self->fos_mutex);
}

cu_clos_def(default_vlogf_bug,
	    cu_prot(void, cu_log_facility_t facility, cu_sref_t sref,
		    char const *fmt, va_list va),
    ( cufo_stream_t fos;
      cu_mutex_t *fos_mutex; ))
{
    cu_clos_self(default_vlogf_bug);
    /* Normally this function is only called once before the program aborts,
     * but
     *   1. It's possible that another error occurs while formatting a bug
     *      report, in which case we have a deadlock.
     *   2. It's also possible, but unlikely that two threads fail
     *      simultaneously, in which case it's unsafe to ignore the lock.
     * The ideal solution may be recursive locks.  For now, just revert to
     * fprintf. */
    if (pthread_mutex_trylock(self->fos_mutex) == 0) {
	cufo_vlogf_at(self->fos, facility, sref, fmt, va);
	cu_mutex_unlock(self->fos_mutex);
    } else {
	fprintf(stderr,
		"** Could not lock mutex for debug log.  This could be a "
		"deadlock or\n"
		"** a simultaneous failure in two threads.\n");
	vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
    }
}

cu_clop_def(default_log_binder, cu_bool_t, cu_log_facility_t facility)
{
    if (facility->severity == CU_LOG_FAILURE &&
	facility->origin == CU_LOG_LOGIC) {
	default_vlogf_bug_t *vlogf = cu_gnew(default_vlogf_bug_t);
	vlogf->fos = cufoP_stderr_bug;
	vlogf->fos_mutex = &cufoP_stderr_bug_mutex;
	facility->vlogf = default_vlogf_bug_prep(vlogf);
    }
    else {
	default_vlogf_t *vlogf = cu_gnew(default_vlogf_t);
	vlogf->fos = cufoP_stderr;
	vlogf->fos_mutex = &cufoP_stderr_mutex;
	facility->vlogf = default_vlogf_prep(vlogf);
    }
    return cu_true;
}

void
print_wstring(cufo_stream_t fos, cufo_prispec_t spec, void *p)
{
    cufo_print_wstring(fos, p);
}

void
print_str(cufo_stream_t fos, cufo_prispec_t spec, void *p)
{
    cufo_print_str(fos, p);
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

    cufoP_stderr = cufo_open_text_fd("UTF-8", NULL, 2);
    cufoP_stderr_bug = cufo_open_text_fd("UTF-8", NULL, 2);
    cu_register_log_binder(cu_clop_ref(default_log_binder));

    cufo_register_ptr_format("wstring", print_wstring);
    cufo_register_ptr_format("str", print_str);
}
