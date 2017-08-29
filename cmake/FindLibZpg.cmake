# This script locates the LibZpg library
# ------------------------------------

# define the list of search paths for headers and libraries
set(FIND_LibZpg_PATHS
    /usr/local
    /usr)

# find the LibZpg include directory
find_path(LibZpg_INCLUDE_DIR Zpg/LibZpg.hpp
          PATH_SUFFIXES include
          PATHS ${FIND_LibZpg_PATHS})

set(LibZpg_FOUND FALSE)
if(LibZpg_INCLUDE_DIR)
    # release library
    find_library(LibZpg_LIBRARIES
                 NAMES Zpg
                 PATH_SUFFIXES lib64 lib
                 PATHS ${FIND_LibZpg_PATHS})
    if (LibZpg_LIBRARIES)
        set(LibZpg_FOUND TRUE)   
    endif()
endif()

# Search Dependencies (From FindSFML)
# start with an empty list
set(LibZpg_DEPENDENCIES)
set(FIND_LibZpg_DEPENDENCIES_NOTFOUND)

# macro that searches for a 3rd-party library
macro(find_zpg_dependency output friendlyname)
    # No lookup in environment variables (PATH on Windows), as they may contain wrong library versions
    find_library(${output} NAMES ${ARGN} PATHS ${FIND_LibZpg_PATHS} PATH_SUFFIXES lib NO_SYSTEM_ENVIRONMENT_PATH)
    if(${${output}} STREQUAL "${output}-NOTFOUND")
        unset(output)
        set(FIND_LibZpg_DEPENDENCIES_NOTFOUND "${FIND_LibZpg_DEPENDENCIES_NOTFOUND} ${friendlyname}")
    endif()
endmacro()

# find libraries
find_zpg_dependency(ZOPFLI_LIBRARY "libzopfli" zopfli)

# update the list
set(LibZpg_DEPENDENCIES ${ZOPFLI_LIBRARY} "-lz")

# handle success
if(LibZpg_FOUND)
    message(STATUS "Found LibZpg in ${LibZpg_INCLUDE_DIR}")
endif()
