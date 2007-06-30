/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuos/path.h>
#include <cu/str.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int errors = 0;


void
test_path(char *cstr, char *xpt)
{
    cu_str_t str = cuos_path_from_cstr(cstr);
    cu_bool_fast_t is_norm = cuos_is_path_cstr(cstr);
    if (is_norm != (xpt != NULL && strcmp(cstr, xpt) == 0)) {
	fprintf(stderr,
		"cuos_is_path_cstr(\"%s\") = %d, should be %d\n",
		cstr, is_norm, !is_norm);
	++errors;
    }

    if (str == NULL) {
	if (xpt != NULL) {
	    fprintf(stderr,
		    "cuos_path_from_cstr(\"%s\") = NULL, "
		    "should be \"%s\"\n", cstr, xpt);
	    ++errors;
	}
    }
    else if (xpt == NULL) {
	if (str != NULL) {
	    fprintf(stderr,
		    "cuos_path_from_cstr(\"%s\") = \"%s\", "
		    "should be invalid\n", cstr, cu_str_to_cstr(str));
	    ++errors;
	}
    }
    else if (strcmp(cu_str_to_cstr(str), xpt) != 0) {
	fprintf(stderr,
		"cuos_path_from_cstr(\"%s\") = \"%s\", "
		"should be \"%s\"\n", cstr, cu_str_to_cstr(str), xpt);
	++errors;
    }
}

cu_str_t
rnd_path(int n)
{
    cu_str_t str;
    if (n == 0)
	return cu_str_new_charr(".", 1);
    if (lrand48() % 2)
	str = cu_str_new_charr("/", 1);
    else {
	int i = lrand48() % n;
	n -= i;
	str = cu_str_new();
	while (i--)
	    cu_str_append_charr(str, "../", 3);
    }
    while (n--) {
	char buf[4];
	int i;
	int k = lrand48() % 3 + 1;
	for (i = 0; i < k; ++i)
	    buf[i] = (lrand48() % 27) + 'a';
	buf[k++] = '/';
	cu_str_append_charr(str, buf, k);
    }
    return str;
}

void
test_split_join(void)
{
    int i;
    for (i = 0; i < 10000; ++i) {
	int i = lrand48() % 8;
	int j = lrand48() % (2*i + 1) - i;
	cu_str_t sp;
	cu_str_t s = cuos_path_from_str(rnd_path(i));
	cu_str_t s0, s1;
	if (!cuos_path_split(s, j, &s0, &s1)) {
	    fprintf(stderr, "Failed to split \"%s\" at %d.\n",
		    cu_str_to_cstr(s), j);
	    ++errors;
	}
	sp = cuos_path_join(s0, s1);
	if (cu_str_cmp(s, sp) != 0) {
	    fprintf(stderr,
		    "\"%s\" is split to (\"%s\", \"%s\") "
		    "but rejoined to \"%s\".\n",
		    cu_str_to_cstr(s),
		    cu_str_to_cstr(s0), cu_str_to_cstr(s1),
		    cu_str_to_cstr(sp));
	    ++errors;
	}
	if (j == -1) {
	    cu_str_t sp0 = cuos_path_dir(s);
	    cu_str_t sp1 = cuos_path_base(s);
	    if (cu_str_cmp(sp0, s0) != 0) {
		fprintf(stderr, "cuos_path_dir(\"%s\") = \"%s\".\n",
			cu_str_to_cstr(s), cu_str_to_cstr(sp0));
		++errors;
	    }
	    if (cu_str_cmp(sp1, s1) != 0) {
		fprintf(stderr, "cuos_path_nondir(\"%s\") = \"%s\".\n",
			cu_str_to_cstr(s), cu_str_to_cstr(sp1));
		++errors;
	    }
	}
    }
}

