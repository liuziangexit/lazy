cmake_minimum_required(VERSION 2.6)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

project(lazy)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-error=unused-parameter -Wno-error=unused-variable")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /EHsc")
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
CHECK_CXX_COMPILER_FLAG("-std=c++1z" COMPILER_SUPPORTS_CXX1z)
if(COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif(COMPILER_SUPPORTS_CXX1z)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. compile might be fail.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}")

# Disable Visual Studio Microsoft language extensions
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Za")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /Za")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRCS
	${PROJECT_SOURCE_DIR}/main.cpp)

source_group(Sources FILES ${SRCS})

include_directories("$(PROJECT_SOURCE_DIR)/../include")

add_executable(lazy_test ${SRCS})

enable_testing()

add_test(LazyTest lazy_test)

set_tests_properties(LazyTest
    PROPERTIES PASS_REGULAR_EXPRESSION "GREAT SUCCESS!")
    
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
