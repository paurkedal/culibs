Editor Support
==============

This directory contains editor support files, currently Vim syntax files for
otab files and culibs extensions for the C syntax.

Vim Setup
---------

To add the culibs extensions to C mode, put the following line in
``~/.vim/after/syntax/c.vim``. ::

    runtime! syntax/culibs.vim

To enable the otab syntax, put ::

    au BufNewFile,BudRead *.otab setl ft=otab

in your ``.vimrc`` file.
