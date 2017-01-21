cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)

project(QSHOW)
 
find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(SDL_gfx REQUIRED)

include_directories(${SDL_INCLUDE_DIR})
include_directories(${SDL_IMAGE_INCLUDE_DIR})
include_directories(${SDL_GFX_INCLUDE_DIR})

# set source files
file(GLOB QSHOW_SRC ${QSHOW_SOURCE_DIR}/src/*.cpp ${QSHOW_SOURCE_DIR}/src/*.h)
include_directories(${QSHOW_SOURCE_DIR}/src)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QSHOW_SOURCE_DIR}/bin/)

add_executable(qshow ${QSHOW_SRC})
target_link_libraries(qshow ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY} ${SDLGFX_LIBRARY})

