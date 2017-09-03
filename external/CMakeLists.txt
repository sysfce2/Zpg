# zopfli
if (NOT(ZOPFLI_FOUND))
    add_library(zopfli STATIC   zopfli/blocksplitter.c
                                zopfli/cache.c
                                zopfli/deflate.c
                                zopfli/gzip_container.c
                                zopfli/hash.c
                                zopfli/katajainen.c
                                zopfli/lz77.c
                                zopfli/squeeze.c
                                zopfli/tree.c
                                zopfli/util.c
                                zopfli/zlib_container.c
                                zopfli/zopfli_lib.c)
    set(ZOPFLI_INCLUDE_DIR external/ CACHE INTERNAL ZOPFLI_INCLUDE_DIR)
    set(ZOPFLI_LIBRARIES zopfli CACHE INTERNAL ZOPFLI_LIBRARIES)
endif()
