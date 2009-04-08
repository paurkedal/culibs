%{

#include <cu/idr.h>
#include <cu/sref.h>
#include <cu/str.h>
#include <cu/clos.h>
#include <cuos/file.h>
#include <cuos/fs.h>
#include <cucon/pmap.h>
#include <cucon/list.h>
#include <cucon/bitvect.h>
#include <cuex/otab.h>
#include <stdio.h>
#include <ctype.h>
#include <argp.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct ot_intrange_s *ot_intrange_t;
typedef struct ot_state_s *ot_state_t;
typedef union ot_value_u *ot_value_t;

struct ot_intrange_s
{
    cuex_meta_t min;
    cuex_meta_t maxp1;
};

struct ot_state_s
{
    cuex_otab_t otab;
    struct cu_sref_s sref;
    FILE *in;
    cu_bool_t is_extern;
    int error_cnt;
    cuex_otab_range_t current_range;
    cucon_list_t import_paths;
};

int ot_parse_file(cuex_otab_t tab, cu_str_t path, cucon_list_t import_paths);

void ot_state_import(ot_state_t state, cu_sref_t sref, cu_str_t path);

cuex_otab_range_t
ot_state_defrange(ot_state_t state, cu_idr_t idr, cu_sref_t sref,
		  cu_idr_t super, ot_intrange_t intrange);
void ot_state_defopr(ot_state_t state, cu_idr_t idr, cu_sref_t sref,
		     int r, cu_bool_t has_ctor, cu_idr_t range_idr);
void ot_state_defprop(ot_state_t state, cu_idr_t idr, cu_sref_t sref,
		      unsigned int bits, cu_str_t type_str,
		      cu_bool_t is_implicit);
void ot_state_reserve(ot_state_t state, cu_sref_t sref, cu_idr_t super_idr,
		      ot_intrange_t r);
void ot_state_defoption(ot_state_t state, cu_sref_t sref,
			cu_idr_t idr, cu_str_t str);

static int yylex(ot_value_t val_out, cu_sref_t loc_out, ot_state_t state);
static void yyerror(cu_sref_t loc, ot_state_t state, char const *msg);

#define YYLTYPE struct cu_sref_s
#define YYLLOC_DEFAULT(lhs, rhs, n)					\
    do {								\
	if (n)								\
	    cu_sref_cct_sref_range(&(lhs),				\
		&YYRHSLOC(rhs, 1), &YYRHSLOC(rhs, n));			\
	else								\
	    cu_sref_cct_sref_last(&(lhs), &YYRHSLOC(rhs, 0));		\
    } while (0)

%}

%locations
%pure-parser
%parse-param {ot_state_t state}
%lex-param {ot_state_t state}
%initial-action { cu_sref_cct_copy(&@$, &state->sref); }

%union ot_value_u {
    int i;
    cu_idr_t idr;
    ot_intrange_t intrange;
    cu_str_t str;
    struct { int r; cu_bool_t has_ctor; } opr;
}

%token ERROR

%token RANGE "range"
%token RESERVE "reserve"
%token ATTR "attr"
%token <opr> OPERATOR
%token CTO ".."
%token LTO ".!"
%token RTO "!."
%token BITS "bits"
%token IMPLICIT "implicit"
%token IMPORT "import"
%token PROVIDE "provide"
%token OPTION "option"

%token <idr> IDR
%token <i> INT
%token <str> STRING

%type <i> int_expr times_expr
%type <intrange> intrange
%type <str> string_opt

%%

grammar:
    PROVIDE STRING { if (!state->is_extern) state->otab->name = $2; }
    prestmt_seq stmt_seq

prestmt_seq:
    /* empty */
  | prestmt_seq prestmt
  ;

prestmt:
    IMPORT STRING { ot_state_import(state, &@$, $2); }
  ;

stmt_seq:
    /* empty */
  | stmt_seq stmt
  ;

stmt:
    RANGE IDR '=' IDR '[' intrange ']'
    { state->current_range = ot_state_defrange(state, $2, &@$, $4, $6); }
    prop_seq
    { state->current_range = NULL; }
  | RESERVE IDR '[' intrange ']'
    { ot_state_reserve(state, &@$, $2, $4); }
  | OPERATOR IDR ':' IDR
    { ot_state_defopr(state, $2, &@$, $1.r, $1.has_ctor, $4); }
  | OPTION IDR '=' STRING
    { ot_state_defoption(state, &@$, $2, $4); }
  ;

