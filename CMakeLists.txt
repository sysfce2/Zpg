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
    set(cxx_flags "${CMAKE_CXX_FLAGS} -Wall")
elseif (TARGET_COMPILER STREQUAL "cl")
    set(cxx_flags "${CMAKE_CXX_FLAGS} /W4 /WX /EHsc")
endif()

set(CMAKE_CXX_FLAGS "${cxx_flags}"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)
set(CMAKE_C_FLAGS   "${cxx_flags}"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)


# REQUIRED LIBRARIES
find_package(ZLIB REQUIRED)


# INCLUDES
set(LIB_INCLUDES ${ZLIB_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/src)
include_directories(${LIB_INCLUDES})


# LIBRARY
add_library(${LIB_NAME} STATIC  src/LibZpg.cpp)


# TOOLS
add_executable(zpg_packer tools/zpg_packer.cpp)
target_link_libraries(zpg_packer ${LIB_NAME} ${ZLIB_LIBRARIES})


# INSTALL
install(DIRECTORY src/ DESTINATION ${BUILD_DIR}/include
        FILES_MATCHING  PATTERN "*.hpp")
install(TARGETS ${LIB_NAME} ARCHIVE DESTINATION ${BUILD_DIR}/lib${TARGET_BITS})

