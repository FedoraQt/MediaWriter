
include_directories(${ADWAITAQT_INCLUDE_DIRS})

set(ADWAITA_THEME_PLUGIN_SRCS
    adwaitathemeplugin.cpp
    icon.cpp
    theme.cpp
)

if (WIN32)
    add_library(adwaitathemeplugin MODULE ${ADWAITA_THEME_PLUGIN_SRCS})
    set_target_properties(adwaitathemeplugin PROPERTIES PREFIX "")
else()
    add_library(adwaitathemeplugin SHARED ${ADWAITA_THEME_PLUGIN_SRCS})
endif()

target_link_libraries(adwaitathemeplugin
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Widgets
    ${ADWAITAQT_LIBRARIES}
)


install(TARGETS adwaitathemeplugin DESTINATION ${QT_INSTALL_QML}/org/fedoraproject/AdwaitaTheme)
install(FILES qmldir DESTINATION ${QT_INSTALL_QML}/org/fedoraproject/AdwaitaTheme)
install(DIRECTORY qml/ DESTINATION ${QT_INSTALL_QML}/QtQuick/Controls.2/org.fedoraproject.AdwaitaTheme)
