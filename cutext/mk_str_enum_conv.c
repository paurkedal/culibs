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

/* A quick hack to creat nasty recursive 'switch' blocks to
 * efficiently enumerate strings.  As a bonus, also creates the
 * inverse function. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <argp.h>

typedef struct args_s *args_t;
struct args_s
{
    char *input_path;
    char *enum_name;
    char *default_value;
    char *str_to_enum;
    char *enum_to_str;
};

typedef struct node_s *node_t;
struct node_s
{
    char *enum_name;
    node_t index[0x100];
};

typedef struct assl_s *assl_t;
struct assl_s
{
    char *enum_name;
    char *value;
    assl_t next;
};

assl_t
assq(assl_t node, char *enum_name)
{
    while (node) {
	if (strcmp(node->enum_name, enum_name) == 0)
	    return node;
	node = node->next;
    }
    return NULL;
}

char *argv0;

void
scan_line(FILE *in, node_t node, char *path, int line, assl_t *assl)
{
    char ch;
    char *buf;
    size_t buf_size;
    size_t i;
    char *value;
    char *enum_name;
    ch = fgetc(in);
    if (ch == '\n' || ch == EOF)
	return;
    buf_size = 80;
    buf = malloc(buf_size);
    i = 0;
    while (ch != EOF && ch != '\t' && ch != '\n') {
	if (!node->index[(unsigned char)ch]) {
	    node = node->index[(unsigned char)ch]
		= calloc(1, sizeof(struct node_s));
	    if (!node) {
		perror(argv0);
		exit(1);
	    }
	}
	else
	    node = node->index[(unsigned char)ch];
	if (i >= buf_size) {
	    buf = realloc(buf, buf_size *= 2);
	    if (!buf) {
		perror(argv0);
		exit(1);
	    }
	}
	buf[i++] = ch;
	ch = fgetc(in);
    }
    while (ch == '\t' || ch == ' ')
	ch = fgetc(in);
    value = buf;

    buf_size = 80;
    buf = malloc(buf_size);
    if (!buf) {
	perror(argv0);
	exit(1);
    }
    i = 0;
    if (!isalpha(ch) && ch != '_') {
	fprintf(stderr, "%s:%d: Expecting character or underscore.\n",
		path, line);
	exit(1);
    }
    while (isalnum(ch) || ch == '_') {
	if (i >= buf_size) {
	    buf = realloc(buf, buf_size *= 2);
	    if (!buf) {
		perror(argv0);
		exit(1);
	    }
	}
	buf[i++] = ch;
	ch = fgetc(in);
    }
    buf[i] = 0;
    enum_name = buf;
    node->enum_name = enum_name;
    while (ch == '\t' || ch == ' ')
	ch = fgetc(in);
    if (ch != '\n') {
	fprintf(stderr, "%s:%d: Bad syntax, expecting newline.\n", path, line);
	exit(1);
    }

    if (!assq(*assl, enum_name)) {
	assl_t assl0 = malloc(sizeof(struct assl_s));
	assl0->enum_name = enum_name;
	assl0->value = value;
	assl0->next = *assl;
	*assl = assl0;
    }
}

void
switchem(node_t node, int indent)
{
    int i;
    printf("switch (*s++) {\n");
    if (node->enum_name)
	printf("case 0:\nreturn %s;\n", node->enum_name);
    for (i = 0; i < 0x100; ++i) {
	if (node->index[i]) {
	    if (i < 0x80 && isprint(i) && i != '\'' && i != '\\')
		printf("case '%c':\n", i);
	    else
		printf("case '\\%03o':\n", i);
	    switchem(node->index[i], indent + 1);
	    printf("break;\n");
	}
    }
    printf("}\n");
}

void
scan_file(args_t args)
{
    FILE *in = fopen(args->input_path, "r");
    int line = 1;
    struct node_s tree;
    assl_t assl = NULL;
    memset(&tree, 0, sizeof(struct node_s));
    if (!in) {
	perror(argv0);
	exit(1);
    }
    while (!feof(in)) {
	scan_line(in, &tree, args->input_path, line, &assl);
	++line;
    }
    printf("%s %s(char const *s)\n{\n",
	   args->enum_name? args->enum_name : "int",
	   args->str_to_enum);
    switchem(&tree, 1);
    printf("return %s;\n}\n",
	   args->default_value? args->default_value : "-1");

    printf("\nchar const *%s(%s e)\n{\nswitch(e) {\n",
	   args->enum_to_str,
	   args->enum_name? args->enum_name : "int");
    while (assl) {
	printf("case %s: return \"%s\";\n", assl->enum_name, assl->value);
	assl = assl->next;
    }
    printf("default: return NULL;\n}\n}\n");
}

/* Main
 * ==== */

static error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
    args_t args = state->input;
    switch (key) {
    case 't':
	args->enum_name = arg;
	break;
    case 'd':
	args->default_value = arg;
	break;
    case 'f':
	args->str_to_enum = arg;
	break;
    case 'g':
	args->enum_to_str = arg;
	break;
    case ARGP_KEY_ARG:
	if (args->input_path)
	    argp_usage(state);
	args->input_path = arg;
	break;
    case ARGP_KEY_END:
	if (!args->input_path ||
	    !(args->enum_name && args->str_to_enum && args->enum_to_str))
	    argp_usage(state);
	break;
    default:
	return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp_option option_arr[] = {
    {"--type", 't', "TYPE", 0, "The name of the enum type."},
    {"--default-value", 'd', "ENUM_CASE", 0,
     "The value to return for non-matches."},
    {"--str-to-enum", 'f', "FUNCTION", 0,
     "Name of function which converts strings to enums"},
    {"--enum-to-str", 'g', "FUNCTION", 0,
     "Name of function which converts enums to strings"},
    {NULL, 0}
};

struct argp argp = {
    option_arr, parse_opt, 0,
    "cutext_mk_str_enum_conv -- Tool to create C code to convert between strings and enums"
};

int
main(int argc, char **argv)
{
    struct args_s args;
    memset(&args, 0, sizeof(args));
    argp_parse(&argp, argc, argv, 0, NULL, &args);
    argv0 = argv[0];
    scan_file(&args);
    return 0;
}
