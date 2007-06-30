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

#include <custo/binary_io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>

void
fail(char const *msg, ...)
{
    va_list va;
    fprintf(stderr, "error: ");
    va_start(va, msg);
    vfprintf(stderr, msg, va);
    va_end(va);
    fputc('\n', stderr);
    exit(1);
}

int
main(int argc, char **argv)
{
#define N 10000
    static intmax_t saved[N];
    int repeat = 1000;
    intmax_t i;
    FILE *file;
    if (argc > 1) {
	int i;
	for (i = 1; i < argc; ++i)
	    custo_fwrite_intmax(atol(argv[i]), stdout);
	return 0;
    }
    while (repeat-- > 0) {
	file = fopen("tmp.bin", "w");
	if (!file)
	    fail("could not create tmp.bin");
	for (i = 0; i < N; ++i) {
	    intmax_t j = lrand48() + RAND_MAX*lrand48();
	    saved[i] = j;
	    if (!custo_fwrite_uintmax(j, file) ||
		!custo_fwrite_intmax(j, file) ||
		!custo_fwrite_intmax(-j, file))
		fail("failed to write at pos %ld", ftell(file));
	}
	fputc('@', file);
	fclose(file);
	file = fopen("tmp.bin", "r");
	if (!file)
	    fail("could not open tmp.bin for reading");
	for (i = 0; i < N; ++i) {
	    uintmax_t ju;
	    intmax_t jp;
	    intmax_t jm;
	    intmax_t j = saved[i];
	    if (!custo_fread_uintmax(&ju, file) ||
		!custo_fread_intmax(&jp, file) ||
		!custo_fread_intmax(&jm, file))
		fail("failed to read at pos %ld", ftell(file));
	    if ((intmax_t)ju != j)
		fail("unsigned read of %"PRIdMAX" should be %"PRIdMAX, ju, j);
	    if (jp != j)
		fail("signed read of %"PRIdMAX" should be %"PRIdMAX, jp, j);
	    if (jm != -j)
		fail("signed read of %"PRIdMAX" should be %"PRIdMAX, jm, -j);
	}
	if (fgetc(file) != '@' || fgetc(file) != EOF)
	    fail("out of sync");
    }
    return 0;
}
