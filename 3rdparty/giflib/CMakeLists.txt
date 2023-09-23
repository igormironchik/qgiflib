
cmake_minimum_required( VERSION 3.19 )

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE "Release"
		CACHE STRING "Choose the type of build."
		FORCE)
endif( NOT CMAKE_BUILD_TYPE )

project( giflib )

file( GLOB SRC_C *.c )
file( GLOB SRC_H *.h )

add_library( giflib ${SRC_C} ${SRC_H} )
