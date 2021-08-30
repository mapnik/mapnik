if(BUILD_DEMO_VIEWER)
    message(STATUS "Building demo viewer")
    add_subdirectory(viewer)
endif()

if(BUILD_DEMO_CPP)
    message(STATUS "Building c++ demo app")
    add_subdirectory(c++)
endif()
