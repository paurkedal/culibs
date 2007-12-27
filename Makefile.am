# Directories
#
culibsdir = $(includedir)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)
aclocaldir = $(datadir)/aclocal
pkgconfigdir = $(libdir)/pkgconfig
docdir = $(datadir)/doc
pkgdocdir = $(docdir)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)
htmldir = $(pkgdocdir)/html


# Common Flags
#
common_ldflags = -release $(PACKAGE_VERSION)
ACLOCAL_AMFLAGS = -I aclocal
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir) \
	$(BDWGC_CFLAGS) $(FFI_CFLAGS)


# Initialise Accumulative Variables
#
lib_LTLIBRARIES =
bin_PROGRAMS =
bin_SCRIPTS =
dist_bin_SCRIPTS =
noinst_HEADERS =
pkgconfig_DATA =
noinst_DATA =
EXTRA_DIST =
BUILT_SOURCES = cu/conf_ac.h
DISTCLEANFILES = cu/conf_ac.h
CLEANFILES =


# Per-Module Build Instructions
#

include cu/cu.am
include cuoo/cuoo.am
include cucon/cucon.am
if enable_cucon
    lib_LTLIBRARIES	+= libcubase.la
    libcubase_la_LIBADD	 = $(BDWGC_LIBS) $(UNWIND_LIBS)
    libcubase_la_LDFLAGS = $(common_ldflags)
    libcubase_la_SOURCES = $(cu_sources) $(cucon_sources) $(cuoo_sources)
    pkgconfig_DATA	+= pkgconfig/cubase.pc
    noinst_DATA		+= pkgconfig/cubase-uninstalled.pc
    dist_bin_SCRIPTS	+= bin/sedi
endif
EXTRA_DIST		+= cu/cu.am cuoo/cuoo.am cucon/cucon.am

include cuflow/cuflow.am
if enable_cuflow
    lib_LTLIBRARIES	+= libcuflow.la
    libcuflow_la_LIBADD	 = @CUFLOW_LIBADD@ $(LIBRT_LIBS) -lm
    libcuflow_la_DEPENDENCIES = @CUFLOW_DEPENDENCIES@
    libcuflow_la_LDFLAGS = $(common_ldflags)
    libcuflow_la_SOURCES = $(cuflow_sources)
    pkgconfig_DATA	+= pkgconfig/cuflow.pc
    noinst_DATA		+= pkgconfig/cuflow-uninstalled.pc
endif
EXTRA_DIST		+= cuflow/cuflow.am

include cugra/cugra.am
if enable_cugra
    lib_LTLIBRARIES	+= libcugra.la
    libcugra_la_LIBADD	 = @CUGRA_LIBADD@ $(BUDDY_LIBS)
    libcugra_la_DEPENDENCIES = @CUGRA_DEPENDENCIES@
    libcugra_la_LDFLAGS	 = $(common_ldflags)
    libcugra_la_SOURCES	 = $(cugra_sources)
    pkgconfig_DATA	+= pkgconfig/cugra.pc
    noinst_DATA		+= pkgconfig/cugra-uninstalled.pc
endif

include cuos/cuos.am
if enable_cuos
    lib_LTLIBRARIES	+= libcuos.la
    libcuos_la_LIBADD	 = @CUOS_LIBADD@
    libcuos_la_DEPENDENCIES = @CUOS_DEPENDENCIES@
    libcuos_la_LDFLAGS	 = $(common_ldflags)
    libcuos_la_SOURCES	 = $(cuos_sources)
    pkgconfig_DATA	+= pkgconfig/cuos.pc
    noinst_DATA		+= pkgconfig/cuos-uninstalled.pc
endif
EXTRA_DIST		+= cuos/cuos.am

include cutext/cutext.am
if enable_cutext
    lib_LTLIBRARIES	+= libcutext.la
    libcutext_la_LIBADD	 = @CUTEXT_LIBADD@
    libcutext_la_DEPENDENCIES = @CUTEXT_DEPENDENCIES@
    libcutext_la_LDFLAGS = $(common_ldflags)
    pkgconfig_DATA	+= pkgconfig/cutext.pc
    noinst_DATA		+= pkgconfig/cutext-uninstalled.pc
endif

include custo/custo.am
if enable_custo
    lib_LTLIBRARIES	+= libcusto.la
    libcusto_la_LIBADD	 = @CUSTO_LIBADD@
    libcusto_la_DEPENDENCIES = @CUSTO_DEPENDENCIES@
    libcusto_la_LDFLAGS	 = $(common_ldflags)
    pkgconfig_DATA	+= pkgconfig/custo.pc
    noinst_DATA		+= pkgconfig/custo-uninstalled.pc
endif

