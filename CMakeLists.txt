cmake_minimum_required(VERSION 3.12)
project(SparkGC)

#### cmake setup
enable_testing()
set(CMAKE_MODULE_PATH "${CMCMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")

include(cmake/helper.cmake)
require_cxx_11()
require_c_11()

if (DEFINED ENV{SPARK_DEBUG_ALL})
    add_definitions(-DSPARK_DEBUG)
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
        include/spark/CollectedObject.h
        src/spark/CollectedHeap.cpp
        include/spark/ColorBitmap.h
        src/spark/SparkGC.cpp
        src/spark/SparkMutator.cpp
        include/spark/SparkMutator.h
        src/spark/CollectedObject.cpp
        include/spark/SparkGC_All.h)

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
add_test_target(color-bit)
add_test_target(align)
add_test_target(gc-allocate)

#### examples: SimpleVM
add_executable(example_SimpleVM
        examples/SimpleVM/main.cpp)
target_link_libraries(example_SimpleVM SparkGC)

#### examples: TinyVM
add_executable(example_TinyVM
        examples/tiny-vm/include/vm/allocator.hpp
        examples/tiny-vm/include/vm/core.hpp
        examples/tiny-vm/include/vm/exception.hpp
        examples/tiny-vm/include/vm/instructions.hpp
        examples/tiny-vm/include/vm/memory.hpp
        examples/tiny-vm/include/vm/vm.hpp
        examples/tiny-vm/include/vm/bytestream.hpp
        examples/tiny-vm/test/main.cpp)

target_include_directories(example_TinyVM
        PRIVATE
        examples/tiny-vm/include)
target_link_libraries(example_TinyVM SparkGC)