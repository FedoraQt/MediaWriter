set(LIBWMI_SRCS
    libwmi.cpp
)

add_library(libwmi STATIC ${LIBWMI_SRCS})

target_link_libraries(libwmi
    Qt6::Core
)
