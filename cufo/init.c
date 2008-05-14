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
cu_mutex_t cufoP_stderr_mutex = CU_MUTEX_INITIALISER;

cu_clos_def(defaul_vlogf,
	    cu_prot(void, cu_log_facility_t facility, cu_sref_t sref,
		    char const *fmt, va_list va),
    ( cufo_stream_t fos;
      cu_mutex_t *fos_mutex; ))
{
    cu_clos_self(defaul_vlogf);
    cu_mutex_lock(self->fos_mutex);
    cufo_vlogf_at(self->fos, facility, sref, fmt, va);
    cu_mutex_unlock(self->fos_mutex);
}

cu_clop_def(default_log_binder, cu_bool_t, cu_log_facility_t facility)
{
    defaul_vlogf_t *vlogf = cu_gnew(defaul_vlogf_t);
    vlogf->fos = cufoP_stderr;
    vlogf->fos_mutex = &cufoP_stderr_mutex;
    facility->vlogf = defaul_vlogf_prep(vlogf);
    return cu_true;
}

void
print_wstring(cufo_stream_t fos, cufo_prispec_t spec, void *p)
{
    cufo_print_wstring(fos, p);
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
    cu_register_log_binder(cu_clop_ref(default_log_binder));

    cufo_register_ptr_format("wstring", print_wstring);
}
