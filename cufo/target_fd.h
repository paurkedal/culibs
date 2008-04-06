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

#ifndef CUFO_TARGET_FD_H
#define CUFO_TARGET_FD_H

#include <cufo/stream.h>
#include <cu/inherit.h>
#include <iconv.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cufo_target_fd_h cufo/target_fd.h:
 *@{\ingroup cufo_target_mod */

struct cufo_fdconvinfo_s
{
    iconv_t cd;
    unsigned int wr_scale;
};

typedef struct cufo_fdstream_s *cufo_fdstream_t;
struct cufo_fdstream_s
{
    cu_inherit (cufo_stream_s);
    int fd;
    cu_bool_t do_close;
    struct cufo_fdconvinfo_s convinfo[2]; /* = { multibyte_iconv, wide_iconv } */
};

void cufo_fdstream_init(cufo_fdstream_t fos, int fd, char const *encoding);

/*!@}*/
CU_END_DECLARATIONS

#endif
