" Vim syntax file
" Language: SIAL (Super Instruction Assembly Language)
" Maintainer: Nakul Jindal
" Latest Revision: 19 December 2013
" Followed this guide : http://vim.wikia.com/wiki/Creating_your_own_syntax_files


if exists("b:current_syntax")
      finish
endif

syn case ignore

" Comments 
syn keyword sialTodo contained TODO FIXME XXX NOTE VFL 
syn match sialComment "#.*$" contains=sialTodo

" Declarations
syn match sialRangeDef contained '[a-zA-Z_0-9]\+\s*,\s*[a-zA-Z_0-9]\+'

syn keyword sialModifier sip_consistent predefined persistent scoped_extent contiguous auto_allocate

syn keyword sialDec temp local distributed served aoindex moindex moaindex mobindex index laindex special  scalar static import proc endproc

syn region sialRangeAssign start='\s*=\s*[a-zA-Z_0-9]\+\s*,\s*[a-zA-Z_0-9]\+' end="$" contains=sialRangeDef

" Statements & Super Instructions
syn keyword sialStatement return call in where exit cycle sial endsial do enddo pardo endpardo if endif else

syn keyword sialBarrier sip_barrier server_barrier

syn keyword sialSuperInstruction allocate deallocate create delete put get prepare request collective destroy create delete println print print_index print_scalar execute gpu_on gpu_allocate gpu_free gpu_put gpu_get gpu_off set_persistent restore_persistent

" Blocks in Sial
syn match sialSuperBlock '[a-zA-Z_0-9]\+\s*(\(\s*[a-zA-Z_0-9]\+\s*,\)\+[a-zA-Z_0-9]\+)'



" Floating point (From http://golang.org/misc/vim/syntax/go.vim?m=text)
syn match       sialFloat             "\<\d\+\.\d*\([Ee][-+]\d\+\)\?\>"
syn match       sialFloat             "\<\.\d\+\([Ee][-+]\d\+\)\?\>"
syn match       sialFloat             "\<\d\+[Ee][-+]\d\+\>"

"Strings and their contents
syn region      sialString            start=+"+ end=+"+

syn region sialSource start="sial" end="endsial" transparent contains=ALL

"syn region sialProcDec start="proc" end="endproc" fold transparent contains=sialStatement, sialSuperInstruction, sialComment, sialSuperBlock, sialBarrier
syn region sialProcDec start="proc" end="endproc" fold transparent contains=ALL

"syn region sialDo start="do" end="enddo" fold transparent contains=sialStatement, sialSuperInstruction, sialComment

"syn region sialPardo start="pardo" end="endpardo" fold transparent contains=sialStatement, sialSuperInstruction, sialComment

"syn region sialIf start="if" end="endif" fold transparent contains=sialStatement, sialSuperInstruction, sialComment



hi def link sialTodo                Todo
hi def link sialComment             Comment
hi def link sialModifier            PreProc
hi def link sialDec                 Type
hi def link sialProcDec             Type
"hi def link sialDo                  Type
"hi def link sialPardo               Type
"hi def link sialIf                  Type
hi def link sialStatement           Statement
hi def link sialSuperInstruction    Special
hi def link sialFloat               Float
hi def link sialString              String
hi def link sialRangeDef            Constant
hi def link sialSuperBlock          Identifier
hi def link sialBarrier             Underlined


" There's a bug in the implementation of grouphere. For now, use the
" following as a more expensive/less precise workaround.
syn sync minlines=500

let b:current_syntax = "sial"



