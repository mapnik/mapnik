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
include(MapnikMinimumVersions)
include(MapnikFindPackage)
include(MapnikCopyDependencies)
include(MapnikInstall)

set(ADDITIONAL_LIBARIES_PATHS "" CACHE STRING "only used on windows. Pass directories containing the dlls that are missing. You can ignore this, if the build (verify_app step) runs successfully")
option(COPY_LIBRARIES_FOR_EXECUTABLES "copies required shared libaries (only windows) to the executable build directory" OFF)
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
option(USE_MULTITHREADED "enables the multithreaded features (threadsafe)" ON)
option(USE_NO_ATEXIT "disable atexit" OFF)
option(USE_NO_DLCLOSE "disable dlclose" OFF)
option(USE_DEBUG_OUTPUT "enables some debug messages for development" OFF)
option(USE_LOG "enables logging output. See log severity level." OFF)
# 0 = debug
# 1 = warn
# 2 = error
# 3 = none
set(USE_LOG_SEVERITY "1" CACHE STRING "sets the logging severity (only applies when USE_LOG is ON")
option(USE_STATS "Enable statistics reporting" OFF)

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
option(USE_GLIBC_WORKAROUND "see https://github.com/mapnik/mapnik/pull/3792 if you building with libstdc++-4.9" OFF)

set(CMAKE_CXX_STANDARD 14 CACHE STRING "Sets the c++ standard. c++14 is minimum.")
message(STATUS "Using c++${CMAKE_CXX_STANDARD}")
# https://cmake.org/cmake/help/latest/prop_tgt/CXX_EXTENSIONS.html
set(CXX_EXTENSIONS OFF CACHE STRING "Enables the compiler specific extensions.") # Fallsback to -std=c++<ver> if off
message(STATUS "Using c++ extensions: ${CXX_EXTENSIONS}")
# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD_REQUIRED.html#prop_tgt:CXX_STANDARD_REQUIRED
set(CXX_STANDARD_REQUIRED ON) # require the specified CMAKE_CXX_STANDARD

# add debug postfix to the libraries
set(MAPNIK_DEBUG_POSTFIX "d" CACHE STRING "sets the debug library postfix on mapnik, wkt and json")
message(STATUS "postfix for debug libraries: ${MAPNIK_DEBUG_POSTFIX}")

include(GNUInstallDirs)
# See for more details: https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html
set(MAPNIK_BIN_DIR ${CMAKE_INSTALL_BINDIR} CACHE STRING "Install directory for binaries")
set(MAPNIK_LIB_DIR ${CMAKE_INSTALL_LIBDIR} CACHE STRING "Install directory for libraries")
set(MAPNIK_ARCHIVE_DIR ${CMAKE_INSTALL_LIBDIR} CACHE STRING "Install directory for archives")
set(MAPNIK_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE STRING "Install directory for archives")
set(MAPNIK_CMAKE_DIR ${MAPNIK_LIB_DIR}/cmake/mapnik CACHE STRING "Install directory of the cmake targets")

if(WIN32)
    set(DEFAULT_PLUGINS_INSTALL_DIR ${MAPNIK_BIN_DIR}/mapnik/input)
else()
    set(DEFAULT_PLUGINS_INSTALL_DIR ${MAPNIK_LIB_DIR}/mapnik/input)
endif()
set(PLUGINS_INSTALL_DIR ${DEFAULT_PLUGINS_INSTALL_DIR} CACHE STRING "installs the plugins in the specified directory")
message(STATUS "Installing plugins to ${PLUGINS_INSTALL_DIR}")

set(FONTS_INSTALL_DIR ${MAPNIK_BIN_DIR}/fonts CACHE STRING "installs the fonts in the specified directory")
message(STATUS "Installing fonts to ${FONTS_INSTALL_DIR}")

set(MAPNIK_COMPILE_DEFS "")
set(MAPNIK_OPTIONAL_LIBS "")
set(MAPNIK_OPTIONAL_LIBS_INCLUDE "")

# Begin project configuration

