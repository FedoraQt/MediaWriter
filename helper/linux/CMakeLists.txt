
include_directories(
    ${CMAKE_SOURCE_DIR}/lib
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(lzma REQUIRED IMPORTED_TARGET liblzma)

set(HELPER_SRCS
    main.cpp
    restorejob.cpp
    writejob.cpp
)

qt5_add_resources(HELPER_SRCS ${CMAKE_SOURCE_DIR}/translations/translations.qrc)

add_executable(helper ${HELPER_SRCS})
target_link_libraries(helper
    Qt5::Core
    Qt5::DBus
    isomd5
    lzma
)

install(TARGETS helper DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR}/mediawriter)