include cuex/cuex.am
include cudyn/cudyn.am
if enable_cuex
    lib_LTLIBRARIES	+= libcuex.la
    libcuex_la_LIBADD	 = @CUEX_LIBADD@ $(FFI_LIBS)
    libcuex_la_DEPENDENCIES = @CUEX_DEPENDENCIES@
    libcuex_la_LDFLAGS	 = $(common_ldflags)
    libcuex_la_SOURCES	 = $(cuex_sources) $(cuex_built_sources) \
			   $(cudyn_sources)
    BUILT_SOURCES	+= $(cuex_built_headers) $(cuex_built_sources)
    pkgconfig_DATA	+= pkgconfig/cuex.pc
    noinst_DATA		+= pkgconfig/cuex-uninstalled.pc
endif
EXTRA_DIST		+= cudyn/cudyn.am cudyn/misc.doxy.sh

nobase_culibs_HEADERS = \
	$(cu_headers) \
	$(cuoo_headers) \
	$(cucon_headers) \
	$(cugra_headers) \
	$(cuflow_headers) \
	$(cuos_headers) \
	$(cutext_headers) \
	$(custo_headers) \
	$(cuex_headers) $(cuex_built_headers) \
	$(cudyn_headers)
nobase_nodist_culibs_HEADERS = \
	$(nodist_cu_headers)

check_programs = \
	$(cu_check_programs) \
	$(cuoo_check_programs) \
	$(cucon_check_programs) \
	$(cugra_check_programs) \
	$(cuflow_check_programs) \
	$(cuos_check_programs) \
	$(cutext_check_programs) \
	$(custo_check_programs) \
	$(cuex_check_programs) \
	$(cudyn_check_programs)

norun_check_programs = \
	$(cu_norun_check_programs) \
	$(cuoo_norun_check_programs) \
	$(cucon_norun_check_programs) \
	$(cuflow_norun_check_programs) \
	$(cuos_norun_check_programs) \
	$(cutext_norun_check_programs) \
	$(custo_norun_check_programs) \
	$(cuex_norun_check_programs) \
	$(cudyn_norun_check_programs)

doxyfiles = doc/main.doxy \
	$(cu_doxyfiles) \
	$(cuoo_doxyfiles) \
	$(cucon_doxyfiles) \
	$(cuex_doxyfiles)
built_doxyfiles = \
	$(cudyn_built_doxyfiles) \
	$(cuex_built_doxyfiles)

check_PROGRAMS = $(check_programs) $(norun_check_programs)
TESTS =		 $(check_programs)


# Build Infrastructure
# ====================

EXTRA_DIST += \
	$(aclocal_SCRIPTS) $(pkgconfig_DATA) \
	autotools.sh \
	culibs.spec.in culibs.spec \
	cubase.spec.in cubase.spec \
	mkskel skel tools migration \
	COPYING.Unicode

dist-hook:
	find $(distdir)/migration -name .svn -print0 | xargs -0 rm -rf

CLEANFILES += tmp.* config.sizeof

dist-rpm: dist-bzip2
	rpmbuild -tb $(PACKAGE)-$(VERSION).tar.bz2

all-local:
	ln -sf cubase-uninstalled.pc pkgconfig/cu-uninstalled.pc
	ln -sf cubase-uninstalled.pc pkgconfig/cucon-uninstalled.pc
	ln -sf cuex-uninstalled.pc pkgconfig/cudyn-uninstalled.pc
CLEANFILES += \
	pkgconfig/cu-uninstalled.pc \
	pkgconfig/cucon-uninstalled.pc \
	pkgconfig/cudyn-uninstalled.pc
install-data-local:
	$(mkinstalldirs) '$(DESTDIR)$(pkgconfigdir)'
if enable_cucon
	ln -sf cubase.pc '$(DESTDIR)$(pkgconfigdir)/cu.pc'
	ln -sf cubase.pc '$(DESTDIR)$(pkgconfigdir)/cucon.pc'
endif
if enable_cuex
	ln -sf cuex.pc '$(DESTDIR)$(pkgconfigdir)/cudyn.pc'
endif
uninstall-local:
if enable_cucon
	rm -f '$(DESTDIR)$(pkgconfigdir)/{cu,cucon}.pc'
endif
if enable_cuex
	rm -f '$(DESTDIR)$(pkgconfigdir)/cudyn.pc'
endif

$(tagsdir)/TAGS.culibs: $(DIST_SOURCES)
	@if [ -e $@ ]; then \
            echo "Updating tags $@ from $?"; \
            ctags -o "$@" -u $?; \
        else \
            echo "Creating new tags $@ from all sources"; \
            ctags -o "$@" $+; \
        fi
tags: $(tagsdir)/TAGS.culibs

check-export: tools/check-export.pl all
	$<

MANIFEST: Makefile
	@echo Writing $@.
	@rm -f $@; for f in $(DISTFILES); do echo $$f >>$@; done
list-unpacked: MANIFEST
	@$(srcdir)/tools/list-unpacked.pl

# Documentation
# =============

include doc/doc.am

# Phony-Declarations of Extra targets
#
.PHONY: check-export
