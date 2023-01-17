include(CMakePackageConfigHelpers)

### exports mapnik cmake config files (mapnikConfigVersion and mapnikConfig)
function(mapnik_export_cmake_config)
    # export mapnik configuration
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfigVersion.cmake"
        VERSION ${MAPNIK_VERSION}
        COMPATIBILITY ExactVersion
    )
    get_property(MAPNIK_UTILITIES GLOBAL PROPERTY MAPNIK_UTILITIES)

    # generate all find_dependency and pkg_config calls
    set(mapnik_find_deps)
    foreach(dep IN LISTS mapnik_deps)
        set(ver_comment "# ${dep} used with version ${mapnik_${dep}_version}")
        set(mapnik_find_deps "${mapnik_find_deps}\n${ver_comment}\n")
        if(mapnik_${dep}_find_args)
            list(REMOVE_DUPLICATES mapnik_${dep}_find_args)
            list(JOIN mapnik_${dep}_find_args " " m_args_joined)
            set(mapnik_find_deps "${mapnik_find_deps}find_dependency(${dep} ${m_args_joined})")
        else()
            list(JOIN mapnik_${dep}_pkg_args " " m_args_joined)
            set(mapnik_find_deps "${mapnik_find_deps}pkg_check_modules(${dep} ${m_args_joined})")
        endif()
    endforeach()

    configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/mapnikConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfig.cmake"
        INSTALL_DESTINATION ${MAPNIK_CMAKE_DIR}
        PATH_VARS MAPNIK_INCLUDE_DIR PLUGINS_INSTALL_DIR FONTS_INSTALL_DIR mapnik_find_deps MAPNIK_UTILITIES
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )
    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfigVersion.cmake"
        DESTINATION ${MAPNIK_CMAKE_DIR}
    )
endfunction()


mapnik_export_cmake_config()

install(EXPORT MapnikTargets
    DESTINATION ${MAPNIK_CMAKE_DIR}
    FILE mapnikTargets.cmake
    NAMESPACE mapnik::
)

### install plugin cmake config files ###
# Create configuration dependend files for the plugin install dirs.
# some package managers are using different paths per configuration.
string(TOLOWER "${CMAKE_BUILD_TYPE}" _build_type)
string(TOUPPER "${CMAKE_BUILD_TYPE}" _build_type_l)
set(m_mapnik_plugin_file_name mapnikPlugins-${_build_type})
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${m_mapnik_plugin_file_name}.cmake.in" "set(MAPNIK_PLUGINS_DIR_${_build_type_l} \"@PACKAGE_PLUGINS_INSTALL_DIR@\" CACHE STRING \"\")\n")
include(CMakePackageConfigHelpers)
configure_package_config_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${m_mapnik_plugin_file_name}.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${m_mapnik_plugin_file_name}.cmake"
  PATH_VARS PLUGINS_INSTALL_DIR
  INSTALL_DESTINATION ${MAPNIK_CMAKE_DIR}
)
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/${m_mapnik_plugin_file_name}.cmake"
  DESTINATION ${MAPNIK_CMAKE_DIR}
)
