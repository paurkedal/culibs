Garbage Collected C Libraries
=============================

For elaborate documentation, see http://www.eideticdew.org/culibs/.

This source code contains several C libraries.  It is split into modules, where
each module roughly corresponds to one library:

======  ===========
Module  Description
======  ===========
cu	Common infrastructure.
cucon	Containers and algorithms.
cugra	Graphs and graph algorithms.
cutext  Unicode text and stream support.
cufo    Text formatting library for logging and other basic text generation.
cuos	Interface to the operating system.
cuex	Expression trees and related algorithms.
cudyn	Dynamically typed objects.  Coupled with cuex.
======  ===========


Compiling and Installing
========================

In addition to a C and POSIX library with librt support, you need:

* The Boehm-Demers-Weiser Conservative Garbage Collector
* libatomic_ops
* libffi (optional)
* buddy (optional)

Optionally, the following may be used

* libunwind (for debugging stack trace)

In the simplest case, ::

    ./configure [OPTIONS]
    make
    make install
    make install-doc  # Optional, installs same docs as on above URL

See ``INSTALL`` for more detailed information about Autoconf based
installation.


Using the Libraries
===================

These libraries use ``pkg-config``.

If you use Autoconf, add ::

    PKG_CHECK_MODULES(CULIBS, modules)

to your configure.ac, where ``modules`` is the desired set of modules to link
against.  Then refer to ``$CULIBS_CFLAGS`` and ``$CULIBS_LIBS`` in your
``Makefile.in`` or ``Makefile.am``.

You can also build directly from the command line with ::

    gcc -c `pkg-config --cflags modules` my_prog.c	        # compile
    gcc -o my_prog my_prog.o `pkg-config --libs modules`        # link
