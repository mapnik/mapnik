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
include(FeatureSummary)
include(MapnikOption)
include(MapnikMinimumVersions)
include(MapnikFindPackage)
include(MapnikInstall)

include(CTest)
add_feature_info(BUILD_TESTING BUILD_TESTING "Adds tests")

mapnik_option(INSTALL_DEPENDENCIES "if ON, all dependencies (eg. required dlls) will be copied into CMAKE_INSTALL_PREFIX/MAPNIK_BIN_DIR." ON)

mapnik_option(BUILD_SHARED_LIBS "build mapnik dynamic(ON) or static(OFF)" ON)
mapnik_option(USE_EXTERNAL_MAPBOX_GEOMETRY "Use a external mapnik/geometry.hpp. If off, use the submodule" OFF)
mapnik_option(USE_EXTERNAL_MAPBOX_POLYLABEL "Use a external mapnik/polylabel. If off, use the submodule" OFF)
mapnik_option(USE_EXTERNAL_MAPBOX_PROTOZERO "Use a external mapnik/protozero. If off, use the submodule" OFF)
mapnik_option(USE_EXTERNAL_MAPBOX_VARIANT "Use a external mapnik/variant. If off, use the submodule" OFF)
mapnik_option(USE_JPEG "adds jpeg support" ON)
mapnik_option(USE_PNG "adds png support" ON)
mapnik_option(USE_TIFF "adds tiff support" ON)
mapnik_option(USE_WEBP "adds webp support" ON)
mapnik_option(USE_LIBXML2 "adds libxml2 support" ON)
mapnik_option(USE_CAIRO "adds the cairo renderer" ON)
mapnik_option(USE_PROJ "adds proj support" ON)
mapnik_option(USE_GRID_RENDERER "adds grid renderer" ON)
mapnik_option(USE_SVG_RENDERER "adds svg renderer" ON)
mapnik_option(USE_BIGINT "uses 64 bit instead of 32" ON)
mapnik_option(USE_MEMORY_MAPPED_FILE "uses file cache" ON)
mapnik_option(USE_MULTITHREADED "enables the multithreaded features (threadsafe)" ON)
mapnik_option(USE_NO_ATEXIT "disable atexit" OFF)
mapnik_option(USE_NO_DLCLOSE "disable dlclose" OFF)
mapnik_option(USE_DEBUG_OUTPUT "enables some debug messages for development" OFF)
mapnik_option(USE_LOG "enables logging output. See log severity level." OFF)
# 0 = debug
# 1 = warn
# 2 = error
# 3 = none
set(USE_LOG_SEVERITY "1" CACHE STRING "sets the logging severity (only applies when USE_LOG is ON")
mapnik_option(USE_STATS "Enable statistics reporting" OFF)

mapnik_option(USE_PLUGIN_INPUT_CSV "adds plugin input csv" ON)
mapnik_option(USE_PLUGIN_INPUT_GDAL "adds plugin input gdal" ON)
mapnik_option(USE_PLUGIN_INPUT_GEOBUF "adds plugin input geobuf" ON)
mapnik_option(USE_PLUGIN_INPUT_GEOJSON "adds plugin input geojson" ON)
mapnik_option(USE_PLUGIN_INPUT_OGR "adds plugin input ogr" ON)
mapnik_option(USE_PLUGIN_INPUT_PGRASTER "adds plugin input pgraster" ON)
mapnik_option(USE_PLUGIN_INPUT_POSTGIS "adds plugin input postgis" ON)
mapnik_option(USE_PLUGIN_INPUT_RASTER "adds plugin input raster" ON)
mapnik_option(USE_PLUGIN_INPUT_SHAPE "adds plugin input shape" ON)
mapnik_option(USE_PLUGIN_INPUT_SQLITE "adds plugin input sqlite" ON)
mapnik_option(USE_PLUGIN_INPUT_TOPOJSON "adds plugin input topojson" ON)

mapnik_option(BUILD_DEMO_VIEWER "builds the demo viewer" ON)
mapnik_option(BUILD_DEMO_CPP "builds the demo c++ application" ON)

mapnik_option(BUILD_BENCHMARK "builds benchmark project" ON)

