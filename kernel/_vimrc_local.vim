" Loaded by [Local_Vimrc]{https://github.com/LucHermitte/local_vimrc}

" Syntastic -- Check C++ files with the right compiler
if exists(':SyntasticCheck')
	let g:syntastic_cpp_compiler="arm-none-eabi-g++"
	let g:syntastic_cpp_compiler_options.=" -DHW_PI_1B "
	SyntasticCheck
endif
