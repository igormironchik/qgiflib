cmake_minimum_required( VERSION 3.19 )

project(qgiflib)

if(NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE "Release"
        CACHE STRING "Choose the type of build."
        FORCE)
endif(NOT CMAKE_BUILD_TYPE)

option(BUILD_QGIFLIB_TESTS "Build tests? Default ON." ON)

add_subdirectory(3rdparty/giflib)
add_subdirectory(src)

if(BUILD_QGIFLIB_TESTS)
    add_subdirectory(test)
endif()
