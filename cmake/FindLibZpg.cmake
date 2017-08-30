# This script locates the LibZpg library
# ------------------------------------

# define the list of search paths for headers and libraries
set(FIND_ZPG_PATHS
    /usr/local
    /usr)

# find the LibZpg include directory
find_path(ZPG_INCLUDE_DIR Zpg/Zpg.hpp
          PATH_SUFFIXES include
          PATHS ${FIND_ZPG_PATHS})

set(ZPG_FOUND FALSE)
if(ZPG_INCLUDE_DIR)
    # release library
    find_library(ZPG_LIBRARIES
                 NAMES Zpg
                 PATH_SUFFIXES lib64 lib
                 PATHS ${FIND_ZPG_PATHS})
    if (ZPG_LIBRARIES)
        set(ZPG_FOUND TRUE)   
    endif()
endif()

# Search Dependencies (From FindSFML)
# start with an empty list
set(FIND_ZPG_DEPENDENCIES_NOTFOUND)

# macro that searches for a 3rd-party library
macro(find_zpg_dependency output friendlyname)
    # No lookup in environment variables (PATH on Windows), as they may contain wrong library versions
    find_library(${output} NAMES ${ARGN} PATHS ${FIND_ZPG_PATHS} PATH_SUFFIXES lib NO_SYSTEM_ENVIRONMENT_PATH)
    if(${${output}} STREQUAL "${output}-NOTFOUND")
        unset(output)
        set(FIND_ZPG_DEPENDENCIES_NOTFOUND "${FIND_ZPG_DEPENDENCIES_NOTFOUND} ${friendlyname}")
    endif()
endmacro()

# find libraries
find_zpg_dependency(ZOPFLI_LIBRARY "libzopfli" zopfli)

# update the list
set(ZPG_DEPENDENCIES ${ZOPFLI_LIBRARY} "-lz")

# handle success
if(ZPG_FOUND)
    message(STATUS "Found LibZpg in ${ZPG_INCLUDE_DIR}")
endif()
