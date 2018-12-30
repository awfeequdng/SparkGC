cmake_minimum_required(VERSION 3.12)
project(SparkGC)

#### cmake setup
include(CheckIncludeFiles)
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
enable_testing()
set(CMAKE_MODULE_PATH "${CMCMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")


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

if (DEFINED ENV{SPARK_DEBUG_ALL})
    add_definitions(-DSPARK_DEBUG)
    add_definitions(-DSPARK_INTERPRETER_DEBUG)
    add_definitions(-DSPARK_CLASSPATH_DEBUG)
endif ()

#### Determine arch
if (CMAKE_SYSTEM_PROCESSOR MATCHES "[xX]86_64|(amd|AMD)64")
    set(SPARK_ARCH_x86_64 1)
    set(ARCH_NAME x86_64)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "[xX]86|i.86")
    set(SPARK_ARCH_x86 1)
    set(ARCH_NAME x86)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm(v.*)?")
    set(SPARK_ARCH_arm 1)
    set(ARCH_NAME arm)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "(AARCH|aarch)64")
    set(SPARK_ARCH_aarch64 1)
    set(ARCH_NAME aarch64)
endif ()

aux_source_directory(include/spark/arch/${ARCH_NAME} SPARK_ARCH_SRC)
message(STATUS "System arch: ${ARCH_NAME}")

#### apply configurations
add_definitions("-DHAVE_CONFIG_H")
configure_file(${CMAKE_SOURCE_DIR}/cmake/cmake-config.h.in ${CMAKE_BINARY_DIR}/compileTimeConfig.h)

#### library setup
include_directories(${CMAKE_BINARY_DIR})
include_directories(include)

set(SPARK_GC_LIBRARY_SRC
        include/spark/SparkGC_Shared.h
        include/spark/SparkGC.h
        include/spark/CollectedHeap.h
        include/spark/CollectedObject.h src/spark/CollectedHeap.cpp include/spark/ColorBitmap.h)

add_library(SparkGC
        ${SPARK_GC_LIBRARY_SRC}
        ${SPARK_ARCH_SRC})

#### ctest setup
macro(add_test_target name)
    add_executable(test-${name} test/test-${name})
    target_link_libraries(test-${name} SparkGC)
    add_test(${name} test-${name})
endmacro()

add_test_target(create-block-tree)
add_test_target(allocate-large-super)
add_test_target(allocate-small-medium)
