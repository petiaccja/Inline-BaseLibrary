# INLINE BASELIBRARY
project(InlineLib)

cmake_minimum_required(VERSION 3.15.0)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MP /await /permissive-")
endif()

include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/dependencies/include")
include_directories("${CMAKE_SOURCE_DIR}/externals/include")

add_subdirectory(src)
add_subdirectory(test)