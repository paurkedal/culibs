/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007--2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#ifndef CU_CONF_H
#define CU_CONF_H

#include <cu/conf_ac.h>
#include <cu/conf_fixed.h>

#define CU_3VERSION(v0, v1, v2) (((v0)<<24) + ((v1)<<16) + ((v2) << 8))
#define CU_2VERSION(v0, v1) CU_3VERSION(v0, v1, 0)

#define CUCONF_VERSION	CU_3VERSION(CUCONF_PACKAGE_VERSION_0, \
				    CUCONF_PACKAGE_VERSION_1, \
				    CUCONF_PACKAGE_VERSION_2)

#if !defined(CUCONF_ENABLE_HASHCONS_DISAPP)
#  define CUCONF_ENABLE_GC_DISCLAIM
#endif

#endif
