set(_plugin_prefix "")
set(_plugin_suffix ".input")
set(_plugin_library_output "${MAPNIK_OUTPUT_DIR}/plugins/input")
set(_plugin_fpic ON)
set(_plugin_linkage MODULE)
set(_plugin_visibility PRIVATE)
if(NOT BUILD_SHARED_PLUGINS)
    set(_plugin_linkage INTERFACE)
    set(_plugin_visibility INTERFACE)
endif()

macro(add_plugin_target plugin_target output_name)
    add_library(${plugin_target} ${_plugin_linkage})
    set_target_properties(${plugin_target} PROPERTIES
        OUTPUT_NAME "${output_name}"
        POSITION_INDEPENDENT_CODE ${_plugin_fpic}
        PREFIX "${_plugin_prefix}"
        SUFFIX "${_plugin_suffix}"
        LIBRARY_OUTPUT_DIRECTORY "${_plugin_library_output}"
    )
    mapnik_install_plugin(${plugin_target})
endmacro()

# add a list with all build plugins so the copy dependencies command can wait for all build events
set(m_build_plugins "")
if(USE_PLUGIN_INPUT_CSV)
    add_subdirectory(csv)
    list(APPEND m_build_plugins input-csv)
endif()
if(USE_PLUGIN_INPUT_GDAL)
    add_subdirectory(gdal)
    list(APPEND m_build_plugins input-gdal)
endif()
if(USE_PLUGIN_INPUT_GEOBUF)
    add_subdirectory(geobuf)
    list(APPEND m_build_plugins input-geobuf)
endif()
if(USE_PLUGIN_INPUT_GEOJSON)
    add_subdirectory(geojson)
    list(APPEND m_build_plugins input-geojson)
endif()
if(USE_PLUGIN_INPUT_OGR)
    add_subdirectory(ogr)
    list(APPEND m_build_plugins input-ogr)
endif()
if(USE_PLUGIN_INPUT_PGRASTER)
    add_subdirectory(pgraster)
    list(APPEND m_build_plugins input-pgraster)
endif()
if(USE_PLUGIN_INPUT_POSTGIS)
    add_subdirectory(postgis)
    list(APPEND m_build_plugins input-postgis)
endif()
if(USE_PLUGIN_INPUT_RASTER)
    add_subdirectory(raster)
    list(APPEND m_build_plugins input-raster)
endif()
if(USE_PLUGIN_INPUT_SHAPE)
    add_subdirectory(shape)
    list(APPEND m_build_plugins input-shape)
endif()
if(USE_PLUGIN_INPUT_SQLITE)
    add_subdirectory(sqlite)
    list(APPEND m_build_plugins input-sqlite)
endif()
if(USE_PLUGIN_INPUT_TOPOJSON)
    add_subdirectory(topojson)
    list(APPEND m_build_plugins input-topojson)
endif()

#
# Copy all plugin dlls, so that these are in the main output dir, since cmake copies those into ${MAPNIK_OUTPUT_DIR}/plugins/input, too.
#
if(BUILD_SHARED_PLUGINS AND WIN32)
    list(LENGTH m_build_plugins m_number_plugins)
    if(m_number_plugins GREATER 0)
        string(CONFIGURE
            [[
                file(GLOB m_deps "./*.dll")
                message(STATUS "plugin deps: ${m_deps}")
                file(COPY ${m_deps} DESTINATION "@MAPNIK_OUTPUT_DIR@")
            ]] m_copy_dlls_str @ONLY
        )
        file (GENERATE OUTPUT "copy_dlls_$<CONFIG>.cmake"
            CONTENT "${m_copy_dlls_str}"
        )
        add_custom_target(copy_plugin_dependencies ALL
            COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/copy_dlls_$<CONFIG>.cmake"
            WORKING_DIRECTORY "${MAPNIK_OUTPUT_DIR}/plugins/input"
            COMMENT "copying plugin dlls into build dir"
        )
        add_dependencies(copy_plugin_dependencies ${m_build_plugins})
    endif()
endif()
