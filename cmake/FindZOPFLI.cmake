# This script locates the ZOPFLI library
# ------------------------------------

# define the list of search paths for headers and libraries
set(FIND_ZOPFLI_PATHS
    /usr/local
    /usr)

# find the ZOPFLI include directory
find_path(ZOPFLI_INCLUDE_DIR zopfli/zopfli.h
          PATH_SUFFIXES include
          PATHS ${FIND_ZOPFLI_PATHS})

set(ZOPFLI_FOUND FALSE)
if(ZOPFLI_INCLUDE_DIR)
    # release library
    find_library(ZOPFLI_LIBRARIES
                 NAMES zopfli
                 PATH_SUFFIXES lib64 lib
                 PATHS ${FIND_ZOPFLI_PATHS})
    if (ZOPFLI_LIBRARIES)
        set(ZOPFLI_FOUND TRUE)   
    endif()
endif()

# handle success
if(ZOPFLI_FOUND)
    message(STATUS "Found ZOPFLI in ${ZOPFLI_INCLUDE_DIR}")
endif()
