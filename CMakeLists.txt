cmake_minimum_required(VERSION 2.6)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "No BuildType set, forcing Release")
endif(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -g -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O2 -march=native -ffast-math -finline-functions -fno-rtti -std=c++11")
if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        set(FILESYSTEM_LIBRARY "-lstdc++fs")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)

project(qShow)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${SDL2_IMAGE_INCLUDE_DIR})

# set source files
file(GLOB QSHOW_SRC ${QSHOW_SOURCE_DIR}/src/*.cpp ${QSHOW_SOURCE_DIR}/src/*.h)
include_directories(${QSHOW_SOURCE_DIR}/src)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QSHOW_SOURCE_DIR}/bin/)

add_executable(qshow ${QSHOW_SRC})
target_link_libraries(qshow ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${FILESYSTEM_LIBRARY})

