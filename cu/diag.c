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
#include <cu/str.h>
#include <cu/sref.h>
#include <cucon/pmap.h>
#include <cutext/ucs4.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* Diagnostic Functions
 * -------------------- */

static int g_error_count;
static int g_warning_count;
static struct cucon_umap_s format_map;

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
    int i;
    for (i = 0; msg[i]; ++i) {
	if (msg[i] == '%') {
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
	    case 'l':
		switch (msg[++i]) {
		case 'c':
		    fprintf(file, "%lc", va_arg(va, int));
		    break;
		case 'd':
		    fprintf(file, "%ld", va_arg(va, long));
		    break;
		case 'x':
		    fprintf(file, "%lx", va_arg(va, long));
		    break;
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
#if 0 /* XXX cutext should register it */
	    case 'U': /* UCS-4 */
		switch (msg[++i]) {
#define BUF_SIZE 6
		    size_t buf_cnt;
		    char buf[BUF_SIZE];
		    char *buf_end;
		    cutext_ucs4char_t ch;
		    cutext_ucs4char_t *ucs4cstr;
		    int err;
		case 'c':
		    ch = va_arg(va, cutext_ucs4char_t);
		    buf_cnt = BUF_SIZE;
		    buf_end = buf;
		    err = cutext_ucs4char_to_charr(ch, &buf_end, &buf_cnt);
		    if (err != 0)
			fprintf(file, "#[iconv_error: %s]", strerror(err));
		    else
			fwrite(buf, 1, buf_end - buf, file);
		    break;
		case 's':
		    ucs4cstr = va_arg(va, cutext_ucs4char_t *);
		    while (*ucs4cstr) {
			buf_cnt = BUF_SIZE;
			buf_end = buf;
			err = cutext_ucs4char_to_charr(*ucs4cstr,
						    &buf_end, &buf_cnt);
			if (err != 0)
			    fprintf(file, "#[iconv_error: %s]", strerror(err));
			else
			    fwrite(buf, 1, buf_end - buf, file);
			++ucs4cstr;
		    }
		    break;
		default:
		    fputs("#[bad formatting key]", file);
		    break;
#undef BUF_SIZE
		}
		break;
#endif
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

static void
print_sref_also(cu_sref_t sref, char const *errorkind, FILE *out)
{
    if (!sref)
	return;
    while ((sref = cu_sref_chain_tail(sref))) {
	cu_sref_fprint(sref, out);
	fputs(errorkind, out);
	fputs(": As above.\n", out);
    }
}

void
cu_verrf_at(cu_sref_t srf, char const *msg, va_list va)
{
    ++g_error_count;
    cu_sref_fprint(srf, stderr);
    fputs(": ", stderr);
    cu_vfprintf(stderr, msg, va);
    fputc('\n', stderr);
    print_sref_also(srf, "", stderr);
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
    ++g_error_count;
    fputs("error: ", stderr);
    cu_vfprintf(stderr, msg, va);
    fputc('\n', stderr);
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
    ++g_warning_count;
    cu_sref_fprint(srf, stderr);
    fputs(": warning: ", stderr);
    cu_vfprintf(stderr, msg, va);
    fputc('\n', stderr);
    print_sref_also(srf, "warning: ", stderr);
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
    ++g_warning_count;
    fputs("warning: ", stderr);
    cu_vfprintf(stderr, msg, va);
    fputc('\n', stderr);
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
    ++g_warning_count;
    cu_sref_fprint(srf, stderr);
    fputs(": internal error: ", stderr);
    va_start(va, msg);
    cu_vfprintf(stderr, msg, va);
    va_end(va);
    fputc('\n', stderr);
    print_sref_also(srf, "internal error: ", stderr);
    abort();
}

void
cu_bugf_fl(char const *file, int line, char const *msg, ...)
{
    va_list va;
    ++g_warning_count;
    fprintf(stderr, "%s:%d: internal error: ", file, line);
    va_start(va, msg);
    cu_vfprintf(stderr, msg, va);
    va_end(va);
    fputc('\n', stderr);
    abort();
}

void
cu_bugf(char const* msg, ...)
{
    va_list va;
    ++g_warning_count;
    fputs("internal error: ", stderr);
    va_start(va, msg);
    cu_vfprintf(stderr, msg, va);
    va_end(va);
    fputc('\n', stderr);
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
    cu_sref_fprint(srf, stderr);
    fputs(": info: ", stderr);
    va_start(va, msg);
    cu_vfprintf(stderr, msg, va);
    va_end(va);
    fputc('\n', stderr);
    print_sref_also(srf, "info: ", stderr);
}

void
cu_verbf(int level, char const *msg, ...)
{
    va_list va;
    if (level > cuP_verbosity)
	return;
    fputs("info: ", stderr);
    va_start(va, msg);
    cu_vfprintf(stderr, msg, va);
    va_end(va);
    fputc('\n', stderr);
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
    char *s;
    cucon_umap_cct(&format_map);
    if ((s = getenv("CU_VERBOSITY"))) {
	int i = atoi(s);
	if (i > 0)
	    cu_set_verbosity(i);
    }
}
