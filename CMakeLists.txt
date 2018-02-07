cmake_minimum_required(VERSION 2.8)

# GLOBAL PARAMS
set(LIB_NAME "Zpg")


# THE PROJECT
if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()
project("Zpg")


# MODULES
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


# DETECT SYSTEM PARAMS
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(TARGET_BITS "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(TARGET_BITS "32")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(TARGET_OS "windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(TARGET_OS "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(TARGET_OS "mac")
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
    set(cxx_flags "${CMAKE_CXX_FLAGS} /W4 /WX")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS) #strncpy
endif()

set(CMAKE_CXX_FLAGS "${cxx_flags}"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)
set(CMAKE_C_FLAGS   "${cxx_flags}"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)


# REQUIRED LIBRARIES
find_package(ZLIB)


# INCLUDES
set(LIB_INCLUDES ${ZLIB_INCLUDE_DIR} ${ZOPFLI_INCLUDE_DIR} src/)
include_directories(${LIB_INCLUDES})

# LIBRARY STATIC
set(LIB_DEPS ${ZLIB_LIBRARY})
add_library(${LIB_NAME} STATIC src/Zpg.cpp)
target_link_libraries(${LIB_NAME} ${LIB_DEPS})

# TOOLS
add_executable(zpg_packer EXCLUDE_FROM_ALL tools/zpg_packer.cpp)
target_link_libraries(zpg_packer ${LIB_NAME} ${LIB_DEPS})
if (TARGET_OS STREQUAL "windows")
	add_custom_command(TARGET zpg_packer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${ZLIB_INCLUDE_DIR}/../bin/zlib.dll ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endif()

# INSTALL
install(DIRECTORY src/ DESTINATION include/${LIB_NAME} COMPONENT library
        FILES_MATCHING  PATTERN "*.hpp")
install(TARGETS ${LIB_NAME} ARCHIVE DESTINATION lib/ COMPONENT library)
install(TARGETS zpg_packer RUNTIME DESTINATION bin/ COMPONENT binaries)
if (TARGET_OS STREQUAL "windows")
	install(FILES ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/zlib.dll DESTINATION bin/ COMPONENT binaries)
endif()

