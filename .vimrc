
syntax on               " syntax highlighting
filetype indent on      " activates indenting for files
set backspace=2         " backspace in insert mode works like normal editor
set smarttab
set shiftwidth=4
set tabstop=4
set colorcolumn=80
set expandtab
set listchars=trail:-
set autoindent          " auto indenting
set number              " line numbers

"colorscheme habamax
" colorscheme lunaperche
" set background=dark
colorscheme slate

" makefile edit
autocmd FileType make,changelog set nosmarttab noexpandtab shiftwidth=0 tabstop=4
autocmd BufWritePre *.c,*.h :%s/\s\+$//e 
au BufRead,BufNewFile *.inc set filetype=c
" secure config recommandé pour l'utilisation de ansible-vault
set nobackup            
set noswapfile
set nowritebackup
set viminfo=
set clipboard=
set formatoptions+=ro

" [i show var def
" [d show macro def
" [{ prev {
" [( prev (
" [} next }
" [) next )
" H top of window
" CTRL-V select lines, I (insert), va inserrer au debut des lignes
" :checkpath , voir les headers non trouvés
" :make, :cl (voir erreurs), :cn (next err), :cp (prev err)
" set foldmethod=syntax
" set foldexpr=MyFoldExpr(v:lnum)
" fun! MyFoldExpr(line)
"     let str = getline(a:line)
"     if str =~ '^sub\>'
"         return '1'
"     elseif str =~ '^}'
"         return '<1'
"     else
"         return foldlevel(a:line - 1)
"     endif
" endfun
map <F4>  [I:let nr = input("Which one: ")<Bar>exe "normal " .. nr .. "[\t"<CR>

"set guifont=DejaVu\ Sans\ Mono\ 11
set noguipty
set guifont=DejaVu\ Sans\ Mono\ 11
set ch=2
set mousehide
set guioptions=acdemgrLtR
map <S-Insert> <MiddleMouse>
map! <S-Insert> <MiddleMouse>

