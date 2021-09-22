include(CMakePackageConfigHelpers)

# set the cmake targets install location
set(INCLUDE_INSTALL_DIR include/)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
get_property(MAPNIK_UTILITIES GLOBAL PROPERTY MAPNIK_UTILITIES)
list(JOIN MAPNIK_DEPENDENCIES "\n" MAPNIK_DEPENDENCIES)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/mapnikConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfig.cmake"
    INSTALL_DESTINATION ${MAPNIK_CMAKE_DIR}
    PATH_VARS INCLUDE_INSTALL_DIR PLUGINS_INSTALL_DIR FONTS_INSTALL_DIR MAPNIK_DEPENDENCIES MAPNIK_UTILITIES
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfigVersion.cmake"
    DESTINATION ${MAPNIK_CMAKE_DIR}
)

install(
    FILES 
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindCairo.cmake" 
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindWebP.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindHarfBuzz.cmake"
    DESTINATION ${MAPNIK_CMAKE_DIR}/Modules
)

install(EXPORT MapnikTargets
    DESTINATION ${MAPNIK_CMAKE_DIR}
    FILE mapnikTargets.cmake
    NAMESPACE mapnik::
)
