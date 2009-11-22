/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/list.h>
#include <cucon/rbset.h>
#include <cu/str.h>
#include <cu/debug.h>

#include <argp.h>
#include <string.h>


typedef enum {
    main_mode_undefined,
    main_mode_set_union,
    main_mode_set_isecn,
    main_mode_set_minus,
} main_mode_t;

static struct {
    char *name;
    main_mode_t mode;
} main_mode_name_map[] = {
    {"set_union", main_mode_set_union},
    {"set_isecn", main_mode_set_isecn},
    {"set_minus", main_mode_set_minus},
    {NULL}
};

struct main_args {
    main_mode_t mode;
    char *output;
    struct cucon_list inputs;
    char **positional;
};

static struct argp_option main_options[] = {
    /*{"input", 'i', "FILE", 0, "Add FILE to the input file stack"},*/
    {"output", 'o', "NEW_FILE", 0,
	"Print result to NEW_FILE instead of stdout"},
    {NULL}
};

static error_t
main_parsearg(int key, char *arg, struct argp_state *state)
{
    struct main_args *args = state->input;
    int i;
    switch (key) {
	case 'o':
	    args->output = arg;
	    break;
	case 'i':
	    cucon_list_append_ptr(&args->inputs, arg);
	    break;
	case ARGP_KEY_ARG:
	    if (args->mode)
		return ARGP_ERR_UNKNOWN;
	    for (i = 0; main_mode_name_map[i].name; ++i)
		if (!strcmp(arg, main_mode_name_map[i].name)) {
		    args->mode = main_mode_name_map[i].mode;
		    break;
		}
	    if (args->mode == main_mode_undefined)
		argp_usage(state);
	    break;
	case ARGP_KEY_ARGS:
	    args->positional = state->argv + state->next;
	    break;
	case ARGP_KEY_END:
	    if (args->mode == main_mode_undefined)
		argp_usage(state);
	    switch (args->mode) {
		case main_mode_set_isecn:
		case main_mode_set_minus:
		    if (state->argc - state->arg_num < 1)
			argp_usage(state);
		    break;
		default:
		    break;
	    }
	    break;
	default:
	    return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp main_argp = {
    main_options,
    main_parsearg,
    "set_union FILES...\n"
    "set_isecn FILES...\n"
    "set_minus FILES...\n",
    "A tool to manipulate sets of strings."
};

static cu_bool_t
listfile_load(FILE *fin, cu_clop(cb, cu_bool_t, cu_str_t elt))
{
    char ch;
    for (;;) {
	cu_str_t str;
	do ch = fgetc(fin); while (isspace(ch));
	if (ch == '#') {
	    do ch = fgetc(fin); while (ch != EOF && ch != '\n');
	    continue;
	}
	if (ch == EOF)
	    return cu_true;
	str = cu_str_new();
	while (ch != '\n') {
	    cu_str_append_char(str, ch);
	    ch = fgetc(fin);
	}
	if (!cu_call(cb, str))
	    return cu_false;
    }
}

cu_clos_def(set_insert,
	    cu_prot(cu_bool_t, cu_str_t str),
    ( cucon_rbset_t set; ))
{
    cu_clos_self(set_insert);
    cucon_rbset_insert(self->set, str);
    return cu_true;
}
static cucon_rbset_t
load_set_union(FILE *fin, cucon_rbset_t set)
{
    set_insert_t cb;
    cb.set = set;
    listfile_load(fin, set_insert_prep(&cb));
    return set;
}

cu_clos_def(set_intersect,
	    cu_prot(cu_bool_t, cu_str_t str),
    ( cucon_rbset_t ref_set;
      cucon_rbset_t set; ))
{
    cu_clos_self(set_intersect);
    if (cucon_rbset_find(self->ref_set, str))
	cucon_rbset_insert(self->set, str);
    return cu_true;
}
static cucon_rbset_t
load_set_isecn(FILE *fin, cucon_rbset_t set)
{
    set_intersect_t cb;
    cb.ref_set = set;
    cb.set = cucon_rbset_new_str_cmp();
    listfile_load(fin, set_intersect_prep(&cb));
    return cb.set;
}

cu_clos_def(set_erase,
	    cu_prot(cu_bool_t, cu_str_t str),
    ( cucon_rbset_t set; ))
{
    cu_clos_self(set_erase);
    cucon_rbset_erase(self->set, str);
    return cu_true;
}
static cucon_rbset_t
load_set_minus(FILE *fin, cucon_rbset_t set)
{
    set_erase_t cb;
    cb.set = set;
    listfile_load(fin, set_erase_prep(&cb));
    return set;
}

static const char *main_argv0;

static FILE *
open_in(char *path)
{
    FILE *fin = fopen(path, "r");
    if (!fin) {
	perror(main_argv0);
	exit(2);
    }
    return fin;
}

cu_clos_def(main_print_cb,
	    cu_prot(cu_bool_t, void *key),
    ( FILE *fout; ))
{
    cu_clos_self(main_print_cb);
    CU_DISCARD(fwrite(cu_str_charr(key), cu_str_size(key), 1, self->fout));
    fputc('\n', self->fout);
    return cu_true;
}

int
main(int argc, char **argv)
{
    cucon_rbset_t set;
    struct main_args args;
    main_print_cb_t print_cb;
    main_argv0 = argv[0];
    cucon_init();
    args.mode = main_mode_undefined;
    args.output = NULL;
    cucon_list_init(&args.inputs);
    args.positional = NULL;
    argp_parse(&main_argp, argc, argv, 0, NULL, &args);
    switch (args.mode) {
	    char **p;
	case main_mode_set_union:
	    set = cucon_rbset_new_str_cmp();
	    p = args.positional;
	    while (*p) {
		set = load_set_union(open_in(*p), set);
		++p;
	    }
	    break;
	case main_mode_set_isecn:
	    set = cucon_rbset_new_str_cmp();
	    p = args.positional;
	    cu_debug_assert(*p);
	    set = load_set_union(open_in(*p), set);
	    ++p;
	    while (*p) {
		set = load_set_isecn(open_in(*p), set);
		++p;
	    }
	    break;
	case main_mode_set_minus:
	    set = cucon_rbset_new_str_cmp();
	    p = args.positional;
	    cu_debug_assert(*p);
	    set = load_set_union(open_in(*p), set);
	    ++p;
	    while (*p) {
		set = load_set_minus(open_in(*p), set);
		++p;
	    }
	    break;
	default:
	    cu_bug_unreachable();
    }
    print_cb.fout = stdout;
    cucon_rbset_conj(set, main_print_cb_prep(&print_cb));
    return 0;
}
