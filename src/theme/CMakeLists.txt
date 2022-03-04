
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
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    ${ADWAITAQT_LIBRARIES}
)

install(TARGETS adwaitathemeplugin DESTINATION ${QT6_INSTALL_QML}/org/fedoraproject/AdwaitaTheme)
install(FILES qmldir DESTINATION ${QT6_INSTALL_QML}/org/fedoraproject/AdwaitaTheme)
install(DIRECTORY qml/ DESTINATION ${QT6_INSTALL_QML}/QtQuick/Controls/org/fedoraproject/AdwaitaTheme)
