
include_directories(
    ${CMAKE_SOURCE_DIR}/lib
)

add_definitions(-DUNICODE)

find_package(LibLZMA QUIET)

set(HELPER_SRCS
    main.cpp
    restorejob.cpp
    writejob.cpp
)

qt5_add_resources(HELPER_SRCS ${CMAKE_SOURCE_DIR}/translations/translations.qrc)

add_executable(helper ${HELPER_SRCS})

# HACK: we need to remove -std=c++11 definition for manifest file, otherwise
# it will complain about undefined option "s" (as first letter from -Std=c++11)
remove_definitions(-std=c++11)
target_sources(helper PRIVATE helper.exe.rc)

target_link_libraries(helper
    Qt5::Core
    isomd5
    ${LIBLZMA_LIBRARIES}
)

set_target_properties(helper PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/app)

install(TARGETS helper DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR}/mediawriter)
