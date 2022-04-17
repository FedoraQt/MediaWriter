
include_directories(
    ${CMAKE_SOURCE_DIR}/src/lib
    ${CMAKE_SOURCE_DIR}/src/theme
    ${ADWAITAQT_INCLUDE_DIRS}
)

set(MEDIAWRITER_SRCS
    crashhandler.cpp
    downloadmanager.cpp
    drivemanager.cpp
    fakedrivemanager.cpp
    main.cpp
    notifications.cpp
    portalfiledialog.cpp
    releasemanager.cpp
    units.cpp
    utilities.cpp
    versionchecker.cpp
    # From theme
    ${CMAKE_SOURCE_DIR}/src/theme/icon.cpp
)

if (UNIX AND NOT APPLE)
    set (MEDIAWRITER_SRCS ${MEDIAWRITER_SRCS} linuxdrivemanager.cpp)
endif()

if (APPLE)
    set (MEDIAWRITER_SRCS ${MEDIAWRITER_SRCS} macdrivemanager.cpp macdrivearbiter.mm)
endif()

if (WIN32)
    add_definitions(-DUNICODE)
    set (MEDIAWRITER_SRCS ${MEDIAWRITER_SRCS} windrivemanager.cpp)
endif()

qt6_add_resources(MEDIAWRITER_SRCS assets.qrc qml.qrc ${CMAKE_SOURCE_DIR}/src/translations/translations.qrc)

if (WIN32)
    qt6_add_resources(MEDIAWRITER_SRCS windowsicon.qrc)
endif()

if (NOT APPLE)
    add_executable(mediawriter ${MEDIAWRITER_SRCS})
endif()

if (WIN32)
    set_target_properties(mediawriter PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if (WIN32)
    # HACK: we need to remove -std=c++11 definition for manifest file, otherwise
    # it will complain about undefined option "s" (as first letter from -Std=c++11)
    remove_definitions(-std=c++11)
    target_sources(mediawriter PRIVATE data/windows.rc)
endif()

if (APPLE)
    set(ICON mediawriter.icns)
    set(ICON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/data/icons/mediawriter.icns)
    add_executable(mediawriter MACOSX_BUNDLE ${MEDIAWRITER_SRCS} ${ICON_PATH})
    set_target_properties(mediawriter PROPERTIES
      OUTPUT_NAME "Fedora Media Writer"
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/data/Info.plist
      MACOSX_DEPLOYMENT_TARGET 10.14
      MACOSX_PACKAGE_LOCATION "Resources"
    )
    set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

target_link_libraries(mediawriter
    Qt6::Core
    Qt6::Network
    Qt6::QuickControls2
    Qt6::Widgets
    isomd5
)

if (UNIX AND NOT APPLE)
    target_link_libraries(mediawriter 
    Qt6::DBus
    ${ADWAITAQT_LIBRARIES}
)
endif()

if (WIN32)
    target_link_libraries(mediawriter dbghelp)
endif()

if (APPLE)
    target_link_libraries(mediawriter "-framework Foundation" "-framework DiskArbitration")
endif()

install(TARGETS mediawriter DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

if (UNIX AND NOT APPLE)
    install(DIRECTORY data/icons/hicolor/ DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor)
    install(FILES data/org.fedoraproject.MediaWriter.desktop DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications)
    install(FILES data/org.fedoraproject.MediaWriter.appdata.xml DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/metainfo)
endif()

if (APPLE AND CREATE_STANDALONE_MAC_BUNDLE)
    get_target_property(QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT_BIN_DIR}")
    add_custom_command(TARGET mediawriter POST_BUILD
                       COMMAND "${MACDEPLOYQT_EXECUTABLE}"
                               "${CMAKE_BINARY_DIR}/src/app/Fedora Media Writer.app"
                               -qmldir="${CMAKE_SOURCE_DIR}/src/app/qml"
                               -executable=\"${CMAKE_BINARY_DIR}/src/app/Fedora Media Writer.app/Contents/MacOS/helper\"
                        COMMENT "Deploying Qt...")
endif()