prop_seq:
    /* empty */
  | prop_seq ATTR IDR ':' INT BITS string_opt
    { ot_state_defprop(state, $3, &@$, $5, $7, cu_false); }
  | prop_seq IMPLICIT ATTR IDR ':' INT BITS string_opt
    { ot_state_defprop(state, $4, &@$, $6, $8, cu_true); }
  ;

string_opt: STRING | /* empty */ { $$ = NULL; }

int_expr:
    times_expr
  | int_expr '+' times_expr { $$ = $1 + $3; }
  | int_expr '-' times_expr { $$ = $1 - $3; }
  | '-' times_expr { $$ = -$2; }
  ;

times_expr:
    INT
  | times_expr '*' INT { $$ = $1 * $3; }
  | times_expr '/' INT { $$ = $1 / $3; }
  ;

intrange:
    int_expr CTO int_expr
    {
	$$ = cu_gnew(struct ot_intrange_s);
	$$->min = $1;
	$$->maxp1 = $3 + 1;
    }
  | int_expr LTO int_expr
    {
	$$ = cu_gnew(struct ot_intrange_s);
	$$->min = $1;
	$$->maxp1 = $3;
    }
  | int_expr RTO int_expr
    {
	$$ = cu_gnew(struct ot_intrange_s);
	$$->min = $1 + 1;
	$$->maxp1 = $3;
    }
  ;
%%

cuex_otab_range_t
ot_state_defrange(ot_state_t state, cu_idr_t idr, cu_sref_t sref,
		  cu_idr_t super_idr, ot_intrange_t intrange)
{
    cuex_otab_def_t super;

    cu_debug_assert(!state->current_range);
    super = cuex_otab_lookup(state->otab, super_idr);
    if (!super) {
	++state->error_cnt;
	cu_errf_at(sref, "Undefined range %s.", cu_idr_to_cstr(super_idr));
	return NULL;
    }
    else if (cuex_otab_def_kind(super) != cuex_otab_range_kind) {
	++state->error_cnt;
	cu_errf_at(sref, "%s is not a range.", cu_idr_to_cstr(super_idr));
	return NULL;
    }
    else
	return cuex_otab_defrange(state->otab, idr, cu_sref_new_copy(sref),
				  cuex_otab_range_from_def(super),
				  intrange->min, intrange->maxp1);
}

void
ot_state_defopr(ot_state_t state, cu_idr_t idr, cu_sref_t sref,
		int r, cu_bool_t has_ctor, cu_idr_t super_idr)
{
    cuex_otab_def_t super;

    if (state->is_extern)
	return;

    cu_debug_assert(!state->current_range);
    super = cuex_otab_lookup(state->otab, super_idr);
    if (!super) {
	++state->error_cnt;
	cu_errf_at(sref, "Undefined range %s.", cu_idr_to_cstr(super_idr));
    }
    else if (cuex_otab_def_kind(super) != cuex_otab_range_kind) {
	++state->error_cnt;
	cu_errf_at(sref, "%s is not a range.", cu_idr_to_cstr(super_idr));
    }
    else {
	cuex_otab_opr_t opr;
	opr = cuex_otab_defopr(state->otab, idr, cu_sref_new_copy(sref),
			       cuex_otab_range_from_def(super), r);
	if (has_ctor)
	    cuex_otab_opr_give_ctor(opr);
    }
}

void
ot_state_defprop(ot_state_t state, cu_idr_t idr, cu_sref_t sref,
		 unsigned int width, cu_str_t type_str, cu_bool_t is_implicit)
{
    cuex_otab_range_t range = state->current_range;
    if (!range)
	return;
    cuex_otab_defprop(state->otab, idr, cu_sref_new_copy(sref), range, width,
		      type_str? cu_str_to_cstr(type_str) : "unsigned int",
		      is_implicit);
}