# needs to be before the first call of find_boost.
if(USE_MULTITHREADED)
    message(STATUS "multithreaded enabled")
    set(Boost_USE_MULTITHREADED ON)
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_THREADSAFE)
else()
    set(Boost_USE_MULTITHREADED OFF)
    message(STATUS "multithreaded disabled")
endif()

find_package(PkgConfig)
mapnik_find_threads()
mapnik_find_package(ICU REQUIRED COMPONENTS uc i18n data)
mapnik_find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS filesystem system regex)
if(USE_BOOST_REGEX_ICU_WORKAROUND)
    message(STATUS "using boost regex workaround")
    set_property(TARGET Boost::regex PROPERTY INTERFACE_LINK_LIBRARIES)
endif()

mapnik_find_package(Freetype REQUIRED)

# try to find harfbuzz with the native configuration and fallback to our "own" FindHarfBuzz
mapnik_find_package(harfbuzz ${HARFBUZZ_MIN_VERSION} CONFIG QUIET)
if(harfbuzz_FOUND)
    message(STATUS "Found harfbuzz native cmake")
    list(APPEND MAPNIK_OPTIONAL_LIBS harfbuzz::harfbuzz)
else()
    # we use our "own" FindHarfBuzz. See https://github.com/mapnik/mapnik/pull/4191#issuecomment-874728157 for more details
    message(STATUS "Fallback to FindHarfBuzz")
    mapnik_find_package(HarfBuzz ${HARFBUZZ_MIN_VERSION} REQUIRED COMPONENTS ICU)
    list(APPEND MAPNIK_OPTIONAL_LIBS HarfBuzz::HarfBuzz HarfBuzz::ICU)
endif()

if(USE_EXTERNAL_MAPBOX_GEOMETRY)
    # this is used to provide a way to specify include dirs with CACHE VARIABLES
    if(NOT MAPBOX_GEOMETRY_INCLUDE_DIRS)
        message(STATUS "Searching for the include dir of mapbox/geometry.hpp")
        find_path(MAPBOX_GEOMETRY_INCLUDE_DIRS "mapbox/geometry.hpp" REQUIRED)
    endif()
else()
    set(MAPBOX_GEOMETRY_INCLUDE_DIRS 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/mapbox/geometry/include>
        $<INSTALL_INTERFACE:include>
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
        $<INSTALL_INTERFACE:include>
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
        $<INSTALL_INTERFACE:include>
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
        $<INSTALL_INTERFACE:include>
    )
endif()
if(NOT MAPBOX_VARIANT_INCLUDE_DIRS)
    message(FATAL_ERROR "Set MAPBOX_VARIANT_INCLUDE_DIRS to the mapbox/variant include dir") 
endif()

# (used by MapnikInstall.cmake. properties are needed since "set(...)" will be out of scope
set_property(GLOBAL PROPERTY TARGETS "")
set_property(GLOBAL PROPERTY PLUGINS "")

if(USE_GLIBC_WORKAROUND)
    message("using glibc workaround")
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_ENABLE_GLIBC_WORKAROUND)
endif()

if(USE_BIGINT) 
    message(STATUS "uses BIGINT")
    list(APPEND MAPNIK_COMPILE_DEFS BIGINT)
endif()

if(USE_MEMORY_MAPPED_FILE)
    message(STATUS "uses MAPNIK_MEMORY_MAPPED_FILE")
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_MEMORY_MAPPED_FILE)
endif()

if(USE_NO_ATEXIT)
    message(STATUS "uses MAPNIK_NO_ATEXIT")
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_NO_ATEXIT)
endif()

if(USE_NO_DLCLOSE)
    message(STATUS "uses MAPNIK_NO_DLCLOSE")
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_NO_DLCLOSE)
endif()

if(USE_DEBUG_OUTPUT)
    message(STATUS "enabled debug outputs")
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_DEBUG)
endif()

if(USE_LOG)
    message(STATUS "logging enabled with level ${USE_LOG_SEVERITY}")
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_LOG MAPNIK_DEFAULT_LOG_SEVERITY=${USE_LOG_SEVERITY})
endif()

