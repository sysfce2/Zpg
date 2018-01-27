cmake_minimum_required(VERSION 2.8)

# GLOBAL PARAMS
set(LIB_NAME "Zpg")


# THE PROJECT
cmake_policy(SET CMP0054 NEW)
project("Zpg Lib")


# MODULES
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


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
find_package(ZLIB)
find_package(ZOPFLI)


# EXTERNAL IF REQUIRED
add_subdirectory(external/ libs/)


# INCLUDES
set(LIB_INCLUDES ${ZLIB_INCLUDE_DIR} ${ZOPFLI_INCLUDE_DIR} src/)
include_directories(${LIB_INCLUDES})

# LIBRARY STATIC
set(LIB_DEPS ${ZLIB_LIBRARY} ${ZOPFLI_LIBRARY})
add_library(${LIB_NAME} STATIC  src/Zpg.cpp)
target_link_libraries(${LIB_NAME} ${LIB_DEPS})

# TOOLS
add_executable(zpg_packer EXCLUDE_FROM_ALL tools/zpg_packer.cpp)
target_link_libraries(zpg_packer ${LIB_NAME} ${LIB_DEPS})


# INSTALL
install(DIRECTORY src/ DESTINATION include/${LIB_NAME}
        FILES_MATCHING  PATTERN "*.hpp")
install(TARGETS ${LIB_NAME} ARCHIVE DESTINATION lib/)

