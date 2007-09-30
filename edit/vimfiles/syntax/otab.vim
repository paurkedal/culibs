" Vim syntax file
" Language: libcuex otab (specification language for operator codes)
" Created By: Petter Urkedal
" Maintainer: Petter Urkedal

if exists("b:current_syntax")
    finish
endif

syn match otabComment /#.*/ contains=otabCommentTodo
syn keyword otabCommentTodo TODO FIXME XXX

syn keyword otabStatement import provide option
syn keyword otabStatement reserve range skipwhite nextgroup=otabRange
syn keyword otabStatement attr implicit
syn match otabStatement /\<o\/\([0-9]\+\|r\)c\?\>/
syn match otabRange contained /\w\+/ skipwhite nextgroup=otabRangeEq
syn match otabRangeEq contained /=/ skipwhite nextgroup=otabRange
hi link otabRangeEq otabOperator

syn match otabOperator /:/ skipwhite nextgroup=otabRange,otabNumber
syn match otabOperator /[=.!+\-*\/^]/

syn region otabParen matchgroup=otabOperator transparent start=/(/ end=/)/ fold
syn region otabBracket matchgroup=otabOperator transparent start=/\[/ end=/\]/ fold
syn match otabError /[\])]/

syn match otabNumber /\<[0-9]\+\>/
syn match otabNumber /\<0x\x\+/
syn keyword otabUnit bit bits
syn region otabString start=/"/ skip=/\\./ end=/"/

hi def link otabStatement Statement
hi def link otabOperator Operator
hi def link otabNumber Number
hi def link otabComment Comment
hi def link otabCommentTodo Todo
hi def link otabUnit Preproc
hi def link otabString String
hi def link otabError Error
hi def link otabRange Type
