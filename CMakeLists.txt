cmake_minimum_required(VERSION 3.15)
# 3.15 is required since the Boost::XXXX targets was first added. https://cmake.org/cmake/help/latest/module/FindBoost.html#imported-targets
# 3.14 is required since SQLite3 Module was first added. https://cmake.org/cmake/help/latest/module/FindSQLite3.html#findsqlite3

include(cmake/GetVersion.cmake)
get_mapnik_version()

project(mapnik 
    VERSION ${MAPNIK_MAJOR_VERSION}.${MAPNIK_MINOR_VERSION}.${MAPNIK_PATCH_VERSION}
    HOMEPAGE_URL "https://mapnik.org/" 
    DESCRIPTION "Mapnik is an open source toolkit for developing mapping applications" 
    LANGUAGES CXX 
)
message(STATUS "mapnik version: ${PROJECT_VERSION}")

# https://cliutils.gitlab.io/modern-cmake/chapters/features/ides.html
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(MapnikFindPackage)
include(MapnikCopyDependencies)
include(MapnikInstall)

set(ADDITIONAL_LIBARIES_PATHS "" CACHE STRING "only used on windows. Pass directories containing the dlls that are missing. You can ignore this, if the build (verify_app step) runs successfully")
option(COPY_LIBRARIES_FOR_EXECUTABLES "copies required shared libaries (only windows) to the executable build directory" ON)
option(COPY_FONTS_AND_PLUGINS_FOR_EXECUTABLES "copies required plugins and fonts into the executable build directory" ON)
option(INSTALL_DEPENDENCIES "if ON, all dependencies (eg. required dlls) will be copied into CMAKE_INSTALL_PREFIX/MAPNIK_BIN_DIR." ON)

option(BUILD_SHARED_LIBS "build mapnik dynamic(ON) or static(OFF)" ON)
option(BUILD_TEST "builds the tests" ON)
option(USE_EXTERNAL_MAPBOX_GEOMETRY "Use a external mapnik/geometry.hpp. If off, use the submodule" OFF)
option(USE_EXTERNAL_MAPBOX_POLYLABEL "Use a external mapnik/polylabel. If off, use the submodule" OFF)
option(USE_EXTERNAL_MAPBOX_PROTOZERO "Use a external mapnik/protozero. If off, use the submodule" OFF)
option(USE_EXTERNAL_MAPBOX_VARIANT "Use a external mapnik/variant. If off, use the submodule" OFF)
option(USE_JPEG "adds jpeg support" ON)
option(USE_PNG "adds png support" ON)
option(USE_TIFF "adds tiff support" ON)
option(USE_WEBP "adds webp support" ON)
option(USE_LIBXML2 "adds libxml2 support" ON)
option(USE_CAIRO "adds the cairo renderer" ON)
option(USE_PROJ "adds proj support" ON)
option(USE_GRID_RENDERER "adds grid renderer" ON)
option(USE_SVG_RENDERER "adds svg renderer" ON)
option(USE_BIGINT "uses 64 bit instead of 32" ON)
option(USE_MEMORY_MAPPED_FILE "uses file cache" ON)

option(USE_PLUGIN_INPUT_CSV "adds plugin input csv" ON)
option(USE_PLUGIN_INPUT_GDAL "adds plugin input gdal" ON)
option(USE_PLUGIN_INPUT_GEOBUF "adds plugin input geobuf" ON)
option(USE_PLUGIN_INPUT_GEOJSON "adds plugin input geojson" ON)
option(USE_PLUGIN_INPUT_OGR "adds plugin input ogr" ON)
option(USE_PLUGIN_INPUT_PGRASTER "adds plugin input pgraster" ON)
option(USE_PLUGIN_INPUT_POSTGIS "adds plugin input postgis" ON)
option(USE_PLUGIN_INPUT_RASTER "adds plugin input raster" ON)
option(USE_PLUGIN_INPUT_SHAPE "adds plugin input shape" ON)
option(USE_PLUGIN_INPUT_SQLITE "adds plugin input sqlite" ON)
option(USE_PLUGIN_INPUT_TOPOJSON "adds plugin input topojson" ON)

option(BUILD_DEMO_VIEWER "builds the demo viewer" ON)
option(BUILD_DEMO_CPP "builds the demo c++ application" ON)

option(BUILD_BENCHMARK "builds benchmark project" ON)

