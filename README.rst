Garbage Collected C Libraries
=============================

**Status:** *This project is no longer actively maintained by the author.*

For elaborate documentation, see http://www.eideticdew.org/culibs/.

This source code contains several C libraries.  It is split into modules, where
each module roughly corresponds to one library:

======= ======================================================  ==========
Module  Description                                             Library
======= ======================================================  ==========
cu_     Common infrastructure.                                  libcubase
cucon_  Containers, including lists, stacks, queues, sets,      libcubase
        and maps tailored for various purposes.
cuoo_   Dynamically typed objects and prerequisites for cuex.   libcubase
cuos_   Interface to the operating system, path manipulation,   libcuos
        XDG base directory support, etc.
cutext_ Unicode text and stream support.                        libcutext
cufo_   Text formatting for logging and program output.         libcufo
cugra_  Graphs and graph algorithms.                            libcugra
cuex_   Expression trees and related algorithms.                libcuex
cudyn_  Dynamically typed objects to supplement cuex.           libcuex
======= ======================================================  ==========


Compiling and Installing
========================

In addition to a C and POSIX library with librt support, you need:

* The Boehm-Demers-Weiser Conservative Garbage Collector, available from
  http://www.hpl.hp.com/personal/Hans_Boehm/gc/.
* libatomic_ops, which is now shipped with the garbage collector.

Optionally, the following may be used

* libffi - Enables a few extra features in cuex.
* buddy - Used in cugra for minimum feedback vertex set algorithm.
* libunwind - Used to print stack trace after a crash.

For a system wide install, ::

    ./configure
    make
    sudo make install

To install to your home directory, you may use ::

    ./configure --prefix=$HOME/local
    make
    make install

adjusting the prefix to your taste.  See INSTALL_ and ``./configure --help``
for more detailed information about options to ``configure``, make targets and
other Autoconf related information.

Optionally, you may install the API documentation with ::

    make install-doc


Building Individual Components
------------------------------

If you don't need the whole set of libraries or want to build them separately,
you can pass the list of modules to build with the ``--enable-modules``
option, e.g. to build only the base and OS libraries::

    ./configure --enable-modules="cu cucon cuos"

If the listed modules depend on unlisted ones, ``configure`` will look for
installed ones to fulfill the dependencies.  Modules which are part of the
same library are selected if one of the modules are listed.  Either of "cu",
"cucon", or "cubase" will select the base library.

For the precise module dependencies, look at the ``CUAC_MODULE`` invocations
in configure.ac_, where the second argument lists the dependencies.


Using the Libraries
===================

The libraries come with a collection of ``pkg-config`` files, one for each
module.  You can use these either under Autoconf or directly with the
``pkg-config`` command.

If you your project uses Autoconf, add ::

    PKG_CHECK_MODULES([CULIBS], [MODULES])

to your ``configure.ac``, where ``MODULES`` is the space-separated list of
modules that you want to use.  You can now refer to ``$(CULIBS_CFLAGS)`` and
``$(CULIBS_LIBS)`` from your ``Makefile.in`` or ``Makefile.am``.

To build from the command line or under non-Autoconf build files, use the
``pkg-config`` command ::

    gcc -c `pkg-config --cflags MODULES` my_prog.c              # compile
    gcc -o my_prog my_prog.o `pkg-config --libs MODULES`        # link



.. Stop reading here.  The rest is for the documentation system.
.. _cu: cu/
.. _cucon: cucon/
.. _cuoo: cuoo/
.. _cuos: cuos/
.. _cutext: cutext/
.. _cufo: cufo/
.. _cugra: cugra/
.. _cuex: cuex/
.. _cudyn: cudyn/
.. _INSTALL: INSTALL
.. _configure.ac: configure.ac
