cmake_minimum_required(VERSION 2.8)

# GLOBAL PARAMS
set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/build"
    CACHE PATH "The path to the external libraries.")
set(LIB_NAME "Zpg")


# THE PROJECT
project("ZLib Pack for Games")


# DETECT SYSTEM PARAMS
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(TARGET_BITS "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(TARGET_BITS "32")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(TARGET_COMPILER "gcc")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(TARGET_COMPILER "clang")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(TARGET_COMPILER "cl")
endif()

# CONFIGURE FLAGS
if (TARGET_COMPILER STREQUAL "gcc" OR
    TARGET_COMPILER STREQUAL "clang")
    set(cxx_flags "-std=c++0x -Wall")
elseif (TARGET_COMPILER STREQUAL "cl")
    set(cxx_flags "/W4 /WX /EHsc")
endif()

set(CMAKE_CXX_FLAGS "${cxx_flags}"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)
set(CMAKE_C_FLAGS   "${cxx_flags}"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)


find_package(ZLIB REQUIRED)


# INCLUDES
set(LIB_INCLUDES ${ZLIB_INCLUDE_DIRS})
include_directories(${LIB_INCLUDES})


# LIBRARY
add_library(${LIB_NAME} STATIC  src/LibZpg.cpp)

# TOOLS
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
add_executable(zpg_packer tools/zpg_packer.cpp)
target_link_libraries(zpg_packer ${LIB_NAME} ${ZLIB_LIBRARIES})

# INSTALL
file(GLOB LIB_PUBLIC_HEADERS "include/*.hpp")
install(FILES ${LIB_PUBLIC_HEADERS} DESTINATION ${BUILD_DIR}/include)
install(TARGETS ${LIB_NAME} ARCHIVE DESTINATION ${BUILD_DIR}/lib${TARGET_BITS})