option(BUILD_UTILITY_GEOMETRY_TO_WKB "builds the utility program geometry_to_wkb" ON)
option(BUILD_UTILITY_MAPNIK_INDEX "builds the utility program mapnik_index" ON)
option(BUILD_UTILITY_MAPNIK_RENDER "builds the utility program mapnik_render" ON)
option(BUILD_UTILITY_OGRINDEX "builds the utility program ogrindex" OFF)
option(BUILD_UTILITY_PGSQL2SQLITE "builds the utility program pgsql2sqlite" ON)
option(BUILD_UTILITY_SHAPEINDEX "builds the utility program shapeindex" ON)
option(BUILD_UTILITY_SVG2PNG "builds the utility program svg2png" ON)

option(USE_BOOST_REGEX_ICU_WORKAROUND "if you don't use your system libraries and get double linked icu libraries set this to ON" OFF)

set(CMAKE_CXX_STANDARD 14 CACHE STRING "Sets the c++ standard. c++14 is minimum.")
message(STATUS "Using c++${CMAKE_CXX_STANDARD}")
# https://cmake.org/cmake/help/latest/prop_tgt/CXX_EXTENSIONS.html
set(CXX_EXTENSIONS OFF CACHE STRING "Enables the compiler specific extensions.") # Fallsback to -std=c++<ver> if off
message(STATUS "Using c++ extensions: ${CXX_EXTENSIONS}")
# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD_REQUIRED.html#prop_tgt:CXX_STANDARD_REQUIRED
set(CXX_STANDARD_REQUIRED ON) # require the specified CMAKE_CXX_STANDARD

if(WIN32)
    set(DEFAULT_BIN_DIR bin)
    set(DEFAULT_LIB_DIR lib)
    set(DEFAULT_ARCHIVE_DIR lib)
else()
    include(GNUInstallDirs)
    set(DEFAULT_BIN_DIR ${CMAKE_INSTALL_BINDIR})
    set(DEFAULT_LIB_DIR ${CMAKE_INSTALL_LIBDIR})
    set(DEFAULT_ARCHIVE_DIR ${CMAKE_INSTALL_LIBDIR})
endif()
# https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#runtime-output-artifacts
set(MAPNIK_BIN_DIR ${DEFAULT_BIN_DIR} CACHE STRING "Install directory for binaries")
# https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#library-output-artifacts
set(MAPNIK_LIB_DIR ${DEFAULT_LIB_DIR} CACHE STRING "Install directory for libraries")
# https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#archive-output-artifacts
set(MAPNIK_ARCHIVE_DIR ${DEFAULT_ARCHIVE_DIR} CACHE STRING "Install directory for archives")

set(INSTALL_CMAKE_DIR ${MAPNIK_LIB_DIR}/cmake/mapnik CACHE STRING "Install directory of the cmake targets")

if(WIN32)
    set(DEFAULT_PLUGINS_INSTALL_DIR ${MAPNIK_BIN_DIR}/mapnik/input)
else()
    set(DEFAULT_PLUGINS_INSTALL_DIR ${MAPNIK_LIB_DIR}/mapnik/input)
endif()
set(PLUGINS_INSTALL_DIR ${DEFAULT_PLUGINS_INSTALL_DIR} CACHE STRING "installs the plugins in the specified directory")
message(STATUS "Installing plugins to ${PLUGINS_INSTALL_DIR}")

set(FONTS_INSTALL_DIR ${MAPNIK_BIN_DIR}/fonts CACHE STRING "installs the fonts in the specified directory")
message(STATUS "Installing fonts to ${FONTS_INSTALL_DIR}")


# Begin project configuration
mapnik_find_threads()
mapnik_find_package(ICU REQUIRED COMPONENTS uc i18n data)
mapnik_find_package(Boost 1.74 REQUIRED COMPONENTS filesystem system regex)
if(USE_BOOST_REGEX_ICU_WORKAROUND)
    set_property(TARGET Boost::regex PROPERTY INTERFACE_LINK_LIBRARIES)
endif()

mapnik_find_package(Freetype REQUIRED)
mapnik_find_package(harfbuzz CONFIG REQUIRED)

if(USE_EXTERNAL_MAPBOX_GEOMETRY)
    # this is used to provide a way to specify include dirs with CACHE VARIABLES
    if(NOT MAPBOX_GEOMETRY_INCLUDE_DIRS)
        message(STATUS "Searching for the include dir of mapbox/geometry.hpp")
        find_path(MAPBOX_GEOMETRY_INCLUDE_DIRS "mapbox/geometry.hpp" REQUIRED)
    endif()
