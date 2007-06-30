# Copyright (C) 2007  Petter Urkedal
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

m4_pattern_forbid([^cu4_])

AC_DEFUN([cu4_ident], [$1])

AC_DEFUN([cu4_list_empty], [])

AC_DEFUN([cu4_list_append],
    [ifelse([$1], [],
	[m4_quote(m4_dquote($2))],
	[m4_quote($1, m4_dquote($2))])])

AC_DEFUN([cu4_list_grow],
    [m4_define([$1],
	m4_ifdef([$1],
	    [m4_dquote($1, m4_dquote($2))],
	    [m4_dquote(m4_dquote($2))]))])

AC_DEFUN([cu4_list_at],
    [m4_pushdef([_cu4_f], [$]m4_incr($1))_cu4_f($2)[]m4_popdef([_cu4_f])])

