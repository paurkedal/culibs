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

#define CU_DEBUG 1

#include <cu/debug.h>
#include <cu/diag.h>
#include <cu/cstr.h>
#include <cu/memory.h>
#include <cucon/hset.h>
#include <cu/str.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <ctype.h>
#ifdef CUCONF_HAVE_LIBUNWIND
/* #  define UNW_LOCAL_ONLY 1 */
#  include <libunwind.h>
#endif

static cu_debug_flags_t debug_flags;

cu_debug_flags_t
cu_debug_enable(cu_debug_flags_t f)
{
    cu_debug_flags_t tmp = debug_flags;
    debug_flags |= f;
    return tmp;
}

cu_debug_flags_t
cu_debug_disable(cu_debug_flags_t f)
{
    cu_debug_flags_t tmp = debug_flags;
    debug_flags |= f;
    return tmp;
}

cu_debug_flags_t
cu_debug_select(cu_debug_flags_t f)
{
    cu_debug_flags_t tmp = debug_flags;
    debug_flags = f;
    return tmp;
}

cu_bool_fast_t
(cu_debug_is_enabled)(cu_debug_flags_t f)
{
    return debug_flags & f;
}

static int debug_indentation = 0;

void
(cu_debug_indent)()
{
    debug_indentation += 2;
}

void
(cu_debug_unindent)()
{
    debug_indentation -= 2;
}

void
cu_findent(FILE *fp, int i)
{
    while (i > 8) { fputc('\t', fp); i -= 8; }
    while (i > 0) { fputc(' ', fp); --i; }
}

void
cuP_debug_msg(const char *file, int line, const char *kind,
	      int fl, const char *fmt, ...)
{
    va_list va;
    if ((debug_flags & fl) != fl) return;
    fprintf(stderr, "%s:%d: %s: ", file, line, kind);
    cu_findent(stderr, debug_indentation);
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
}

void
cu_debug_bug_report_default(const char *file, int line, const char *fmt, ...)
{
    va_list va;
    fprintf(stderr,
	    "An unrecoverable error has occured.  Sorry.  This may help "
	    "the\nmaintainer (or you?) to fix the problem:\n%s:%d: ",
	    file, line);
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n\n");
}

void (*cuP_debug_bug_report)(const char *, int, const char *, ...)
    = cu_debug_bug_report_default;


#ifdef CUCONF_HAVE_LIBUNWIND
static int
_backtrace()
{
    int err;
    unw_context_t context;
    unw_cursor_t cursor;
    err = unw_getcontext(&context);
    if (err < 0) {
        fprintf(stderr, "Could not get unwind context.\n");
	return -1;
    }
    err = unw_init_local(&cursor, &context);
    if (err < 0) {
        fprintf(stderr, "Could not initialise unwind cursor: error %d.\n", err);
	return -1;
    }
    err = unw_step(&cursor); /* skip backtrace() */
    err = unw_step(&cursor); /* skip cu_debug_abort() */
    fprintf(stderr, "Stack trace (inner calls first):\n");
    while (err > 0) {
#define PROC_NAME_MAX 1024
        char proc_name[PROC_NAME_MAX] = "";
        unw_word_t proc_name_off;
        unw_word_t ip, sp;
        unw_proc_info_t info;
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        unw_get_reg(&cursor, UNW_REG_SP, &sp);
        err = unw_get_proc_info(&cursor, &info);
        if (err < 0) {
            fprintf(stderr, "Could not get procedure info: error %d.\n", err);
	    return -1;
        }
        unw_get_proc_name(&cursor, proc_name, PROC_NAME_MAX, &proc_name_off);
	if (strncmp(proc_name, "cu_bugf_", 8) != 0) {
#if 0
	    fprintf(stderr,
		    "FRAME ip = 0x%lx, sp = 0x%lx\n"
		    "IN PROCEDURE %s (offset 0x%x)\n"
		    "\tip_range = [0x%lx, 0x%lx)\n"
		    "\tlsda = 0x%lx, handler = 0x%lx, gp = 0x%lx\n"
		    "\tflags = 0x%lx, format = 0x%x\n\n",
		    (unsigned long)ip, (unsigned long)sp,
		    proc_name, proc_name_off,
		    (unsigned long)info.start_ip, (unsigned long)info.end_ip,
		    (unsigned long)info.lsda,
		    (unsigned long)info.handler,
		    (unsigned long)info.gp,
		    (unsigned long)info.flags,
		    info.format);
#else
	    if (strlen(proc_name))
		fprintf(stderr, "    %s + %#lx\n",
			proc_name, (unsigned long)proc_name_off);
	    else
		fprintf(stderr, "    %#lx\n", (unsigned long)proc_name_off);
#endif
	}
	if (strcmp(proc_name, "main") == 0)
	    break;
        err = unw_step(&cursor);
#undef PROC_NAME_MAX
    }
    if (err < 0) {
        fprintf(stderr, "Could not step up more frames: error %d.\n", err);
	return -1;
    }
    return 0;
}
#endif