else()
    set(MAPBOX_GEOMETRY_INCLUDE_DIRS 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/mapbox/geometry/include>
    )
endif()
if(NOT MAPBOX_GEOMETRY_INCLUDE_DIRS)
    message(FATAL_ERROR "Set -DMAPBOX_GEOMETRY_INCLUDE_DIRS to the mapbox/geometry.hpp include dir") 
endif()

if(USE_EXTERNAL_MAPBOX_POLYLABEL)
    if(NOT MAPBOX_POLYLABEL_INCLUDE_DIRS)
        message(STATUS "Searching for the include dir of mapbox/polylabel")
        find_path(MAPBOX_POLYLABEL_INCLUDE_DIRS "mapbox/polylabel.hpp")
    endif()
else()
    set(MAPBOX_POLYLABEL_INCLUDE_DIRS 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/mapbox/polylabel/include>
    )
endif()
if(NOT MAPBOX_POLYLABEL_INCLUDE_DIRS)
    message(FATAL_ERROR "Set MAPBOX_POLYLABEL_INCLUDE_DIRS to the mapbox/geometry include dir") 
endif()

if(USE_EXTERNAL_MAPBOX_PROTOZERO)
    if(NOT MAPBOX_PROTOZERO_INCLUDE_DIRS)
        message(STATUS "Searching for the include dir of mapbox/protozero")
        find_path(MAPBOX_PROTOZERO_INCLUDE_DIRS "protozero/pbf_message.hpp")
    endif()
else()
    set(MAPBOX_PROTOZERO_INCLUDE_DIRS 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/mapbox/protozero/include>
    )
endif()
if(NOT MAPBOX_PROTOZERO_INCLUDE_DIRS)
    message(FATAL_ERROR "Set MAPBOX_PROTOZERO_INCLUDE_DIRS to the mapbox/protozero include dir") 
endif()

if(USE_EXTERNAL_MAPBOX_VARIANT)
    if(NOT MAPBOX_VARIANT_INCLUDE_DIRS)
        message(STATUS "Searching for the include dir of mapbox/variant")
        find_path(MAPBOX_VARIANT_INCLUDE_DIRS "mapbox/variant.hpp")
    endif()
else()
    set(MAPBOX_VARIANT_INCLUDE_DIRS 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/mapbox/variant/include>
    )
endif()
if(NOT MAPBOX_VARIANT_INCLUDE_DIRS)
    message(FATAL_ERROR "Set MAPBOX_VARIANT_INCLUDE_DIRS to the mapbox/variant include dir") 
endif()

set(MAPNIK_COMPILE_DEFS "")
set(MAPNIK_OPTIONAL_LIBS "")
set(MAPNIK_OPTIONAL_LIBS_INCLUDE "")
# (used by MapnikInstall.cmake. properties are needed since "set(...)" will be out of scope
set_property(GLOBAL PROPERTY TARGETS "")
set_property(GLOBAL PROPERTY PLUGINS "")

if(USE_BIGINT) 
    message(STATUS "uses BIGINT")
    list(APPEND MAPNIK_COMPILE_DEFS BIGINT)
endif()
if(USE_MEMORY_MAPPED_FILE)
    message(STATUS "uses MAPNIK_MEMORY_MAPPED_FILE")
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_MEMORY_MAPPED_FILE)
endif()

if(USE_LIBXML2)
    mapnik_find_package(LibXml2 REQUIRED)
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_LIBXML2)
    list(APPEND MAPNIK_OPTIONAL_LIBS LibXml2::LibXml2)
endif()
if(USE_PNG)
    mapnik_find_package(PNG REQUIRED)
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_PNG)
    list(APPEND MAPNIK_OPTIONAL_LIBS PNG::PNG)
endif()

if(USE_JPEG)
    mapnik_find_package(JPEG REQUIRED)
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_JPEG)
    list(APPEND MAPNIK_OPTIONAL_LIBS JPEG::JPEG)
endif()

if(USE_TIFF)
    mapnik_find_package(TIFF REQUIRED)
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_TIFF)
    list(APPEND MAPNIK_OPTIONAL_LIBS TIFF::TIFF)
endif()

if(USE_WEBP)
    mapnik_find_package(WebP CONFIG REQUIRED)
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_WEBP)
    list(APPEND MAPNIK_OPTIONAL_LIBS WebP::webp)
endif()

