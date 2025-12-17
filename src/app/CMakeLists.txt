
include_directories(
    ${CMAKE_SOURCE_DIR}/src/lib
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
    utilities.cpp
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
    # NOTE: QProcess::start() can not start a new process with UAC level
    # requireAdministrator, since it is using CreateProcess. There is one
    # API that can trigger the UAC prompt, ShellExecute or ShellExecuteEx,
    # but process started by them can not be managed by QProcess.
    # There are some workarounds but since the QProcess instance is connected
    # to various slots, it would require a rewrite to apply the workarounds.
    # We simply specify the main app to require administrator privilege in
    # the manifest.
    set_target_properties(mediawriter PROPERTIES WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
endif()

if (WIN32)
    # HACK: we need to remove -std=c++11 definition for manifest file, otherwise
    # it will complain about undefined option "s" (as first letter from -Std=c++11)
    remove_definitions(-std=c++11)
    # target_sources(mediawriter PRIVATE data/windows.rc)
endif()

if (APPLE)
    set(ICON mediawriter.icns)
    set(ICON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/data/icons/mediawriter.icns)
    add_executable(mediawriter MACOSX_BUNDLE ${MEDIAWRITER_SRCS} ${ICON_PATH})
    set_target_properties(mediawriter PROPERTIES
      OUTPUT_NAME BazziteMediaWriter
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_BUNDLE_NAME BazziteMediaWriter
      MACOSX_BUNDLE_BUNDLE_VERSION ${MEDIAWRITER_VERSION}
      MACOSX_BUNDLE_SHORT_VERSION_STRING ${MEDIAWRITER_VERSION}
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/data/Info.plist
      MACOSX_PACKAGE_LOCATION "Resources"
    )
    set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

target_link_libraries(mediawriter
    Qt6::Core
    Qt6::Network
    Qt6::Svg # Needed for macdeployqt to pick svg plugin
    Qt6::QuickControls2
    Qt6::Widgets
    isomd5
)

if (UNIX AND NOT APPLE)
    target_link_libraries(mediawriter 
    Qt6::DBus
)
endif()

if (WIN32)
    target_link_libraries(mediawriter dbghelp libwindisk)
endif()

if (APPLE)
    target_link_libraries(mediawriter "-framework Foundation" "-framework DiskArbitration")
endif()


install(TARGETS mediawriter
    DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
)

if (UNIX AND NOT APPLE)
    install(DIRECTORY data/icons/hicolor/ DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor)
    install(FILES data/org.bazzite.MediaWriter.desktop DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications)
    install(FILES data/org.bazzite.MediaWriter.metainfo.xml DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/metainfo)
endif()
