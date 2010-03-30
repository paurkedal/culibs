# SYNOPSIS
#
#   CUAC_CONFIG_INSTALLDIRS(OUTPUT_PATH, VARIABLE_NAME)
#
# DESCRIPTION
#
#   Generates a C file which defines
#
#       cu_installdirs_t VARIABLE_NAME = ...;
#
#   initialised to the installation directories of the current package.

# Note!  The list of directory names must match cu/installdirs.h.
m4_define([_cuac_installdir_names],
	  [[prefix],
	   [exec_prefix],
	   [bindir],
	   [sbindir],
	   [libexecdir],
	   [sysconfdir],
	   [sharedstatedir],
	   [localstatedir],
	   [libdir],
	   [includedir],
	   [datarootdir],
	   [datadir],
	   [infodir],
	   [localedir],
	   [mandir],
	   [docdir],
	   [htmldir], [dvidir], [pdfdir], [psdir]])

AC_DEFUN([CUAC_CONFIG_INSTALLDIRS],
  [ m4_if([$1], [],
	  [m4_fatal([Missing first argument of CUAC_CONFIG_INSTALLDIRS])])
    m4_if([$2], [],
	  [m4_fatal([Missing secord argument of CUAC_CONFIG_INSTALLDIRS])])
    AC_CONFIG_COMMANDS([$1],
      [ cat >$1 <<__CUAC_END__
/* This is a generated file. */

#include <cu/installdirs.h>

struct cu_installdirs $2 = {
    0,
    {
__CUAC_END__
	m4_foreach([dirname], [_cuac_installdir_names],
	  [ _cuac_dir="$_cuac_installdir_[]dirname"
	    AS_CASE(["$_cuac_dir"],
		['${'*], [
		    _cuac_varname=${_cuac_dir%%/*}
		    _cuac_tmp=`echo ${_cuac_varname} | tr -d '${}'`
		    _cuac_prefix=CU_INSTALLDIR_[]AS_TR_CPP([${_cuac_tmp}])
		    _cuac_suffix="${_cuac_dir#${_cuac_varname}}"
		], [
		    _cuac_prefix=CU_INSTALLDIR_NONE
		    _cuac_suffix="${_cuac_dir}"
		])
	    _cuac_tmp="\"dirname\", CU_INSTALLDIR_[]m4_toupper(dirname)"
	    _cuac_tmp="$_cuac_tmp, $_cuac_prefix, \"$_cuac_suffix\""
	    echo >>$1 "	{$_cuac_tmp},"
	  ])
	echo >>$1 '	{NULL}'
	echo >>$1 '    }'
	echo >>$1 '};'
      ],
      [ m4_foreach([dirname], [_cuac_installdir_names],
		   [_cuac_installdir_[]dirname='$dirname[]'m4_newline])
      ])
  ])
