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

#include <cutext/src.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


void
copy_to_stdout(cutext_src_t src)
{
    static const size_t bufsize = 10;
    for (;;) {
	size_t n;
	cutext_src_lookahead(src, bufsize);
	n = cutext_src_data_size(src);
	if (n == 0)
	    break;
	n = write(1, cutext_src_data_start(src), n);
	if (n <= 0) {
	    if (n == -1) perror("write");
	    else fprintf(stderr, "write failed\n");
	    exit(2);
	}
	cutext_src_advance(src, n);
    }
}

int
main()
{
    int fd = 0;
    cutext_src_t src;
    cutext_encoding_t chenc;

    cu_init();
    src = cutext_src_new(cutext_producer_new_read(fd));
    chenc = cutext_src_detect_chenc(src);
    fprintf(stderr, "chenc=%s\n", cutext_encoding_name(chenc));
    copy_to_stdout(src);
    return 0;
}