if(USE_STATS)
    message(STATUS "adding statistics")
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_STATS)
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
    mapnik_find_package(WebP REQUIRED)
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_WEBP)
    list(APPEND MAPNIK_OPTIONAL_LIBS WebP::WebP)
endif()

if(USE_CAIRO)
    mapnik_find_package(Cairo REQUIRED)
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_CAIRO)
    list(APPEND MAPNIK_OPTIONAL_LIBS Cairo::Cairo)
endif()

if(USE_PROJ)
    #https://proj.org/development/cmake.html
    mapnik_find_package(PROJ ${PROJ_MIN_VERSION} QUIET)
    # currently the cmake files are not installed, when installing proj via apt-get. So search via pkg-config
    if(NOT PROJ_FOUND)
        message(STATUS "PROJ not found via FindPROJ. Searching via pkg-config...")
        pkg_check_modules(PROJ REQUIRED IMPORTED_TARGET proj>=${PROJ_MIN_VERSION})
        string(REGEX MATCH "([0-9]+)\.([0-9]+)\.([0-9]+)" _dummy "${PROJ_VERSION}")
        set(PROJ_VERSION_MAJOR "${CMAKE_MATCH_1}")
        set(PROJ_VERSION_MINOR "${CMAKE_MATCH_2}")
        set(PROJ_VERSION_PATCH "${CMAKE_MATCH_3}")
    endif()
    math(EXPR MAPNIK_PROJ_VERSION "${PROJ_VERSION_MAJOR}*10000 + ${PROJ_VERSION_MINOR}*100 + ${PROJ_VERSION_PATCH}" OUTPUT_FORMAT DECIMAL)
    message(STATUS "Using mapnik PROJ version: ${MAPNIK_PROJ_VERSION}")
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_USE_PROJ MAPNIK_PROJ_VERSION=${MAPNIK_PROJ_VERSION})
    list(APPEND MAPNIK_OPTIONAL_LIBS ${PROJ_LIBRARIES})
    list(APPEND MAPNIK_OPTIONAL_LIBS_INCLUDE $<BUILD_INTERFACE:${PROJ_INCLUDE_DIRS}>)
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
    ${MAPNIK_OPTIONAL_LIBS}
)
target_compile_definitions(core INTERFACE ${MAPNIK_COMPILE_DEFS})

mapnik_install(TARGET core)

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
  "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

list(JOIN MAPNIK_DEPENDENCIES "\n" MAPNIK_DEPENDENCIES)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/mapnikConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/mapnikConfig.cmake"
  INSTALL_DESTINATION ${MAPNIK_CMAKE_DIR}
  PATH_VARS INCLUDE_INSTALL_DIR PLUGINS_INSTALL_DIR FONTS_INSTALL_DIR MAPNIK_DEPENDENCIES
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

install(DIRECTORY include/ DESTINATION ${MAPNIK_INCLUDE_DIR})
install(DIRECTORY deps/agg/include/ DESTINATION ${MAPNIK_INCLUDE_DIR})
install(DIRECTORY deps/mapnik DESTINATION ${MAPNIK_INCLUDE_DIR})
install(DIRECTORY fonts/ DESTINATION ${FONTS_INSTALL_DIR} FILES_MATCHING PATTERN "*.py" EXCLUDE PATTERN "*")

if(NOT USE_EXTERNAL_MAPBOX_GEOMETRY)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/mapbox/geometry/include/ DESTINATION ${MAPNIK_INCLUDE_DIR})
endif()
if(NOT USE_EXTERNAL_MAPBOX_POLYLABEL)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/mapbox/polylabel/include/ DESTINATION ${MAPNIK_INCLUDE_DIR})
endif()
if(NOT USE_EXTERNAL_MAPBOX_PROTOZERO)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/mapbox/protozero/include/ DESTINATION ${MAPNIK_INCLUDE_DIR})
endif()
if(NOT USE_EXTERNAL_MAPBOX_VARIANT)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/mapbox/variant/include/ DESTINATION ${MAPNIK_INCLUDE_DIR})
endif()

mapnik_install_targets()

include(pack)
