
cmake_minimum_required( VERSION 3.19 )

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE "Release"
		CACHE STRING "Choose the type of build."
		FORCE)
endif( NOT CMAKE_BUILD_TYPE )

project( qgiflib )

set( CMAKE_CXX_STANDARD 17 )

set( CMAKE_CXX_STANDARD_REQUIRED ON )

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

find_package( Qt6 REQUIRED COMPONENTS Core Gui )

add_library( qgiflib qgiflib.hpp qgiflib.cpp )

target_link_libraries( qgiflib giflib Qt6::Gui Qt6::Core )
