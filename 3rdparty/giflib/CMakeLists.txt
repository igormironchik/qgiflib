
cmake_minimum_required( VERSION 3.19 )

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE "Release"
		CACHE STRING "Choose the type of build."
		FORCE)
endif( NOT CMAKE_BUILD_TYPE )

project( giflib )

set( SRC gif_hash.h
	gif_lib.h
	gif_lib_private.h
	dgif_lib.c
	egif_lib.c
	gif_hash.c
	gif_err.c
	gif_font.c
	gifalloc.c
	openbsd-reallocarray.c )

add_library( giflib ${SRC} )
