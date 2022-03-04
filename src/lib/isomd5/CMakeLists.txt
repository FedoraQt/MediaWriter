
# This is not valid for C files
remove_definitions(-std=c++17)

set(ISOMD5_SRCS
    libcheckisomd5.c
    md5.c
)

add_library(isomd5 STATIC ${ISOMD5_SRCS})
