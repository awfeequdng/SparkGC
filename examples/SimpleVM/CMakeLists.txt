cmake_minimum_required(VERSION 3.12)
project(SimpleVM)

include(CheckIncludeFiles)
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)

#### Determine C++11
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if (COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif (COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else ()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif ()
set(CMAKE_CXX_STANDARD 11)

#### Determine C11
check_c_compiler_flag("-std=c11" COMPILER_SUPPORTS_C11)
if (COMPILER_SUPPORTS_CXX11)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
else ()
    message(STATUS "The compiler ${CMAKE_C_COMPILER} has no C11 support. Please use a different C compiler.")
endif ()
set(CMAKE_C_STANDARD 11)

if (DEFINED ENV{SPARK_DEBUG})
    add_definitions(-DSPARK_DEBUG)
endif ()


#### SimpleVM
include_directories(../../include)
link_directories(../../cmake-build-debug)

add_executable(SimpleVM main.cpp)
target_link_libraries(SimpleVM SparkGC)
