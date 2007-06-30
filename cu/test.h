/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_TEST_H
#define CU_TEST_H

#include <cu/fwd.h>
#include <atomic_ops.h>
#include <stdarg.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_test_h cu/test.h: Utilities for Testing
 *@{\ingroup cu_mod */

typedef enum {
    cu_test_bugaction_cont,	/* continue */
    cu_test_bugaction_abort,	/* abort after max_bug_count */
    cu_test_bugaction_exit,	/* exit after max_bug_count */
} cu_test_bugaction_t;

void cu_test_on_bug(cu_test_bugaction_t action, unsigned long max_bug_cnt);
int cu_test_bug_count(void);

void cuP_test_vbugf(char const *file, int line, char const *msg, va_list);
void cuP_test_bugf(char const *file, int line, char const *msg, ...);

#define cu_test_bugf(args...) cuP_test_bugf(__FILE__, __LINE__, args)
#define cu_test_vbugf(args...) cuP_test_vbugf(__FILE__, __LINE__, args)

#define cu_test_assert(EXPR)						\
    do {								\
	if (!(EXPR))							\
	    cu_test_bugf("assertion '"#EXPR"' failed.");		\
    } while (0)

#define cu_test_assert_binary(OP, type, PRI, LHS, RHS)			\
    do {								\
	type lhs = (LHS);						\
	type rhs = (RHS);						\
	if (!(OP(lhs, rhs)))						\
	    cu_test_bugf("assertion '"#LHS" = "#RHS"' failed, "		\
			 "LHS = %"PRI", RHS = %"PRI".", lhs, rhs);	\
    } while (0)

#define cu_test_op_eq(x, y) ((x) == (y))
#define cu_test_op_lt(x, y) ((x) < (y))
#define cu_test_op_leq(x, y) ((x) <= (y))
#define cu_test_op_gt(x, y) ((x) > (y))
#define cu_test_op_geq(x, y) ((x) >= (y))

#define cu_test_assert_int_eq(LHS, RHS) \
    cu_test_assert_binary(cu_test_op_eq, int, "d", LHS, RHS)
#define cu_test_assert_int_lt(LHS, RHS) \
    cu_test_assert_binary(cu_test_op_lt, int, "d", LHS, RHS)
#define cu_test_assert_int_leq(LHS, RHS) \
    cu_test_assert_binary(cu_test_op_leq, int, "d", LHS, RHS)
#define cu_test_assert_int_gt(LHS, RHS) \
    cu_test_assert_binary(cu_test_op_gt, int, "d", LHS, RHS)
#define cu_test_assert_int_geq(LHS, RHS) \
    cu_test_assert_binary(cu_test_op_geq, int, "d", LHS, RHS)

#define cu_test_assert_ptr_eq(LHS, RHS) \
    cu_test_assert_binary(cu_test_op_eq, void *, "p", LHS, RHS)

/*!@}*/
CU_END_DECLARATIONS

#endif
