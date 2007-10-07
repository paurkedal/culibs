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

#ifndef CUEX_TEST_H
#define CUEX_TEST_H

#include <cuex/fwd.h>
#include <cu/test.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_test_h cuex/test.h: Utilities for Testing
 *@{\ingroup cuex_mod */

#define cuex_test_assert_eq(LHS, RHS) \
    cu_test_assert_binary(cu_test_op_eq, cuex_t, "!", LHS, RHS)

/*!@}*/
CU_END_DECLARATIONS

#endif
