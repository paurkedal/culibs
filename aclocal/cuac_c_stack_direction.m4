# Copyright (C) 2007  Petter Urkedal
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

AC_DEFUN([CUAC_C_STACK_DIRECTION],
  [ AC_MSG_CHECKING([stack direction])
    AC_TRY_RUN(
      [ int f(int* stackitem)
	{
	    int substackitem;
	    return &substackitem > stackitem;
	}
	int main()
	{
	    int stackitem;
	    return f(&stackitem);
	}
],
      [ cuac_c_stack_direction=-1
	AC_MSG_RESULT(-1) ],
      [ cuac_c_stack_direction=1
	AC_MSG_RESULT(1) ]
      [ AC_MSG_ERROR([Cross compilation is not supported.]) ] )
    AC_DEFINE_UNQUOTED([STACK_DIRECTION], [$cuac_c_stack_direction],
	[The unit (1 or -1) address direction of a growing stack.])
  ])
