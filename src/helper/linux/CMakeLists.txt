
find_package(LibLZMA REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/lib
    ${LIBLZMA_INCLUDE_DIRS}
)

set(HELPER_SRCS
    main.cpp
    restorejob.cpp
    writejob.cpp
)

qt5_add_resources(HELPER_SRCS ${CMAKE_SOURCE_DIR}/src/translations/translations.qrc)

add_executable(helper ${HELPER_SRCS})
target_link_libraries(helper
    Qt5::Core
    Qt5::DBus
    isomd5
    ${LIBLZMA_LIBRARIES}
)

install(TARGETS helper DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR}/mediawriter)
