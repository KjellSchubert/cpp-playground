" from https://github.com/BastianBerndsen/myDotFiles/blob/master/.vimrc
" Also see https://github.com/gmarik/Vundle.vim

set nocompatible              " be iMproved, required
filetype off                  " required

" set the runtime path to include Vundle and initialize
set rtp+=~/.vim/bundle/Vundle.vim
call vundle#begin()
" alternatively, pass a path where Vundle should install plugins
"call vundle#begin('~/some/path/here')

" let Vundle manage Vundle, required
Plugin 'gmarik/Vundle.vim'

" Python-mode
Plugin 'git://github.com/klen/python-mode.git'

" All of your Plugins must be added before the following line
call vundle#end()            " required
filetype plugin on    " required (removed indent here)


autocmd FileType * set tabstop=2|set shiftwidth=2|set noexpandtab
autocmd FileType python set tabstop=2|set shiftwidth=2|set expandtab
au BufEnter *.py set ai sw=2 ts=2 sta et fo=croql
au BufEnter *.cpp set ai sw=2 ts=2 sta et fo=croql
