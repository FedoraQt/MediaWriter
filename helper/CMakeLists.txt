
if (UNIX)
    add_subdirectory(linux)
endif()

#if (APPLE)
    #add_subdirectory(mac)
#endif()

if (WIN32)
    add_subdirectory(win)
endif()

