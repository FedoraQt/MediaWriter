
include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/theme
)

set(MEDIAWRITER_SRCS
    crashhandler.cpp
    downloadmanager.cpp
    drivemanager.cpp
    fakedrivemanager.cpp
    main.cpp
    notifications.cpp
    releasemanager.cpp
    units.cpp
    utilities.cpp
    versionchecker.cpp
    # From theme
    ${CMAKE_SOURCE_DIR}/theme/icon.cpp
)

if (UNIX)
    set (MEDIAWRITER_SRCS ${MEDIAWRITER_SRCS} linuxdrivemanager.cpp)
endif()

if (APPLE)
    #set (MEDIAWRITER_SRCS ${MEDIAWRITER_SRCS} macdrivemanager.cpp)
endif()

if (WIN32)
    add_definitions(-DUNICODE)
    set (MEDIAWRITER_SRCS ${MEDIAWRITER_SRCS} windrivemanager.cpp)
endif()

qt5_add_resources(MEDIAWRITER_SRCS assets.qrc qml.qrc ${CMAKE_SOURCE_DIR}/translations/translations.qrc)

if (WIN32)
    qt5_add_resources(MEDIAWRITER_SRCS windowsicon.qrc)
endif()

add_executable(mediawriter ${MEDIAWRITER_SRCS})

if (WIN32)
    # HACK: we need to remove -std=c++11 definition for manifest file, otherwise
    # it will complain about undefined option "s" (as first letter from -Std=c++11)
    remove_definitions(-std=c++11)
    target_sources(mediawriter PRIVATE data/windows.rc)
endif()

target_link_libraries(mediawriter
    Qt5::Core
    Qt5::Network
    Qt5::QuickControls2
    Qt5::Widgets
    ${ADWAITAQT_LIBRARIES}
    isomd5
)

if (UNIX)
    target_link_libraries(mediawriter Qt5::DBus)
endif()

if (WIN32)
    target_link_libraries(mediawriter dbghelp)
endif()

install(TARGETS mediawriter DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

if (UNIX)
    install(DIRECTORY data/icons/hicolor/ DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor)
    install(FILES data/org.fedoraproject.MediaWriter.desktop DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications)
    install(FILES data/org.fedoraproject.MediaWriter.appdata.xml DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/metainfo)
endif()

#macx {
    #TARGET = "Fedora Media Writer"
    #HEADERS += macdrivemanager.h \
                #macdrivearbiter.h
    #SOURCES += macdrivemanager.cpp
    #OBJECTIVE_SOURCES += macdrivearbiter.mm

    #QMAKE_LFLAGS += -F/System/Library/Frameworks
    #LIBS += -framework Foundation
    #LIBS += -framework DiskArbitration
    #QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

    #QMAKE_INFO_PLIST = Info.plist
    #ICON = assets/icon/mediawriter.icns

    #MY_ENTITLEMENTS.name = CODE_SIGN_ENTITLEMENTS
    #MY_ENTITLEMENTS.value = "$$top_srcdir/app/Entitlements.plist"
    #QMAKE_MAC_XCODE_SETTINGS += MY_ENTITLEMENTS

    #QMAKE_POST_LINK += sed -i -e "s/@MEDIAWRITER_VERSION_SHORT@/$$MEDIAWRITER_VERSION_SHORT/g" \"./$${TARGET}.app/Contents/Info.plist\";
    #QMAKE_POST_LINK += sed -i -e "s/@MEDIAWRITER_VERSION@/$$MEDIAWRITER_VERSION/g" \"./$${TARGET}.app/Contents/Info.plist\";
#}

