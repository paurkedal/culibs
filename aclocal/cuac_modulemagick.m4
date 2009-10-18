# Copyright (C) 2007  Petter Urkedal
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.


AC_DEFUN([CUAC_MODULE],
  [ cu4_list_grow([CUAC_MODULE_LIST], [$1])
    m4_define(m4_quote($1[]_DEPS), m4_quote($2))
    m4_define(m4_quote($1[]_LIBNAME),
	      m4_quote(m4_if([$3], [], [lib$1.la], [$3])))
  ])

AC_DEFUN([CUAC_MODULE_ALIAS],
  [ cu4_list_grow([_CUAC_MODULE_ALIASES], [[$1, [$2]]]) ])

AC_DEFUN([CUAC_MODULE_IMPLIES],
  [ cu4_list_grow($1[_IMPLIES], [$2]) ])

AC_DEFUN([CUAC_MODULE_EQUIVALENCE],
  [ cu4_list_grow([_CUAC_MODULE_EQV_LIST], [$1]) ])

AC_DEFUN([CUAC_ARG_MODULES],
  [ # Parse modulemagick command-line.
    AC_ARG_ENABLE([modules],
	[AC_HELP_STRING([--enable-modules=MODULELIST],
	    [Comma-separated list the modules to build.  By default, all
	     modules which present in the source directory are built.])],
      [ enabled_modules="${enableval//,/ }" ],
      [ m4_foreach([_mod], [CUAC_MODULE_LIST],
	  [ test ! -e $srcdir/_mod/_mod.am || enabled_modules="$enabled_modules _mod"
	  ])
      ])

    # Initialise the $enable_MODULE and $check_MODULE variables.
    m4_foreach([_mod], [CUAC_MODULE_LIST],
      [ enable_[]_mod=false
	check_[]_mod=false
      ])
    m4_foreach([_mod], [CUAC_MODULE_LIST],
      [ m4_append([CUAC_MODULE_CASE], _mod, [|]) ])
    for module in $enabled_modules; do
	case $module in
m4_foreach([_alias], m4_quote(_CUAC_MODULE_ALIASES), [dnl
	    cu4_list_at([0], [_alias]))
m4_foreach([_mod], [cu4_list_at([1], [_alias])], [dnl
		enable_[]_mod=true
])dnl
		;;
])dnl
	    CUAC_MODULE_CASE)
		eval enable_$module=true
		;;
	    *)
		AC_MSG_ERROR([Unknown module $module])
		;;
	esac
    done

    # Enable all implied modules.
    m4_foreach([_mod], [CUAC_MODULE_LIST],
      [ m4_ifdef(_mod[_IMPLIES],
	  [ m4_foreach([_impl], m4_defn(_mod[_IMPLIES]),
	      [ AS_IF([$enable_[]_mod && ! $enable_[]_impl],
		  [ AC_MSG_NOTICE([Enabeling module _impl due to _mod.])
		    enable_[]_impl=true
		  ]) ]) ]) ])
    m4_foreach([_eqv], [_CUAC_MODULE_EQV_LIST],
      [ AS_IF([m4_foreach([_mod], [_eqv], [$enable_[]_mod || ])false],
	  [ m4_foreach([_mod], [_eqv],
	      [ AS_IF([! $enable_[]_mod],
		  [ AC_MSG_NOTICE([Enabeling module _mod due to equivalence of {_eqv}])
		    enable_[]_mod=true
		  ]) ]) ]) ])

    # For each dependency of an enabled, assign check_DEP=true to force
    # checking for it's presense.
    m4_foreach([_mod], [CUAC_MODULE_LIST],
      [ m4_if(m4_defn(_mod[_DEPS]), [], [],
	  [ AS_IF([$enable_[]_mod],
	      [ m4_foreach([_dep], m4_defn(_mod[_DEPS]),
		  [ check_[]_dep=true; ]) ]) ]) ])

    # Collect the lists of built, imported, and unused modules, and define
    # conditionals and substitutions.
    cuac_build_modules=
    cuac_import_modules=
    cuac_unused_modules=
    m4_foreach([_mod], [CUAC_MODULE_LIST],
      [ libadd="m4_foreach([_dep], m4_defn(_mod[_DEPS]),
			   [\$[](m4_toupper(_dep)_LIBS) ])"
	if $enable_[]_mod; then
	    cuac_build_modules="$cuac_build_modules _mod"
	    AC_SUBST(m4_toupper(_mod)[_LIBS], m4_defn(_mod[_LIBNAME]))
	    AC_SUBST(m4_toupper(_mod)[_DEPENDENCIES], ["$libadd"])
	elif $check_[]_mod; then
	    cuac_import_modules="$cuac_import_modules _mod"
	    PKG_CHECK_MODULES(m4_toupper(_mod), _mod)
	else
	    cuac_unused_modules="$cuac_unused_modules _mod"
	fi
	AM_CONDITIONAL(enable_[]_mod, [$enable_[]_mod])
	AC_SUBST(m4_toupper(_mod)[_LIBADD], ["$libadd"])
      ])
    AC_MSG_NOTICE([Modules to build:${cuac_build_modules}])
    AC_MSG_NOTICE([Modules imported:${cuac_import_modules}])
    AC_MSG_NOTICE([Modules not used:${cuac_unused_modules}])
  ])