void
test_split(char *s, int pos, char *s0, char *s1)
{
    cu_str_t sj = cuos_path_join(cu_str_new_cstr(s0),
				      cu_str_new_cstr(s1));
    cu_str_t s0s, s1s;
    if (!cuos_path_split(cu_str_new_cstr(s), pos, &s0s, &s1s)) {
	fprintf(stderr, "Failed to split path \"%s\" at %d.\n", s, pos);
	++errors;
	return;
    }
    if (strcmp(s, cu_str_to_cstr(sj)) != 0) {
	fprintf(stderr, "Join \"%s\" should be \"%s\".\n",
		cu_str_to_cstr(sj), s);
	++errors;
    }
    if (strcmp(cu_str_to_cstr(s0s), s0) != 0 ||
	strcmp(cu_str_to_cstr(s1s), s1) != 0) {
	fprintf(stderr, "Split (\"%s\", \"%s\") should be (\"%s\", \"%s\").\n",
		cu_str_to_cstr(s0s), cu_str_to_cstr(s1s), s0, s1);
	++errors;
    }
}

void
test_depth(char *s, int xdepth, int xncomp)
{
    cu_str_t path = cuos_path_from_cstr(s);
    int depth = cuos_path_depth(path);
    int ncomp = cuos_path_component_count(path);
    if (depth != xdepth) {
	fprintf(stderr, "Depth of %s is %d, should be %d.\n",
		s, depth, xdepth);
	++errors;
    }
    if (ncomp != xncomp) {
	fprintf(stderr, "component count of %s is %d, should be %d.\n",
		s, ncomp, xncomp);
	++errors;
    }
}

void
test_join(char *s0, char *s1, char *s)
{
    cu_str_t joined = cuos_path_join(cu_str_new_cstr(s0),
					  cu_str_new_cstr(s1));
    if (strcmp(cu_str_to_cstr(joined), s) != 0) {
	fprintf(stderr,
		"Join of (\"%s\", \"%s\") gives \"%s\", should be \"%s\".\n",
		s0, s1, cu_str_to_cstr(joined), s);
	++errors;
    }
}

int
main()
{
    cu_init();

    test_path("", NULL);
    test_path("/", "/");
    test_path(".", ".");
    test_path("./", ".");
    test_path("/.", "/");
    test_path("..", "..");
    test_path("../", "..");
    test_path("/..", NULL);
    test_path("../..", "../..");
    test_path(".//./", ".");
    test_path("..///.", "..");
    test_path("../..///", "../..");
    test_path("a", "a");
    test_path("a/", "a");
    test_path("/a", "/a");
    test_path("./a", "a");
    test_path("././a/.", "a");
    test_path("a/b", "a/b");
    test_path("/a/b/", "/a/b");
    test_path("//a//b//.", "/a/b");
    test_path("a/./b/./../.", "a");
    test_path("a/b/../..", ".");
    test_path("a/b/..//../../..", "../..");
    test_path("a/.././b", "b");
    test_path("a/../../b/..", "..");
    test_path("bla/blaa/", "bla/blaa");
    test_path("bla/blaa//", "bla/blaa");
    test_path("/bla", "/bla");
    test_path("//bla", "/bla");
    test_path("test/./bla", "test/bla");
    test_path("test/../bla", "bla");
    test_path("test/../../bla", "../bla");
    test_path("test/../../bla/be/../../..", "../..");
    test_split_join();

    test_join(".", "..", "..");
    test_join("a/b", "..", "a");
    test_join("a/b", "../..", ".");
    test_join("a/b", "../../..", "..");
    test_join("../a", "..", "..");
    test_join("../a", "../..", "../..");
    test_join(".", ".", ".");

    test_split("/", 0, ".", "/");
    test_split("/", 1, "/", ".");
    test_split(".", 0, ".", ".");
    test_split("a", 0, ".", "a");
    test_split("a", 1, "a", ".");
    test_split("a", -1, ".", "a");
    test_split("/a", 0, ".", "/a");
    test_split("/a", 1, "/", "a");
    test_split("/a", 2, "/a", ".");
    test_split("/a", -1, "/", "a");
    test_split("/a", -2, ".", "/a");
    test_split("../vu", 0, ".", "../vu");
    test_split("../vu", 1, "..", "vu");
    test_split("../vu", 2, "../vu", ".");
    test_split("../vu", -1, "..", "vu");
    test_split("../vu", -2, ".", "../vu");

    test_depth(".", 0, 0);
    test_depth("/", 1, 1);
    test_depth("a", 1, 1);
    test_depth("a/b", 2, 2);
    test_depth("..", -1, 1);
    test_depth("../..", -2, 2);
    test_depth("../a/b", 1, 3);
    test_depth("/al", 2, 2);

    return errors != 0;
}
