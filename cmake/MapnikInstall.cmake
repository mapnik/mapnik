#
# Install library targets that consuming users need.
#
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
    if (TARGET_TYPE STREQUAL "SHARED_LIBRARY")
        set_property(GLOBAL APPEND PROPERTY TARGETS ${_target})
    endif()
endfunction()

#
# Install plugins
#
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
    set_property(GLOBAL APPEND PROPERTY PLUGINS ${_target})
endfunction()

#
# Install executables. These are available via COMPONENTS in find_package
#
function(mapnik_install_utility _target)
    set(_target_name "mapnikUtilityTargets_${_target}")
    install(TARGETS ${_target}
        EXPORT ${_target_name}
        INCLUDES DESTINATION ${MAPNIK_INCLUDE_DIR}
        RUNTIME DESTINATION ${MAPNIK_BIN_DIR}
            COMPONENT MapnikRuntime
        LIBRARY DESTINATION ${MAPNIK_LIB_DIR}
            COMPONENT MapnikRuntime
            NAMELINK_COMPONENT MapnikDevelopment
        ARCHIVE DESTINATION ${MAPNIK_ARCHIVE_DIR}
            COMPONENT MapnikDevelopment
    )
    install(EXPORT ${_target_name}
        FILE ${_target_name}.cmake
        NAMESPACE mapnik::
        DESTINATION ${MAPNIK_CMAKE_DIR}
    )
    set_property(GLOBAL APPEND PROPERTY MAPNIK_UTILITIES ${_target})
endfunction()

function(mapnik_install_targets)
    if(INSTALL_DEPENDENCIES AND WIN32)
        # https://cmake.org/cmake/help/latest/policy/CMP0087.html
        cmake_policy(SET CMP0087 NEW)
        get_property(_installed_utilities GLOBAL PROPERTY MAPNIK_UTILITIES)
        get_property(_installed_targets GLOBAL PROPERTY TARGETS)
        get_property(_installed_plugins GLOBAL PROPERTY PLUGINS)
        set(_internal_executables "")
        set(_internal_libraries "")

        foreach(_target IN LISTS _installed_utilities)
            list(APPEND _internal_executables "\${CMAKE_INSTALL_PREFIX}/${MAPNIK_BIN_DIR}/$<TARGET_FILE_NAME:${_target}>")
        endforeach()
        foreach(_target IN LISTS _installed_targets)
            list(APPEND _internal_libraries "\${CMAKE_INSTALL_PREFIX}/${MAPNIK_BIN_DIR}/$<TARGET_FILE_NAME:${_target}>")
        endforeach()
        foreach(_target IN LISTS _installed_plugins)
            list(APPEND _internal_libraries "\${CMAKE_INSTALL_PREFIX}/${PLUGINS_INSTALL_DIR}/$<TARGET_FILE_NAME:${_target}>")
        endforeach()
        # all other executables get auto detected and fixed.
        if(_internal_executables)
            list(GET _internal_executables 0 _internal_executables)
        endif()

        INSTALL(CODE "
            message(STATUS \"internal_executables: ${_internal_executables}\")
            message(STATUS \"internal_libraries: ${_internal_libraries}\")
            message(STATUS \"ADDITIONAL_LIBARIES_PATHS: ${ADDITIONAL_LIBARIES_PATHS}\")
            
            include(BundleUtilities)
            fixup_bundle(\"${_internal_executables}\" \"${_internal_libraries}\" \"${ADDITIONAL_LIBARIES_PATHS}\")
        " COMPONENT MapnikRuntime)
    endif()
endfunction()
