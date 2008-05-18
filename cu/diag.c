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

#include <cu/diag.h>
#include <cu/logging.h>
#include <cu/str.h>
#include <cu/sref.h>
#include <cucon/pmap.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* Diagnostic Functions
 * -------------------- */

static int g_error_count;
static int g_warning_count;
static struct cucon_umap_s format_map;

#if 0  /* Locking shall be done by vlogf implementation! */
static cu_mutex_t g_log_mutex = CU_MUTEX_INITIALISER;
#  define lock_logs(msg, va) cu_vlogf_mutex_lock(&g_log_mutex, msg, va)
#  define unlock_logs() cu_mutex_unlock(&g_log_mutex)
#else
#  define lock_logs(msg, va) ((void)0)
#  define unlock_logs() ((void)0)
#endif

enum {FI_VERB, FI_WARN, FI_ERR, FI_DEBUG, FI_BUG};
static struct cu_log_facility_s g_log_facility_arr[] = {
    {CU_LOG_INFO, CU_LOG_USER},
    {CU_LOG_WARNING, CU_LOG_USER},
    {CU_LOG_ERROR, CU_LOG_USER},
    {CU_LOG_INFO, CU_LOG_LOGIC},
    {CU_LOG_FAILURE, CU_LOG_LOGIC},
};

void
cu_diag_define_format_key(char key, cu_diag_format_fn_t fn)
{
    cu_diag_format_fn_t *slot;
    if (!cucon_umap_insert_mem(&format_map, key,
			       sizeof(cu_diag_format_fn_t), &slot)) {
	/* cu_warnf("Formatting key %c is already defined.", key); */
    }
    *slot = fn;
}

void
cu_vfprintf(FILE *file, char const* msg, va_list va)
{
    char fmt_buf[8] = "%";
    int i;
    for (i = 0; msg[i]; ++i) {
	if (msg[i] == '%') {
	    char *fmt_ptr = fmt_buf + 1;
	    ++i;
	    switch (msg[i]) {
		cu_diag_format_fn_t fmt_fn;
		cu_str_t str;
	    case '&': {
		void (*print)(void *obj, FILE *file)
		    = va_arg(va, void (*)(void *, FILE *)); 
		void *obj = va_arg(va, void *);
		print(obj, file);
		break;
	    }
	    case 'h': case 'l':
		if (msg[i + 1] == msg[i])
		    *fmt_ptr++ = msg[i++];
		/* fall though */
	    case 'j': case 'z': case 't':
		*fmt_ptr++ = msg[i];
		switch (msg[++i]) {
		case 'd': case 'i': case 'u': case 'o': case 'x':
		    *fmt_ptr++ = msg[i];
		    *fmt_ptr++ = 0;
		    fprintf(file, fmt_buf, va_arg(va, long));
		    break;
		case 'c':
		    if (msg[i - 1] == 'l') {
			fprintf(file, "%lc", va_arg(va, int));
			break;
		    }
		    /* fall through */
		default:
		    fputs("#[bad_formatting_key]", file);
		    break;
		}
		break;
	    case 'c':
		fputc(va_arg(va, int), file);
		break;
	    case 'd':
		fprintf(file, "%d", va_arg(va, int));
		break;
	    case 'x':
		fprintf(file, "%x", va_arg(va, int));
		break;
	    case 's':
		fputs(va_arg(va, char const*), file);
		break;
	    case 'S':
		str = va_arg(va, cu_str_t);
		if (!str)
		    fputs("(null)", file);
		else
		    fwrite(cu_str_charr(str), cu_str_size(str), 1, file);
		break;
	    case 'p':
		fprintf(file, "%p", va_arg(va, void*));
		break;
	    default:
		fmt_fn = cucon_umap_find_ptr(&format_map, msg[i]);
		if (fmt_fn) {
		    cu_str_t fmt = cu_str_new_charr(&msg[i], 1);
		    cu_call(fmt_fn, fmt, cu_va_ref_of_va_list(va), file);
		}
		else
		    fputs("#[bad formatting key]", file);
		break;
	    }
	}
	else
	    fputc(msg[i], file);
    }
}