void
ot_state_reserve(ot_state_t state, cu_sref_t sref, cu_idr_t super_idr,
		 ot_intrange_t intrange)
{
    cuex_otab_def_t super;

    cu_debug_assert(!state->current_range);
    super = cuex_otab_lookup(state->otab, super_idr);
    if (!super) {
	++state->error_cnt;
	cu_errf_at(sref, "Undefined range %s.", cu_idr_to_cstr(super_idr));
    }
    else if (cuex_otab_def_kind(super) != cuex_otab_range_kind) {
	++state->error_cnt;
	cu_errf_at(sref, "%s is not a range.", cu_idr_to_cstr(super_idr));
    }
    else
	cuex_otab_reserve(state->otab, cu_sref_new_copy(sref),
			  cuex_otab_range_from_def(super),
			  intrange->min, intrange->maxp1, state->is_extern);
}

static void
yyerror(cu_sref_t loc, ot_state_t state, char const *msg)
{
    ++state->error_cnt;
    cu_errf_at(loc, "%s", msg);
}

static struct cucon_pmap_s ot_keyword_map;

static int
yylex(ot_value_t val_out, cu_sref_t loc_out, ot_state_t state)
{
    int token = ERROR;
    FILE *in = state->in;
    char ch;
    cu_sref_t sref = &state->sref;

    ch = fgetc(in);
    for (;;) {
	while (isspace(ch)) {
	    cu_sref_advance_char(sref, ch);
	    ch = fgetc(in);
	}
	if (ch != '#')
	    break;
	do ch = fgetc(in); while (ch != '\n' && ch != EOF);
    }
    cu_sref_cct_copy(loc_out, sref);

    if (isalpha(ch) || ch == '_') {
	cu_str_t str = cu_str_new();
	do {
	    cu_str_append_char(str, ch);
	    cu_sref_advance_char(sref, ch);
	    ch = fgetc(in);
	} while (isalnum(ch) || ch == '_');
	if (cu_str_cmp_charr(str, "o", 1) == 0 && ch == '/') {
	    int len;
	    if (fscanf(in, "%d%n", &val_out->opr.r, &len) > 0) {
		cu_sref_advance_columns(sref, len);
		token = OPERATOR;
		ch = fgetc(in);
		if (ch == 'c')
		    val_out->opr.has_ctor = cu_true;
		else {
		    val_out->opr.has_ctor = cu_false;
		    ungetc(ch, in);
		    if (!isspace(ch)) {
			++state->error_cnt;
			cu_errf_at(sref, "Invalid operator flag.");
			token = ERROR;
		    }
		}
	    } else {
		ch = fgetc(in);
		if (ch == 'r') {
		    val_out->opr.has_ctor = cu_false;
		    val_out->opr.r = -1;
		    token = OPERATOR;
		} else {
		    ungetc(ch, in);
		    token = ERROR;
		}
	    }
	}
	else {
	    int *p;
	    ungetc(ch, in);
	    val_out->idr = cu_str_to_idr(str);
	    p = cucon_pmap_find_mem(&ot_keyword_map, val_out->idr);
	    token = p? *p : IDR;
	}
    }
    else if (isdigit(ch)) {
	int len;
	ungetc(ch, in);
	fscanf(in, "%i%n", &val_out->i, &len);
	cu_sref_advance_columns(sref, len);
	token = INT;
    }
    else switch (ch) {
	case '"': {
	    cu_str_t str = cu_str_new();
	    token = STRING;
	    cu_sref_next(sref);
	    for (;;) {
		char ch_out;
		cu_sref_advance_char(sref, ch);
		ch = ch_out = fgetc(in);
		if (ch == EOF) {
		    ++state->error_cnt;
		    cu_errf_at(loc_out, "End of file while parsing string.");
		    token = ERROR;
		    break;
		}
		else if (ch == '"') {
		    do {
			cu_sref_advance_char(sref, ch);
			ch = fgetc(in);
		    } while (isspace(ch));
		    if (ch != '"') {
			ungetc(ch, in);
			break;
		    }
		    continue;
		}
		else if (ch == '\\') {
		    cu_sref_advance_char(sref, ch);
		    ch = ch_out = fgetc(in);
		    switch (ch) {
			case 'n': ch_out = '\n'; break;
			case 't': ch_out = '\t'; break;
			case '"': case '\\': break;
			default:
			    ++state->error_cnt;
			    cu_errf_at(sref, "Invalid character escape.");
			    break;
		    }
		}
		cu_str_append_char(str, ch_out);
	    }
	    cu_sref_next(sref);
	    val_out->str = str;
	    break;
	}
	case ':': case '=':
	case '[': case ']':
	case '(': case ')':
	case '+': case '-': case '*': case '/':
	    cu_sref_next(sref);
	    token = ch;
	    break;
	case '.':
	    cu_sref_next(sref);
	    ch = fgetc(in);
	    cu_sref_advance_char(sref, ch);
	    switch (ch) {
		case '.':
		    token = CTO;
		    break;
		case '!':
		    token = LTO;
		    break;
		default:
		    break;
	    }
	    break;
	case '!':
	    cu_sref_next(sref);
	    ch = fgetc(in);
	    cu_sref_advance_char(sref, ch);
	    switch (ch) {
		case '.':
		    token = RTO;
		    break;
		default:
		    break;
	    }
	    break;
	case EOF:
	    token = 0;
	    break;
    }
    cu_sref_set_sref_last(loc_out, sref);
    if (token == ERROR && ch != EOF) {
	++state->error_cnt;
	cu_errf_at(sref, "Unexpected character %c.", ch);
    }
    return token;
}

