cmake_minimum_required(VERSION 3.3)
project(lazy C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Za")
endif()

add_executable(lazy_test main.cpp)
set_target_properties(lazy_test PROPERTIES OUTPUT_NAME "lazy_test")
set_target_properties(lazy_test PROPERTIES PREFIX "")