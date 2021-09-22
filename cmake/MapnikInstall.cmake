function(mapnik_install)
    set(options ALREADY_INSTALLED IS_PLUGIN)
    set(oneValueArgs TARGET)
    set(multiValueArgs)
    cmake_parse_arguments(MAPNIK_INSTALL "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT MAPNIK_INSTALL_ALREADY_INSTALLED AND NOT MAPNIK_INSTALL_IS_PLUGIN)
        install(TARGETS ${MAPNIK_INSTALL_TARGET}
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
    elseif(NOT MAPNIK_INSTALL_ALREADY_INSTALLED AND MAPNIK_INSTALL_IS_PLUGIN)
        install(TARGETS ${MAPNIK_INSTALL_TARGET}
            RUNTIME DESTINATION ${PLUGINS_INSTALL_DIR}
                COMPONENT MapnikPluginRuntime
            LIBRARY DESTINATION ${PLUGINS_INSTALL_DIR}
                COMPONENT MapnikPluginRuntime
                NAMELINK_COMPONENT MapnikPluginDevelopment
            ARCHIVE DESTINATION ${PLUGINS_INSTALL_DIR}
                COMPONENT MapnikPluginDevelopment
        )
    endif()
    if(NOT MAPNIK_INSTALL_IS_PLUGIN)
        get_target_property(TARGET_TYPE "${MAPNIK_INSTALL_TARGET}" TYPE)
        if (TARGET_TYPE STREQUAL "EXECUTABLE")
            get_property(MAPNIK_INSTALLED_TARGETS GLOBAL PROPERTY TARGETS)
            list(APPEND MAPNIK_INSTALLED_TARGETS ${MAPNIK_INSTALL_TARGET})
            set_property(GLOBAL PROPERTY TARGETS ${MAPNIK_INSTALLED_TARGETS})
        endif()
    else()
        get_property(MAPNIK_INSTALLED_PLUGINS GLOBAL PROPERTY PLUGINS)
        list(APPEND MAPNIK_INSTALLED_PLUGINS ${MAPNIK_INSTALL_TARGET})
        set_property(GLOBAL PROPERTY PLUGINS ${MAPNIK_INSTALLED_PLUGINS})
    endif()
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