void
ot_error_cb(cuex_otab_t tab, cu_sref_t sref, char const *msg, ...)
{
    va_list va;
    va_start(va, msg);
    cu_verrf_at(sref, msg, va);
    va_end(va);
}

int
ot_parse_file(cuex_otab_t tab, cu_str_t path, cucon_list_t import_paths)
{
    struct ot_state_s state;
    state.import_paths = import_paths;
    state.error_cnt = 0;
    state.otab = tab;
    state.in = fopen(cu_str_to_cstr(path), "r");
    state.is_extern = cuex_otab_is_extern(tab);
    if (!state.in) {
	cu_errf("Could not open %s: %s", cu_str_to_cstr(path),
		strerror(errno));
	return 1;
    }
    cu_sref_cct(&state.sref, path, 1, 0);
    state.current_range = NULL;
    yyparse(&state);
    return state.error_cnt;
}

void
ot_state_import(ot_state_t state, cu_sref_t sref, cu_str_t rel_path)
{
    cu_str_t path;
    cucon_listnode_t incl_node;
    cu_bool_t is_extern;
    if (cu_str_size(rel_path) == 0) {
	++state->error_cnt;
	cu_errf_at(sref, "Zero path in include.");
	return;
    }
    if (cu_str_at(rel_path, 0) == '/')
	path = rel_path;
    else {
	cu_bool_t found = cu_false;
	for (incl_node = cucon_list_begin(state->import_paths);
	     incl_node != cucon_list_end(state->import_paths);
	     incl_node = cucon_listnode_next(incl_node)) {
	    struct stat st;
	    cu_str_t incl_path = cucon_listnode_ptr(incl_node);
	    path = cu_str_new_copy(incl_path);
	    cu_str_append_char(path, '/');
	    cu_str_append_str(path, rel_path);
	    if (stat(cu_str_to_cstr(path), &st) != -1) {
		found = cu_true;
		break;
	    }
	}
	if (!found) {
	    ++state->error_cnt;
	    cu_errf_at(sref, "Could not find %s in include path.",
		       cu_str_to_cstr(rel_path));
	    return;
	}
    }
    is_extern = cuex_otab_is_extern(state->otab);
    cuex_otab_set_extern(state->otab, cu_true);
    state->error_cnt += ot_parse_file(state->otab, path,
				      state->import_paths);
    cuex_otab_set_extern(state->otab, is_extern);
}

static cu_idr_t idr_h_prologue;
static cu_idr_t idr_h_epilogue;
static cu_idr_t idr_c_prologue;
static cu_idr_t idr_c_epilogue;

void
ot_state_defoption(ot_state_t state, cu_sref_t sref,
		   cu_idr_t idr, cu_str_t str)
{
    if (idr == idr_h_prologue) {
	if (!state->is_extern)
	    cuex_otab_set_h_prologue(state->otab, str);
    }
    else if (idr == idr_h_epilogue) {
	if (!state->is_extern)
	    cuex_otab_set_h_epilogue(state->otab, str);
    }
    else if (idr == idr_c_prologue) {
	if (!state->is_extern)
	    cuex_otab_set_c_prologue(state->otab, str);
    }
    else if (idr == idr_c_epilogue) {
	if (!state->is_extern)
	    cuex_otab_set_c_epilogue(state->otab, str);
    }
    else {
	++state->error_cnt;
	cu_errf_at(sref, "Unknown option %s.", cu_idr_to_cstr(idr));
    }
}

