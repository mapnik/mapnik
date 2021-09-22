function(mapnik_install _target)
    install(TARGETS ${_target}
        EXPORT MapnikTargets
        INCLUDES DESTINATION ${MAPNIK_INCLUDE_DIR}
        RUNTIME DESTINATION ${MAPNIK_BIN_DIR}
            COMPONENT MapnikRuntime
        LIBRARY DESTINATION ${MAPNIK_LIB_DIR}
            COMPONENT MapnikRuntime
            NAMELINK_COMPONENT MapnikDevelopment
        ARCHIVE DESTINATION ${MAPNIK_ARCHIVE_DIR}
            COMPONENT MapnikDevelopment
    )
    get_target_property(TARGET_TYPE "${_target}" TYPE)
    if (TARGET_TYPE STREQUAL "EXECUTABLE")
        get_property(MAPNIK_INSTALLED_TARGETS GLOBAL PROPERTY TARGETS)
        list(APPEND MAPNIK_INSTALLED_TARGETS ${_target})
        set_property(GLOBAL PROPERTY TARGETS ${MAPNIK_INSTALLED_TARGETS})
    endif()
endfunction()

function(mapnik_install_plugin _target)
    install(TARGETS ${_target}
        RUNTIME DESTINATION ${PLUGINS_INSTALL_DIR}
            COMPONENT MapnikPluginRuntime
        LIBRARY DESTINATION ${PLUGINS_INSTALL_DIR}
            COMPONENT MapnikPluginRuntime
            NAMELINK_COMPONENT MapnikPluginDevelopment
        ARCHIVE DESTINATION ${PLUGINS_INSTALL_DIR}
            COMPONENT MapnikPluginDevelopment
    )
    get_property(MAPNIK_INSTALLED_PLUGINS GLOBAL PROPERTY PLUGINS)
    list(APPEND MAPNIK_INSTALLED_PLUGINS ${_target})
    set_property(GLOBAL PROPERTY PLUGINS ${MAPNIK_INSTALLED_PLUGINS})
endfunction()


function(mapnik_install_targets)
    if(INSTALL_DEPENDENCIES AND WIN32)
        # https://cmake.org/cmake/help/latest/policy/CMP0087.html
        cmake_policy(SET CMP0087 NEW)
        get_property(MAPNIK_INSTALLED_TARGETS GLOBAL PROPERTY TARGETS)
        get_property(MAPNIK_INSTALLED_PLUGINS GLOBAL PROPERTY PLUGINS)
        set(INTERNAL_TARGETS "")
        set(INTERNAL_PLUGINS "")

        foreach(_target IN LISTS MAPNIK_INSTALLED_TARGETS)
            list(APPEND INTERNAL_TARGETS "${CMAKE_INSTALL_PREFIX}/${MAPNIK_BIN_DIR}/$<TARGET_FILE_NAME:${_target}>")
        endforeach()
        foreach(_target IN LISTS MAPNIK_INSTALLED_PLUGINS)
            list(APPEND INTERNAL_PLUGINS "${CMAKE_INSTALL_PREFIX}/${PLUGINS_INSTALL_DIR}/$<TARGET_FILE_NAME:${_target}>")
        endforeach()
        # all other executables get auto detected and fixed.
        list(GET INTERNAL_TARGETS 0 INTERNAL_TARGETS)

        INSTALL(CODE "
            message(STATUS \"${INTERNAL_TARGETS}\")
            message(STATUS \"${INTERNAL_PLUGINS}\")
            
            include(BundleUtilities)
            fixup_bundle(\"${INTERNAL_TARGETS}\" \"${INTERNAL_PLUGINS}\" \"${ADDITIONAL_LIBARIES_PATHS}\")
        " COMPONENT MapnikRuntime)
    endif()

endfunction()