mapnik_option(BUILD_UTILITY_GEOMETRY_TO_WKB "builds the utility program geometry_to_wkb" ON)
mapnik_option(BUILD_UTILITY_MAPNIK_INDEX "builds the utility program mapnik_index" ON)
mapnik_option(BUILD_UTILITY_MAPNIK_RENDER "builds the utility program mapnik_render" ON)
mapnik_option(BUILD_UTILITY_OGRINDEX "builds the utility program ogrindex" OFF)
mapnik_option(BUILD_UTILITY_PGSQL2SQLITE "builds the utility program pgsql2sqlite" ON)
mapnik_option(BUILD_UTILITY_SHAPEINDEX "builds the utility program shapeindex" ON)
mapnik_option(BUILD_UTILITY_SVG2PNG "builds the utility program svg2png" ON)

mapnik_option(USE_BOOST_REGEX_ICU_WORKAROUND "if you don't use your system libraries and get double linked icu libraries set this to ON" OFF)
mapnik_option(USE_GLIBC_WORKAROUND "see https://github.com/mapnik/mapnik/pull/3792 if you building with libstdc++-4.9" OFF)

feature_summary(WHAT ENABLED_FEATURES DISABLED_FEATURES)
feature_summary(FILENAME "${CMAKE_CURRENT_BINARY_DIR}/features.log" WHAT ENABLED_FEATURES DISABLED_FEATURES)

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
set(MAPNIK_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE STRING "Install directory for the headers")
set(MAPNIK_CMAKE_DIR ${MAPNIK_LIB_DIR}/cmake/mapnik CACHE STRING "Install directory of the cmake targets")
set(MAPNIK_PKGCONF_DIR ${MAPNIK_LIB_DIR}/pkgconfig CACHE STRING "Install directory for the .pc files for pkg-config")
set(MAPNIK_OUTPUT_DIR "${CMAKE_BINARY_DIR}/out")
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
    set(Boost_USE_MULTITHREADED ON)
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_THREADSAFE)
else()
    set(Boost_USE_MULTITHREADED OFF)
endif()


find_package(PkgConfig)
mapnik_find_threads()
mapnik_find_package(ICU REQUIRED COMPONENTS uc i18n data)

mapnik_find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS filesystem system regex)
list(APPEND MAPNIK_COMPILE_DEFS BOOST_REGEX_HAS_ICU)
if(USE_BOOST_REGEX_ICU_WORKAROUND)
    message(STATUS "using boost regex workaround")
    set_property(TARGET Boost::regex PROPERTY INTERFACE_LINK_LIBRARIES)
endif()
include(CheckBoostRegexIcu)
check_boost_regex()
if(BOOST_REGEX_HAS_ICU)
    message(STATUS "boost regex has icu support")
    list(APPEND MAPNIK_COMPILE_DEFS BOOST_REGEX_HAS_ICU)
endif()

mapnik_find_package(Freetype REQUIRED)

# try to find harfbuzz with the native configuration and fallback to our "own" FindHarfBuzz
mapnik_find_package(harfbuzz CONFIG QUIET)
if(harfbuzz_FOUND)
    message(STATUS "Found harfbuzz native cmake")
    list(APPEND MAPNIK_OPTIONAL_LIBS harfbuzz::harfbuzz)
else()
    # Use pkg-config when harfbuzz is not found.
    # It might be possible that in future version harfbuzz could only be found via pkg-config.
    # harfbuzz related discussion: https://github.com/harfbuzz/harfbuzz/issues/2653
    message(STATUS "harfbuzz not found via cmake. Searching via pkg-config...")
    pkg_check_modules(harfbuzz REQUIRED IMPORTED_TARGET harfbuzz>=${HARFBUZZ_MIN_VERSION})
    list(APPEND MAPNIK_OPTIONAL_LIBS PkgConfig::harfbuzz)
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
set_property(GLOBAL PROPERTY MAPNIK_UTILITIES "")

if(USE_GLIBC_WORKAROUND)
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_ENABLE_GLIBC_WORKAROUND)
endif()

if(USE_BIGINT)
    list(APPEND MAPNIK_COMPILE_DEFS BIGINT)
endif()

if(USE_MEMORY_MAPPED_FILE)
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_MEMORY_MAPPED_FILE)
endif()