void
ot_init()
{
    int *token_ptr;
    cucon_pmap_cct(&ot_keyword_map);
#define K(str, token)							\
	cucon_pmap_insert_mem(&ot_keyword_map, cu_idr_by_cstr(str),	\
			      sizeof(int), &token_ptr);			\
	*token_ptr = (token)
    K("attr", ATTR);
    K("bit", BITS);
    K("bits", BITS);
    K("option", OPTION);
    K("implicit", IMPLICIT);
    K("import", IMPORT);
    K("provide", PROVIDE);
    K("range", RANGE);
    K("reserve", RESERVE);
#undef K
    idr_h_prologue = cu_idr_by_cstr("h_prologue");
    idr_h_epilogue = cu_idr_by_cstr("h_epilogue");
    idr_c_prologue = cu_idr_by_cstr("c_prologue");
    idr_c_epilogue = cu_idr_by_cstr("c_epilogue");
}


/* Main
 * ==== */

struct main_opts_s
{
    char const *path;
    char const *out_base;
    cu_bool_t update;
    struct cucon_list_s import_paths;
};

error_t
main_parseopt(int key, char *arg, struct argp_state *state)
{
    struct main_opts_s *opts = state->input;
    switch (key) {
	case ARGP_KEY_ARG:
	    if (opts->path)
		return ARGP_ERR_UNKNOWN;
	    opts->path = arg;
	    break;
	case ARGP_KEY_END:
	    if (!opts->path)
		argp_error(state, "Missing input path.");
	    if (!opts->out_base)
		argp_error(state, "Option -o is required.");
	    break;
	case 'o':
	    opts->out_base = arg;
	    break;
	case 'u':
	    opts->update = 1;
	    break;
	case 'I':
	    cucon_list_append_ptr(&opts->import_paths, cu_str_new_cstr(arg));
	    break;
	default:
	    return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

struct argp_option main_options[] = {
    {"output-prefix", 'o', "PREFIX", 0,
     "Prefix for output, \".h\" and \".c\" will be appended.", 0},
    {"update", 'u', NULL, 0, "Only overwrite outputs if they would change."},
    {NULL, 'I', "DIRECTORY", 0, "Add DIRECTORY as import seach path."},
    {NULL}
};

struct argp main_argp = {
    main_options, main_parseopt, "FILES...", "Process cuex otab files.",
    NULL, NULL, NULL
};

int
main(int argc, char **argv)
{
    cuex_otab_t otab;
    struct main_opts_s main_opts;
    int err_cnt;
    cucon_init();
    ot_init();
    cucon_list_cct(&main_opts.import_paths);
    main_opts.out_base = NULL;
    main_opts.path = NULL;
    main_opts.update = 0;
    argp_parse(&main_argp, argc, argv, 0, NULL, &main_opts);
    cu_debug_assert(main_opts.out_base);
    otab = cuex_otab_new(16, ot_error_cb);
    err_cnt = ot_parse_file(otab, cu_str_new_cstr(main_opts.path),
			    &main_opts.import_paths);
    if (!err_cnt) {
	cu_str_t out_h, out_c;
	out_h = cu_str_new_2cstr(main_opts.out_base, ".h");
	out_c = cu_str_new_2cstr(main_opts.out_base, ".c");
	if (main_opts.update) {
	    cu_str_t out_h_old = NULL, out_c_old = NULL;
	    if (cuos_dentry_type(out_h) == cuos_dentry_type_file) {
		out_h_old = out_h;
		out_h = cu_str_new_2cstr(main_opts.out_base, ".h~");
	    }
	    if (cuos_dentry_type(out_c) == cuos_dentry_type_file) {
		out_c_old = out_c;
		out_c = cu_str_new_2cstr(main_opts.out_base, ".c~");
	    }
	    if (!cuex_otab_print_std_sources(otab, out_h, out_c))
		return 2;
	    if (out_h_old && cuos_file_update(out_h, out_h_old) < 0) {
		perror("cuos_file_update");
		++err_cnt;
	    }
	    if (out_c_old && cuos_file_update(out_c, out_c_old) < 0) {
		perror("cuos_file_update");
		++err_cnt;
	    }
	} else {
	    if (!cuex_otab_print_std_sources(otab, out_h, out_c))
		return 2;
	}
    }
    return err_cnt? 2 : 0;
}
