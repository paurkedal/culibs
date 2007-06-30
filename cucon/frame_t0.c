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

#include <cucon/frame.h>
#include <cu/test.h>

int main()
{
    int i, j;
    int *frame, *framep, *framepp;

    cucon_init();
    frame = NULL;
    for (i = 0; i < 1000; ++i) {
	frame = cucon_frame_push(frame, sizeof(int));
	*frame = i;
    }
    cu_test_assert(cucon_frame_depth(frame) == 1000);
    framep = cucon_frame_pop(frame);
    cu_test_assert(*framep == 998);
    for (i = 0; i < 1000; ++i) {
	framep = cucon_frame_at(frame, i);
	cu_test_assert(*framep == 999 - i);
	for (j = i; j < 1000; ++j) {
	    framepp = cucon_frame_at(framep, j - i);
	    cu_test_assert(*framepp == 999 - j);
	}
    }
    return 2*!!cu_test_bug_count();
}
