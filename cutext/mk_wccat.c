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

#include <cutext/wccat.h>
#include <cu/wchar.h>
#include <cu/clos.h>
#include <argp.h>
#include <string.h>

#define CUTEXT_UNIPREP_C 1
#include "wccat.c"


/* == Parsing of Unicode Data == */

typedef struct _codepoint_info *_codepoint_info_t;
struct _codepoint_info
{
    cu_wint_t first_codepoint, last_codepoint;
    cutext_wccat_t wccat;
};

void
_codepoint_iter(char const *in_path, cu_clop(f, void, _codepoint_info_t))
{
#define BUF_SIZE 1024
    char s0[BUF_SIZE];
    FILE *in;
    struct _codepoint_info cpi;
    cu_bool_t have_first = cu_false;

    in = fopen(in_path, "r");
    if (!in) {
	cu_errf("Could not open %s for reading: %s", in_path, strerror(errno));
	exit(66); /* EX_NOINPUT */
    }

    cpi.wccat = CUTEXT_WCCAT_NONE;
    while (fgets(s0, BUF_SIZE, in)) {
	char *s1, *s2, *s3;
	size_t s1len;
	unsigned int codepoint;
	cutext_wccat_t wccat;

	s1 = strchr(s0, ';'); if (!s1) goto error; *s1++ = 0;
	s2 = strchr(s1, ';'); if (!s2) goto error; s1len = s2 - s1; *s2++ = 0;
	s3 = strchr(s2, ';'); if (!s3) goto error; *s3++ = 0;

	if (sscanf(s0, "%x", &codepoint) != 1) {
	    cu_errf("Can't parse codepoint \"%s\".", s0);
	    exit(65);
	}

	wccat = cutext_wccat_by_name(s2);
	if (wccat == CUTEXT_WCCAT_NONE) {
	    cu_errf("Unknown category %s", s2);
	    exit(65);
	}

	if (s1len > 7 && strcmp(s1 + s1len - 7, ", Last>") == 0) {
	    if (!have_first) {
		cu_errf("Range last line without preceding first.");
		exit(65);
	    }
	    if (wccat != cpi.wccat) {
		cu_errf("Expected same categories for first and last "
			"codepoints.");
		exit(65);
	    }
	    cpi.last_codepoint = codepoint;
	    have_first = cu_false;
	} else {
	    have_first = s1len > 8 && strcmp(s1 + s1len - 8, ", First>") == 0;
	    if (wccat == cpi.wccat && codepoint == cpi.last_codepoint + 1)
		++cpi.last_codepoint;
	    else {
		if (cpi.wccat != CUTEXT_WCCAT_NONE)
		    cu_call(f, &cpi);
		cpi.first_codepoint = codepoint;
		cpi.last_codepoint = codepoint;
		cpi.wccat = wccat;
	    }
	}
	continue;
error:
	fprintf(stderr, "Don't understand line \"%s\"\n", s0);
	exit(65); /* EX_DATAERR */
    }
#undef BUF_SIZE
}


/* == Make Switch == */

cu_clos_def(_make_switch_case, cu_prot(void, _codepoint_info_t cpi),
    (FILE *out;))
{
    cu_clos_self(_make_switch_case);
    int i;
    for (i = cpi->first_codepoint; i <= cpi->last_codepoint; ++i)
	fprintf(self->out, "case 0x%x: ", i);
    fprintf(self->out, "return %d;\n", cpi->wccat);
}

void
_make_switch(char const *in_path, char const *out_path)
{
    _make_switch_case_t cb;
    FILE *out;

    out = fopen(out_path, "w");
    if (!out) {
	cu_errf("Could not create %s: %s.", out_path, strerror(errno));
	exit(73); /* EX_CANTCREAT */
    }
    fputs("#include <cutext/wccat.h>\n\n"
	  "cutext_wccat_t\n"
	  "cutext_wchar_wccat(cu_wint_t ch)\n{\n"
	  "switch (ch) {\n",
	  out);
    cb.out = out;
    _codepoint_iter(in_path, _make_switch_case_prep(&cb));
    fprintf(out, "default: return %d;\n}\n}\n", CUTEXT_WCCAT_NONE);
}


/* == Main == */

struct _main_args
{
    int mode;
    char const *in_path;
    char const *out_path;
    char const *out_base;
};
static error_t
_main_parse_opt(int key, char *arg, struct argp_state *state)
{
    struct _main_args *args = state->input;
    switch (key) {
	    char const *s;
	case 'b':
	    args->out_base = arg;
	    return 0;
	case 'o':
	    args->out_path = arg;
	    return 0;
	case ARGP_KEY_ARGS:
	    if (state->argc - state->next != 2)
		argp_usage(state);

	    s = state->argv[state->next];
	    if (strcmp(s, "switch") == 0)
		args->mode = 1;
	    else if (strcmp(s, "blocks") == 0) {
		if (!args->out_base)
		    argp_usage(state);
		args->mode = 2;
	    }
	    else
		argp_usage(state);

	    args->in_path = state->argv[state->next + 1];
	    return 0;
	case ARGP_KEY_END:
	    if (!args->out_path)
		argp_usage(state);
	    return 0;
	default:
	    return ARGP_ERR_UNKNOWN;
    }
}
static struct argp_option _main_opts[] = {
    {NULL, 'o', "OUTPUT", 0,
	"Path to output file or base directory for multiple outputs."},
    {NULL, 'b', "BASE_DIR", 0,
	"Base directory required for \"blocks\" mode."},
    {NULL}
};
static struct argp main_argp = {
    _main_opts, _main_parse_opt, "MODE INPUT"
};

int
main(int argc, char **argv)
{
    struct _main_args args;

    cu_init();

    memset(&args, 0, sizeof(args));
    argp_parse(&main_argp, argc, argv, 0, NULL, &args);
    switch (args.mode) {
	case 1:
	    _make_switch(args.in_path, args.out_path);
	    break;
	case 2:
	    cu_bug_unfinished();
	    /* _make_blocks(args.in_path, args.out_path, args.out_base); */
	    break;
    }
    return 0;
}
