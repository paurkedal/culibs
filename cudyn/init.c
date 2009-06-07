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

#include <cu/conf.h>

void cudynP_type_init(void);
#ifdef CUCONF_HAVE_LIBFFI
void cudynP_proto_init(void);
#endif
void cudynP_misc_init(void);

void
cudynP_init()
{
    cudynP_type_init();
#ifdef CUCONF_HAVE_LIBFFI
    cudynP_proto_init();
#endif
    cudynP_misc_init();
}
