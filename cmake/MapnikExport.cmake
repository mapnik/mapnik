include(CMakePackageConfigHelpers)

# export mapnik configuration
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfigVersion.cmake"
    VERSION ${MAPNIK_VERSION}
    COMPATIBILITY ExactVersion
)
get_property(MAPNIK_UTILITIES GLOBAL PROPERTY MAPNIK_UTILITIES)
list(JOIN MAPNIK_DEPENDENCIES "\n" MAPNIK_DEPENDENCIES)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/mapnikConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfig.cmake"
    INSTALL_DESTINATION ${MAPNIK_CMAKE_DIR}
    PATH_VARS MAPNIK_INCLUDE_DIR PLUGINS_INSTALL_DIR FONTS_INSTALL_DIR MAPNIK_DEPENDENCIES MAPNIK_UTILITIES
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfigVersion.cmake"
    DESTINATION ${MAPNIK_CMAKE_DIR}
)

# install our modules, so that the expected target names are found. 
install(
    FILES 
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindCairo.cmake" 
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindWebP.cmake"
    DESTINATION ${MAPNIK_CMAKE_DIR}/Modules
)

install(EXPORT MapnikTargets
    DESTINATION ${MAPNIK_CMAKE_DIR}
    FILE mapnikTargets.cmake
    NAMESPACE mapnik::
)


# Create configuration dependend files for the plugin install dirs.
# some package managers are using different paths per configuration.
string(TOLOWER "${CMAKE_BUILD_TYPE}" _build_type)
string(TOUPPER "${CMAKE_BUILD_TYPE}" _build_type_l)
set(_mapnik_plugin_file_name "mapnikPlugins-${_build_type}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${_mapnik_plugin_file_name}.cmake.in "set(MAPNIK_PLUGINS_DIR_${_build_type_l} \"@PACKAGE_PLUGINS_INSTALL_DIR@\" CACHE STRING \"\")\n")
include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${_mapnik_plugin_file_name}.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${_mapnik_plugin_file_name}.cmake
  PATH_VARS PLUGINS_INSTALL_DIR
  INSTALL_DESTINATION ${MAPNIK_CMAKE_DIR}
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${_mapnik_plugin_file_name}.cmake
  DESTINATION ${MAPNIK_CMAKE_DIR}
)