if(USE_NO_ATEXIT)
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_NO_ATEXIT)
endif()

if(USE_NO_DLCLOSE)
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_NO_DLCLOSE)
endif()

if(USE_DEBUG_OUTPUT)
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_DEBUG)
endif()

if(USE_LOG)
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_LOG MAPNIK_DEFAULT_LOG_SEVERITY=${USE_LOG_SEVERITY})
endif()

if(USE_STATS)
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
    if(WIN32)
        mapnik_find_package(Cairo REQUIRED)
        list(APPEND MAPNIK_OPTIONAL_LIBS Cairo::Cairo)
    else()
        pkg_check_modules(Cairo REQUIRED IMPORTED_TARGET cairo)
        list(APPEND MAPNIK_OPTIONAL_LIBS PkgConfig::Cairo)
    endif()
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_CAIRO)
endif()

if(USE_PROJ)
    #https://proj.org/development/cmake.html
    mapnik_find_package(PROJ QUIET)
    # currently the cmake files are not installed, when installing proj via apt-get. So search via pkg-config
    if(NOT PROJ_FOUND)
        message(STATUS "PROJ not found via FindPROJ. Searching via pkg-config...")
        pkg_check_modules(PROJ REQUIRED IMPORTED_TARGET proj>=${PROJ_MIN_VERSION})
        string(REGEX MATCH "([0-9]+)\.([0-9]+)\.([0-9]+)" _dummy "${PROJ_VERSION}")
        set(PROJ_VERSION_MAJOR "${CMAKE_MATCH_1}")
        set(PROJ_VERSION_MINOR "${CMAKE_MATCH_2}")
        set(PROJ_VERSION_PATCH "${CMAKE_MATCH_3}")
        list(APPEND MAPNIK_OPTIONAL_LIBS PkgConfig::PROJ)
    else()
        if(PROJ_VERSION VERSION_LESS PROJ_MIN_VERSION)
            message(FATAL_ERROR "Proj needs to be at least version ${PROJ_MIN_VERSION}")
        endif()
        list(APPEND MAPNIK_OPTIONAL_LIBS ${PROJ_LIBRARIES})
        list(APPEND MAPNIK_OPTIONAL_LIBS_INCLUDE $<BUILD_INTERFACE:${PROJ_INCLUDE_DIRS}>)
    endif()
    math(EXPR MAPNIK_PROJ_VERSION "${PROJ_VERSION_MAJOR}*10000 + ${PROJ_VERSION_MINOR}*100 + ${PROJ_VERSION_PATCH}" OUTPUT_FORMAT DECIMAL)
    message(STATUS "Using mapnik PROJ version: ${MAPNIK_PROJ_VERSION}")
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_USE_PROJ MAPNIK_PROJ_VERSION=${MAPNIK_PROJ_VERSION})
endif()

if(USE_GRID_RENDERER)
    list(APPEND MAPNIK_COMPILE_DEFS GRID_RENDERER)
endif()

if(USE_SVG_RENDERER)
    list(APPEND MAPNIK_COMPILE_DEFS SVG_RENDERER)
endif()

if(NOT WIN32)
    message(STATUS "Compiling with -DMAPNIK_HAS_DLCFN")
    list(APPEND MAPNIK_COMPILE_DEFS MAPNIK_HAS_DLCFN)
    list(APPEND MAPNIK_OPTIONAL_LIBS ${CMAKE_DL_LIBS})
endif()

# force utf-8 source code processing
# see https://docs.microsoft.com/de-de/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8?view=msvc-170
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

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

mapnik_install(core)

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(plugins)
add_subdirectory(utils)
add_subdirectory(demo)
if(BUILD_BENCHMARK)
    add_subdirectory(benchmark)
endif()
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
file(COPY fonts DESTINATION "${MAPNIK_OUTPUT_DIR}")

feature_summary(FILENAME "${CMAKE_CURRENT_BINARY_DIR}/packages.log" WHAT PACKAGES_FOUND PACKAGES_NOT_FOUND INCLUDE_QUIET_PACKAGES)

# start package mapnik
include(MapnikExport)
include(MapnikExportPkgConfig)

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
