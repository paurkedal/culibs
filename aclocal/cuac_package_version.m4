AC_DEFUN([CUAC_PACKAGE_VERSION], [
    _cuac_v0=`echo $ECHO_N $PACKAGE_VERSION | cut -d. -f1`
    _cuac_v1=`echo $ECHO_N $PACKAGE_VERSION.0 | cut -d. -f2`
    _cuac_v2=`echo $ECHO_N $PACKAGE_VERSION.0.0 | cut -d. -f3`
    AC_DEFINE_UNQUOTED([PACKAGE_VERSION_0], [$_cuac_v0],
		       [The first component of the culibs version.])
    AC_DEFINE_UNQUOTED([PACKAGE_VERSION_1], [$_cuac_v1],
		       [The second component of the culibs version.])
    AC_DEFINE_UNQUOTED([PACKAGE_VERSION_2], [$_cuac_v2],
		       [The third component of the culibs version.])
])
