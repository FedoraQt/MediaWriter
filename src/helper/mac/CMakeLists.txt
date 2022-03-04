
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

qt6_add_resources(HELPER_SRCS ${CMAKE_SOURCE_DIR}/src/translations/translations.qrc)

add_executable(helper ${HELPER_SRCS})
target_link_libraries(helper
    Qt6::Core
    Qt6::DBus
    isomd5
    ${LIBLZMA_LIBRARIES}
)

set_target_properties(helper PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/app/Fedora Media Writer.app/Contents/MacOS/")

install(TARGETS helper DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR}/mediawriter)