if(USE_CAIRO)
    mapnik_find_package(Cairo REQUIRED)
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_CAIRO)
    list(APPEND MAPNIK_OPTIONAL_LIBS Cairo::Cairo)
endif()

if(USE_PROJ)
    #https://proj.org/development/cmake.html
    mapnik_find_package(PROJ REQUIRED)
    math(EXPR MAPNIK_PROJ_VERSION "${PROJ_VERSION_MAJOR}*10000 + ${PROJ_VERSION_MINOR}*100 + ${PROJ_VERSION_PATCH}" OUTPUT_FORMAT DECIMAL)
    message(STATUS "Using mapnik PROJ version: ${MAPNIK_PROJ_VERSION}")
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_USE_PROJ MAPNIK_PROJ_VERSION=${MAPNIK_PROJ_VERSION})
    list(APPEND MAPNIK_OPTIONAL_LIBS ${PROJ_LIBRARIES})
    list(APPEND MAPNIK_OPTIONAL_LIBS_INCLUDE ${PROJ_INCLUDE_DIRS})
endif()

if(USE_GRID_RENDERER)
    message(STATUS "Using grid renderer")
    list(APPEND MAPNIK_COMPILE_DEFS GRID_RENDERER)
endif()

if(USE_SVG_RENDERER)
    message(STATUS "Using svg renderer")
    list(APPEND MAPNIK_COMPILE_DEFS SVG_RENDERER)
endif()

if(NOT WIN32) 
    message(STATUS "Compiling with -DMAPNIK_HAS_DLCFN")
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_HAS_DLCFN)
    list(APPEND MAPNIK_OPTIONAL_LIBS ${CMAKE_DL_LIBS})
endif()

add_library(core INTERFACE)
add_library(mapnik::core ALIAS core)

target_include_directories(core INTERFACE  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
    $<BUILD_INTERFACE:${MAPBOX_GEOMETRY_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${MAPBOX_POLYLABEL_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${MAPBOX_VARIANT_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${MAPBOX_PROTOZERO_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps>
    $<INSTALL_INTERFACE:include>
    ${MAPNIK_OPTIONAL_LIBS_INCLUDE}
)
target_link_libraries(core INTERFACE 
    Threads::Threads
    ICU::uc
    ICU::data
    ICU::i18n
    Boost::headers
    Boost::regex
    Boost::filesystem
    Freetype::Freetype
    harfbuzz::harfbuzz
    ${MAPNIK_OPTIONAL_LIBS}
)
target_compile_definitions(core INTERFACE ${MAPNIK_COMPILE_DEFS})

install(TARGETS core 
    EXPORT MapnikTargets
    LIBRARY DESTINATION ${MAPNIK_LIB_DIR}
    ARCHIVE DESTINATION ${MAPNIK_ARCHIVE_DIR}
    RUNTIME DESTINATION ${MAPNIK_BIN_DIR}
    INCLUDES DESTINATION include/
    PUBLIC_HEADER DESTINATION include/
)

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(plugins)
add_subdirectory(utils)
add_subdirectory(demo)
if(BUILD_BENCHMARK)
    add_subdirectory(benchmark)
endif()
if(BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

# start package mapnik
include(CMakePackageConfigHelpers)

# set the cmake targets install location
set(INCLUDE_INSTALL_DIR include/)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/MapnikConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

list(JOIN MAPNIK_DEPENDENCIES "\n" MAPNIK_DEPENDENCIES)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/MapnikConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/MapnikConfig.cmake"
  INSTALL_DESTINATION ${INSTALL_CMAKE_DIR}
  PATH_VARS INCLUDE_INSTALL_DIR PLUGINS_INSTALL_DIR FONTS_INSTALL_DIR MAPNIK_DEPENDENCIES
)

install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/MapnikConfig.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/MapnikConfigVersion.cmake"
        DESTINATION ${INSTALL_CMAKE_DIR}
)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindCairo.cmake" DESTINATION ${INSTALL_CMAKE_DIR}/Modules)

install(EXPORT MapnikTargets
        FILE MapnikTargets.cmake
        NAMESPACE mapnik::
        DESTINATION ${INSTALL_CMAKE_DIR}
)

install(DIRECTORY include/ TYPE INCLUDE)
install(DIRECTORY deps/agg/include/ TYPE INCLUDE)
install(DIRECTORY fonts/ DESTINATION ${FONTS_INSTALL_DIR} FILES_MATCHING PATTERN "*.py" EXCLUDE PATTERN "*")

mapnik_install_targets()
