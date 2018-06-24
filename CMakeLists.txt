cmake_minimum_required(VERSION 2.8)

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No BuildType set, forcing Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug, Release." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_VERBOSE_MAKEFILE ON)
if (MSVC)
	message(STATUS "Found MSVC: " ${CMAKE_CXX_COMPILER})
	set(CMAKE_CXX_FLAGS "/nologo /D_CRT_SECURE_NO_WARNINGS /std:c++11")
    set(CMAKE_CXX_FLAGS_DEBUG "/W3 /Zi /Od /D_DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/Ox /fp:fast /DNDEBUG")
else()
	message(STATUS "Found C++ Compiler: " ${CMAKE_CXX_COMPILER})
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -O0 -g")
	set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O2 -march=native -ffast-math -finline-functions")
endif()

if (NOT MSVC)
	set(FILESYSTEM_LIBRARY "-lstdc++fs")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)

project(QSHOW)

find_package(SDL2 REQUIRED)
find_package(FreeImage REQUIRED)

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${FREEIMAGE_INCLUDE_DIRS})

file(GLOB QSHOW_SRC ${QSHOW_SOURCE_DIR}/src/*.cpp ${QSHOW_SOURCE_DIR}/src/*.h)
include_directories(${QSHOW_SOURCE_DIR}/src)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QSHOW_SOURCE_DIR}/bin/)

add_executable(qshow ${QSHOW_SRC})
target_link_libraries(qshow ${SDL2_LIBRARY} ${FREEIMAGE_LIBRARIES} ${FILESYSTEM_LIBRARY})
