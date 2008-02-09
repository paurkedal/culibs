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

#include <cu/thread.h>
#include <cu/fwd.h>
#include <cu/conf.h>
#include <cu/str.h>
#include <locale.h>
#include <string.h>
#include <gc/gc.h>
#include <gc/gc_pthread_redirects.h>


void cucon_init(void);
void cuooP_init(void);
void cuP_debug_init(void);
void cuP_memory_init(void);
void cuP_thread_init(void);
void cuP_tstate_init(void);
void cuP_diag_init(void);
void cuP_idr_init(void);
void cuP_str_init(void);
void cuP_wstring_init(void);

cu_bool_t cuP_locale_is_utf8;
pthread_mutexattr_t cuP_mutexattr;

char const *cuP_application_name = NULL;

cu_bool_t
cuconf_get_bool(char const *name)
{
    char const *v = getenv("name");
    return v && (strcmp(v, "true") == 0 || strcmp(v, "yes") == 0);
}

static void *
null_thread_proc(void *a)
{
    return a;
}

void
cu_set_application_name(char const *name)
{
    if (cuP_application_name && strcmp(cuP_application_name, name) != 0)
	cu_bugf("Tried to set application name to %s which conflicts with "
		"previous setting %s.", name, cuP_application_name);
    if (strchr(name, '/') != NULL)
	cu_bugf("The application name may not contain slashes.");
    cuP_application_name = name;
}

static void error_application_name()
{
    cu_bugf("This application uses a culibs function which requires the "
	    "application name to be set.  You will have to call "
	    "cu_set_application_name at startup, after cu_init.");
}

char const *
cu_application_name_cstr()
{
    if (!cuP_application_name)
	error_application_name();
    return cuP_application_name;
}

cu_str_t
cu_application_name_str()
{
    if (!cuP_application_name)
	error_application_name();
    return cu_str_new_cstr(cuP_application_name);
}

void
cu_init(void)
{
    char const *cstr;
    static int done_init = 0;
    if (done_init)
	return;
    done_init = 1;

    GC_init();

    setlocale(LC_ALL, "");
    cstr = setlocale(LC_CTYPE, NULL);
    cstr = strchr(cstr, '.');
    if (cstr == NULL || strncmp(cstr, ".utf8", 5))
	cuP_locale_is_utf8 = cu_false;
    else
	cuP_locale_is_utf8 = cu_true;

    pthread_mutexattr_init(&cuP_mutexattr);
#   if defined(CUCONF_DEBUG_SELF) && defined(CUCONF_MUTEX_ERRORCHECK)
	pthread_mutexattr_settype(&cuP_mutexattr,
				  PTHREAD_MUTEX_ERRORCHECK_NP);
	if (cuconf_get_bool("cu_debug_start_threading")) {
	    /* Mutex attribute checking may not be enabled before a
	     * thread is started, so when debugging, start a thread. */
	    pthread_t th;
	    int err;
	    err = GC_pthread_create(&th, NULL, null_thread_proc, NULL);
	    if (err != 0) {
		fprintf(stderr, "Could not spawn thread: %s\n", strerror(err));
		abort();
	    }
	    GC_pthread_join(th, NULL);
	}
#   endif

    /* Init the most basic stuff */
    cuP_thread_init();
    cuP_tstate_init();
    cuP_memory_init();
    cuP_diag_init();
    cuP_debug_init();

    /* Dynamic */
    cuooP_init();
    cuP_idr_init();
    cuP_str_init();
    cuP_wstring_init();

    /* Submodules */
    cucon_init();

    if ((cstr = getenv("CU_SEED48"))) {
	long s;
	if (strcmp(cstr, "time") == 0) {
	    s = time(NULL);
	    cu_verbf(0, "Seeding rng with %ld (from current time).", s);
	    srand48(s);
	}
	else if (cstr[0] == '0' && cstr[1] == 'x') {
	    unsigned short seed[3];
	    int i;
	    cstr += 2;
	    for (i = 0; i < 3; ++i) {
		if (sscanf(cstr, "%4hx", &seed[i]) != 1)
		    break;
		cstr += 4;
	    }
	    if (i == 3) {
		cu_verbf(0, "Seeding RNG with seed48(0x%04x%04x%04x).",
			seed[0], seed[1], seed[2]);
		seed48(seed);
	    }
	    else
		cu_errf("Value in $CU_SEED48 is incompatible for seed48.");
	}
	else if (sscanf(cstr, "%ld", &s) == 1) {
	    cu_verbf(0, "Seeding rng with %ld.", s);
	    srand48(s);
	}
	else
	    cu_warnf("Invalid value for CU_SEED48 environment variable.");
    }
}
