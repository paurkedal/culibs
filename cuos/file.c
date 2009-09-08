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

#include <cuos/file.h>
#include <cu/str.h>
#include <unistd.h>

cu_order_t
cuos_file_order(cu_str_t path0, cu_str_t path1)
{
    FILE *in0, *in1;
    char ch0, ch1;

    in0 = fopen(cu_str_to_cstr(path0), "r");
    if (!in0)
	return cu_order_none;

    in1 = fopen(cu_str_to_cstr(path1), "r");
    if (!in1)
	return cu_order_none;

    do {
	ch0 = fgetc(in0);
	ch1 = fgetc(in1);
    } while (ch0 == ch1 && ch0 != EOF);

    fclose(in0);
    fclose(in1);
    return ch0 < ch1? cu_order_lt
	 : ch0 > ch1? cu_order_gt
	 :            cu_order_eq;
}

int
cuos_file_update(cu_str_t tmp_path, cu_str_t dst_path)
{
    if (cuos_file_order(tmp_path, dst_path) == cu_order_eq)
	return unlink(cu_str_to_cstr(tmp_path));
    else
	return rename(cu_str_to_cstr(tmp_path),
		      cu_str_to_cstr(dst_path)) < 0?  -1 : 1;
}

FILE *
cuos_fopen(cu_str_t path, char const *mode)
{
    return fopen(cu_str_to_cstr(path), mode);
}
