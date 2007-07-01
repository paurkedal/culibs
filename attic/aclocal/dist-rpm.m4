AC_DEFUN([AM_DIST_RPM],
  [ AC_ARG_ENABLE(rpm-topdir,
      [ AC_HELP_STRING([--enable-rpm-topdir=DIR],
	  [ Provide RPM topdir, yes for RPM default (.rpmmacros) or no
	    to use the build-directory. ])],
      [],
      [ enable_rpm_topdir=no ])
    test "$enable_rpm_topdir" != no || enable_rpm_topdir="`pwd`"
    AM_CONDITIONAL(nondefault_rpm_topdir, [test "$enable_rpm_topdir" != yes])
    AC_SUBST(rpm_topdir, [$enable_rpm_topdir])
    AC_CHECK_PROGS([RPMBUILD], [rpmbuild rpm])

    if test $enable_rpm_topdir = yes; then
	rpm_defines=
	rpm_mkinstalldirs='$(mkinstalldirs) BUILD_BUILDROOT SRPMS RPMS/i386 RPMS/noarch'
    else
	rpm_defines="--define '_topdir \$(rpm_topdir)'"
	rpm_mkinstalldirs=:
    fi
    cat >config.dist-rpm.am <<EOF
dist-rpm: \$(PACKAGE).spec dist-bzip2
	$rpm_mkinstalldirs
	release=0; test ! -e release.int || read release <release.int; \\
	\$(RPMBUILD) -ba \$(PACKAGE).spec \$(RPMBUILD_FLAGS) \\
	    $rpm_defines --define "_sourcedir \`pwd\`" \\
	    --define "release \$\$release" \\
	&& ((release=release+1)) \\
	&& echo \$\$release >release.int \\
	&& rm -rf BUILDROOT
.PHONY: dist-rpm
EOF
    dist_rpm_rule=config.dist-rpm.am
    AC_SUBST_FILE(dist_rpm_rule)
  ])