void
cu_debug_abort()
{
    if (cu_debug_is_enabled(cu_debug_soft_exit_flag))
	exit(1);
#ifdef CUCONF_HAVE_LIBUNWIND
    else
	_backtrace();
#endif
    raise(SIGABRT);
    abort();
}

void
cu_debug_trap()
{
    raise(SIGTRAP);
}


static struct cucon_hset_s debug_keys;

cu_bool_t
(cu_debug_key)(char const *key)
{
    return cucon_hset_contains(&debug_keys, (char *)key);
}

void
cuP_dprintf(char const *file, int line, char const *key,
	       char const *fmt, ...)
{
    if (cu_debug_key(key)) {
	static pthread_mutex_t mutex = CU_MUTEX_INITIALISER;
	va_list va;
	cu_mutex_lock(&mutex);
	fprintf(stderr, "%s:%d: debug: ", file, line);
	va_start(va, fmt);
	cu_vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
	fflush(stderr);
	va_end(va);
	cu_mutex_unlock(&mutex);
    }
}

void
cu_err_invalid_arg(char const *function, int argnum, char const *argname,
		   char const *msg)
{
    fprintf(stderr, "%s arg #%d (%s): %s\n", function, argnum, argname, msg);
    abort();
}

#ifndef CU_NDEBUG
void
cu_debug_enable_key(char const *key)
{
    cucon_hset_insert(&debug_keys, (char *)key);
}

void
cu_debug_disable_key(char const *key)
{
    cucon_hset_erase(&debug_keys, (char *)key);
}
#endif

cu_bool_t
cuP_debug_facility_enabled(cu_log_facility_t facility)
{
    char const **keys = facility->keys;
    while (*keys) {
	if (strncmp(*keys, "dtag=", 5) == 0 && cu_debug_key(*keys + 5))
	    return cu_true;
	++keys;
    }
    return cu_false;
}

void
cuP_debug_init()
{
    char const *s;
    cucon_hset_init(&debug_keys,
	   (cu_clop(, cu_bool_t, void const *, void const *))cu_cstr_eq_clop,
	   (cu_clop(, cu_hash_t, void const *))cu_cstr_hash_clop);
    if ((s = getenv("CU_DEBUG_KEYS"))) {
	FILE *in = fopen(s, "r");
	if (in) {
	    char ch = fgetc(in);
	    for (;;) {
		while (isspace(ch))
		    ch = fgetc(in);
		if (ch == EOF)
		    break;
		cu_str_t key = cu_str_new();
		do {
		    cu_str_append_char(key, ch);
		    ch = fgetc(in);
		} while (ch != EOF && !isspace(ch));
		cucon_hset_insert(&debug_keys, (char *)cu_str_to_cstr(key));
	    }
	}
	else
	    cu_warnf("Could not open %s", s);
    }
}
