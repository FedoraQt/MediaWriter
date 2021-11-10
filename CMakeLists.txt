project(MediaWriter)

cmake_minimum_required(VERSION 3.17)

set(MEDIAWRITER_MAJOR_VERSION "5")
set(MEDIAWRITER_MINOR_VERSION "0")
# MICRO_VERSION >= 50 means a development version
set(MEDIAWRITER_MICRO_VERSION "50")
set(MEDIAWRITER_VERSION "${MEDIAWRITER_MAJOR_VERSION}.${MEDIAWRITER_MINOR_VERSION}.${MEDIAWRITER_MICRO_VERSION}")

set(QT_MIN_VERSION "6.2.0")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_AUTOMOC ON)

if (WIN32)
    cmake_policy(SET CMP0020 NEW) # to use winmain to avoid opening a console window when starting
endif()

include(GNUInstallDirs)
include(FeatureSummary)


if (${MEDIAWRITER_MICRO_VERSION} GREATER_EQUAL 50)
    execute_process(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE MEDIAWRITER_LAST_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE)
    add_definitions(-DMEDIAWRITER_VERSION="${MEDIAWRITER_VERSION}-${MEDIAWRITER_LAST_COMMIT}")
else()
    add_definitions(-DMEDIAWRITER_VERSION="${MEDIAWRITER_VERSION}")
endif()

if (UNIX AND NOT APPLE)
    # FIXME: this is not ideal
    add_definitions(-DLIBEXECDIR="${CMAKE_INSTALL_FULL_LIBEXECDIR}/mediawriter")
endif()

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED Core DBus Gui Network Qml Quick QuickControls2 Widgets)

find_package(AdwaitaQt "1.4.50" REQUIRED)


# Installation paths
get_target_property(REAL_QMAKE_EXECUTABLE Qt6::qmake
                    IMPORTED_LOCATION)

if (NOT QT_INSTALL_QML)
    execute_process(COMMAND "${REAL_QMAKE_EXECUTABLE}" -query QT_INSTALL_QML
                    OUTPUT_VARIABLE QT_INSTALL_QML
                    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
