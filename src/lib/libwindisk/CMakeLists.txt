set(LIBWINDISK_SRCS
    windisk.cpp
)

add_library(libwindisk STATIC ${LIBWINDISK_SRCS})

target_link_libraries(libwindisk
    Qt6::Core
)
