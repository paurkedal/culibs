" Vim syntax file
" Language: C culibs extension
" Maintainer: Petter Urkedal
" URL: http://www.eideticdew.org/culibs/

" This file must be loaded after the main syntax.  In particular, for yacc,
" loading it from "~/vim/after/syntax/c.vim" will be too early, since this is
" loaded before the yacc definitions complete.  Instead use something like
"
"     au Syntax c,yacc ru! syntax/culibs.vim
"
" If these definitions cause any problems, try to disable the greying out of
" debug statemests with
"
"     let c_no_debug = 1


" Declarations
syn keyword cStorageClass CU_SINLINE
syn keyword cStatement CU_BEGIN_DECLARATIONS CU_END_DECLARATIONS
syn keyword cStatement CU_END_BOILERPLATE CU_DOXY_AID
syn keyword cComment CU_DOXY_FAKED
syn match cComment 'CU_DOXY_ENDFAKED([^()]*)'
syn match cPreProc /\<cuPP_\w\+/
syn keyword cStorageClass CU_ATTR_NORETURN CU_ATTR_PURE CU_ATTR_CONST
syn keyword cStorageClass CU_ATTR_UNUSED CU_ATTR_DEPRECATED CU_ATTR_MALLOC
syn keyword cPreproc CU_THREADLOCAL_DECL CU_THREADLOCAL_DEF
syn keyword cPreproc CU_THREADLOCAL_INIT

" Types
syn match cType '\<cu\(con\|dyn\|ex\|flow\|fo\|gra\|oo\|os\|sto\|text\)\?P\?_\i*_t\>'
syn keyword cType cuex_t

" Control Flow
syn keyword cStatement cu_expect cu_expect_true cu_expect_false

" New Elementary Types
syn keyword cConstant cu_true cu_false

" Closures
syn keyword cType cu_clop cu_clop0 cu_prot cu_prot0
syn keyword cStatement cu_clop_def cu_clop_def0 cu_clop_edef cu_clop_edef0
syn keyword cStatement cu_clos_def cu_clos_dec cu_clos_fun
syn keyword cStatement cu_clos_edec cu_clos_efun
syn keyword cStatement cu_clof_decl cu_clof_decl0 cu_clof_fun cu_clof_fun0
syn keyword cType cu_clos_self cu_clof_self
syn keyword cStructure cu_clof_decl cu_clof_decl0
syn keyword cOperator cu_clos_ref cu_clos_ref_cast cu_clof_ref
syn keyword cStatement cu_call cu_call0

" Objects, Classes, Hash-Consing
syn keyword cStorageClass CUOO_OBJ CUOO_HCOBJ
syn keyword cStatement cu_inherit
syn keyword cType cudyn_hctem_decl

" Expressions
syn match cConstant '\<CUEX_O[0-9]M\?_[A-Z0-9_]\+'

" Grey-out debugging statements so one can focus on the essentials.
if !exists("c_no_debug")
    syn cluster cParenGroup add=cDebugParen,cDebugString
    syn cluster cMultiGroup add=cDebugParen,cDebugString
    syn cluster cPreProcGroup add=cDebugParen,cDebugString
    syn cluster yaccActionGroup add=cDebugParen,cDebugString
    syn match cDebug '\<cu_debug_\i*' nextgroup=cDebugParen
    syn keyword cDebug cu_dlogf cu_dlog_def cu_dlog_edec cu_dlog_edef
	\ nextgroup=cDebugParen
    syn keyword cDebug cu_dprintf nextgroup=cDebugParen
    syn keyword cDebug CU_NOINIT nextgroup=cDebugParen
    syn region cDebugParen contained start='(' end=');' contains=cDebugString
    syn region cDebugString contained start='"' skip='\\.' end='"'
endif
hi def link cDebugParen cDebug
hi def link cDebugString cDebug

" Obsolete
syn match cObsolete 'cu_func_\(decl\|init\)\(_e\)\?'
syn keyword cObsolete cu_func_ref cu_func_ref_cast
syn keyword cObsolete cu_clptr cu_clptr0 cu_proto cu_proto0

" Newly introduced highlighting groups.
hi def link cDebug Debug
hi def link cObsolete Error