void
cu_verrf_at(cu_sref_t srf, char const *msg, va_list va)
{
    lock_logs(msg, va);
    ++g_error_count;
    cu_vlogf_at(&g_log_facility_arr[FI_ERR], srf, msg, va);
    unlock_logs();
}

void
cu_errf_at(cu_sref_t srf, char const *msg, ...)
{
    va_list va;
    va_start(va, msg);
    cu_verrf_at(srf, msg, va);
    va_end(va);
}

void
cu_verrf(char const *msg, va_list va)
{
    lock_logs(msg, va);
    ++g_error_count;
    cu_vlogf(&g_log_facility_arr[FI_ERR], msg, va);
    unlock_logs();
}

void
cu_errf(char const *msg, ...)
{
    va_list va;
    va_start(va, msg);
    cu_verrf(msg, va);
    va_end(va);
}

void
cu_vwarnf_at(cu_sref_t srf, char const *msg, va_list va)
{
    lock_logs(msg, va);
    ++g_warning_count;
    cu_vlogf_at(&g_log_facility_arr[FI_WARN], srf, msg, va);
    unlock_logs();
}

void
cu_warnf_at(cu_sref_t srf, char const *msg, ...)
{
    va_list va;
    va_start(va, msg);
    cu_vwarnf_at(srf, msg, va);
    va_end(va);
}

void
cu_vwarnf(char const* msg, va_list va)
{
    lock_logs(msg, va);
    ++g_warning_count;
    cu_vlogf(&g_log_facility_arr[FI_WARN], msg, va);
    unlock_logs();
}

void
cu_warnf(char const* msg, ...)
{
    va_list va;
    va_start(va, msg);
    cu_vwarnf(msg, va);
    va_end(va);
}

void
cu_bugf_at(cu_sref_t srf, char const* msg, ...)
{
    va_list va;
    va_start(va, msg);
    lock_logs(msg, va);
    cu_vlogf_at(&g_log_facility_arr[FI_BUG], srf, msg, va);
    unlock_logs();
    va_end(va);
    abort();
}

void
cu_bugf_fl(char const *file, int line, char const *msg, ...)
{
    va_list va;
    cu_sref_t srf = cu_sref_new_cstr(file, line, -1);
    va_start(va, msg);
    cu_vlogf_at(&g_log_facility_arr[FI_BUG], srf, msg, va);
    va_end(va);
    abort();
}

void
cu_bugf(char const* msg, ...)
{
    va_list va;
    va_start(va, msg);
    lock_logs(msg, va);
    cu_vlogf(&g_log_facility_arr[FI_BUG], msg, va);
    unlock_logs();
    va_end(va);
    abort();
}

int cuP_verbosity = 10;

void
cu_set_verbosity(int level)
{
    cuP_verbosity = level;
}

void
cu_verbf_at(int level, cu_sref_t srf, char const *msg, ...)
{
    va_list va;
    if (level > cuP_verbosity)
	return;
    va_start(va, msg);
    lock_logs(msg, va);
    cu_vlogf_at(&g_log_facility_arr[FI_VERB], srf, msg, va);
    unlock_logs();
    va_end(va);
}

void
cu_verbf(int level, char const *msg, ...)
{
    va_list va;
    if (level > cuP_verbosity)
	return;
    va_start(va, msg);
    lock_logs(msg, va);
    cu_vlogf(&g_log_facility_arr[FI_VERB], msg, va);
    unlock_logs();
    va_end(va);
}

void
cu_fprintf(FILE *file, char const *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    cu_vfprintf(file, fmt, va);
    va_end(va);
}

void
cuP_diag_init(void)
{
    int i;
    char *s;

    for (i = 0; i < sizeof(g_log_facility_arr)/sizeof(g_log_facility_arr[0]);
	 ++i)
	cu_register_permanent_log(&g_log_facility_arr[i]);

    cucon_umap_init(&format_map);
    if ((s = getenv("CU_VERBOSITY"))) {
	int i = atoi(s);
	if (i > 0)
	    cu_set_verbosity(i);
    }
}
