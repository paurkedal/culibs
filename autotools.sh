#! /bin/sh

automake_versions="1.10 1.9 1.8 1.7"

echodo() {
    echo '[autogen]' $*
    "$@"
}

if test -r $prefix/share/config.site; then
    echo "Including $prefix/share/config.site."
    . $prefix/share/config.site
fi

srcdir=`dirname $0`
srcdir=${srcdir%/common}
test -n "$srcdir" || srcdir=.
builddir=`pwd`

if test "$srcdir" != .; then
    echodo cd $srcdir
fi

am_version=
for v in $automake_versions; do
    if type &>/dev/null aclocal-$v && type &>/dev/null automake-$v; then
	am_version="-$v"
	break
    fi
done

[ -d build-aux ] || mkdir build-aux
echodo libtoolize --automake --force --copy
#ACLOCAL_FLAGS="-I . $ACLOCAL_AMFLAGS"
#export ACLOCAL_FLAGS
echodo aclocal$am_version -I aclocal $ACLOCAL_FLAGS
echodo autoheader
echodo automake$am_version -ac
echodo autoconf

##
##  If we are in the the source directory, exit.
##
if test "$srcdir" = .; then
    fmt <<EOF
Building in the source directory is OK, but I do not run ./configure here by
default.
EOF
    exit 0
fi

##
##  It can be quite annoying to mess up the wrong directory, so check
##  that it is empty, or that there has been a previous configuration
##  here.
##
echodo cd $builddir

empty=true
if test ! -f config.log; then
    for x in `ls`; do
	if test $x != my-config.sh; then
	    empty=false
	    break
	fi
    done
fi

if test $empty = false; then
    cat <<EOF

This directory is not empty, and there seems to be no prior autoconf
configuration here.  Are you sure you don't want to start from an empty
directory?

Say \`touch config.log' first if you want to configure here anyway.

EOF
    exit 1
fi

echodo $srcdir/configure $configure_flags "$@"
