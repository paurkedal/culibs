dnl  Macro to detect Norman Walsh’s DocBook XSLT stylesheets.
AC_DEFUN([AM_CCF_DOCBOOK], [
    AC_CHECK_PROGS([xsltproc], xsltproc)
    AC_ARG_ENABLE(docs,
	AC_HELP_STRING([--enable-docs], [Disable building of documentation]),
	[], [enable_docs=no])
    AM_CONDITIONAL(enable_docs, [test $enable_docs != no])
    AM_CONDITIONAL(have_docbook_to_html_xsl, [test -n "$docbook_to_html_xsl" ])
    AM_CONDITIONAL(have_docbook_to_xhtml_xsl, [test -n "$docbook_to_xhtml_xsl" ])
    AM_CONDITIONAL(have_docbook_to_fo_xsl, [test -n "$docbook_to_fo_xsl" ])
    AM_CONDITIONAL(have_docbook_to_man_xsl, [test -n "$docbook_to_man_xsl" ])

    AC_ARG_WITH([docbook-xsl-dir],
	AC_HELP_STRING(
	    [--with-docbook-xsl-dir=DIR],
	    [Specify root for locating Norman Walsh’s DocBook XSL
	     stylesheets]),
	[],
        [
	    for dir in \
		/usr/share/sgml/docbook-xsl \
		/usr/share/sgml/docbook/xsl-stylesheets; do
		if test -e $dir; then
		    with_docbook_xsl_dir=$dir
		    break
		fi
	    done
        ])

    docbook_to_html_xsl=$with_docbook_xsl_dir/html/docbook.xsl
    docbook_to_xhtml_xsl=$with_docbook_xsl_dir/xhtml/docbook.xsl
    docbook_to_fo_xsl=$with_docbook_xsl_dir/fo/docbook.xsl
    docbook_to_man_xsl=$with_docbook_xsl_dir/manpages/docbook.xsl

    AC_ARG_WITH([docbook-to-html-xsl],
	AC_HELP_STRING(
	    [--with-docbook-to-html-xsl=STYLESHEET],
	    [Specify location of XSLT to turn DocBook into HTML]),
	[	docbook_to_html_xsl=$withval ])
    AC_ARG_WITH([docbook-to-xhtml-xsl],
	AC_HELP_STRING(
	    [--with-docbook-to-xhtml-xsl=STYLESHEET],
	    [Specify location of XSLT to turn DocBook into XHTML]),
	[	docbook_to_xhtml_xsl=$withval ])
    AC_ARG_WITH([docbook-to-fo-xsl],
	AC_HELP_STRING(
	    [--with-docbook-to-fo-xsl=STYLESHEET],
	    [Specify location of XSLT to turn DocBook into XSL FOs]),
	[	docbook_to_fo_xsl=$withval ])
    AC_ARG_WITH([docbook-to-man-xsl],
	AC_HELP_STRING(
	    [--with-docbook-to-man-xsl=STYLSHEET],
	    [Specify location of XSLT to turn DocBook into a manual page]),
	[	docbook_to_man_xsl=$withval ])

    AC_MSG_CHECKING(for DocBook to HTML style sheet)
    if test -e $docbook_to_html_xsl; then
	AC_MSG_RESULT(yes)
    else
	AC_MSG_RESULT(no)
	docbook_to_html_xsl=
    fi
    AC_MSG_CHECKING(for DocBook to XHTML style sheet)
    if test -e $docbook_to_xhtml_xsl; then
	AC_MSG_RESULT(yes)
    else
	AC_MSG_RESULT(no)
	docbook_to_xhtml_xsl=
    fi
    AC_MSG_CHECKING(for DocBook to FO style sheet)
    if test -e $docbook_to_fo_xsl; then
	AC_MSG_RESULT(yes)
    else
	AC_MSG_RESULT(no)
	docbook_to_fo_xsl=
    fi
    AC_MSG_CHECKING(for DocBook to manpages style sheet)
    if test -e $docbook_to_man_xsl; then
	AC_MSG_RESULT(yes)
    else
	AC_MSG_RESULT(no)
	docbook_to_man_xsl=
    fi

    AC_SUBST(docbook_to_html_xsl)
    AC_SUBST(docbook_to_xhtml_xsl)
    AC_SUBST(docbook_to_fo_xsl)
    AC_SUBST(docbook_to_man_xsl)
])
