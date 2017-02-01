#!/usr/bin/env bash

set -eu
set -o pipefail

: '

todo

- docs for base setup: sudo apt-get -y install zlib1g-dev make git
- shrink icu data
'

MASON_VERSION="v0.5.0"

function setup_mason() {
    if [[ ! -d ./.mason ]]; then
        git clone https://github.com/mapbox/mason.git ./.mason
        (cd ./.mason && git checkout ${MASON_VERSION})
    else
        echo "Updating to latest mason"
        (cd ./.mason && git fetch && git checkout ${MASON_VERSION} && git pull origin ${MASON_VERSION})
    fi
    export PATH=$(pwd)/.mason:$PATH
    export CXX=${CXX:-clang++}
    export CC=${CC:-clang}
}

function install() {
    MASON_PLATFORM_ID=$(mason env MASON_PLATFORM_ID)
    if [[ ! -d ./mason_packages/${MASON_PLATFORM_ID}/${1}/${2} ]]; then
        mason install $1 $2
        if [[ ${3:-false} != false ]]; then
            LA_FILE=$(mason prefix $1 $2)/lib/$3.la
            if [[ -f ${LA_FILE} ]]; then
                perl -i -p -e 's:\Q$ENV{HOME}/build/mapbox/mason\E:$ENV{PWD}:g' ${LA_FILE}
            else
                echo "$LA_FILE not found"
            fi
        fi
    fi
    # the rm here is to workaround https://github.com/mapbox/mason/issues/230
    rm -f ./mason_packages/.link/mason.ini
    mason link $1 $2
}

ICU_VERSION="55.1"

function install_mason_deps() {
    FAIL=0
    install ccache 3.3.0 &
    install zlib system &
    install jpeg_turbo 1.5.0 libjpeg &
    install libpng 1.6.24 libpng &
    install libtiff 4.0.6 libtiff &
    install libpq 9.5.2 &
    install sqlite 3.14.1 libsqlite3 &
    install expat 2.2.0 libexpat &
    install icu ${ICU_VERSION} &
    install proj 4.9.2 libproj &
    install pixman 0.34.0 libpixman-1 &
    install cairo 1.14.6 libcairo &
    install protobuf 2.6.1 &
    # technically protobuf is not a mapnik core dep, but installing
    # here by default helps make mapnik-vector-tile builds easier
    install webp 0.5.1 libwebp &
    install gdal 2.1.1 libgdal &
    install boost 1.61.0 &
    install boost_libsystem 1.61.0 &
    install boost_libfilesystem 1.61.0 &
    install boost_libprogram_options 1.61.0 &
    install boost_libregex_icu 1.61.0 &
    # technically boost thread and python are not a core dep, but installing
    # here by default helps make python-mapnik builds easier
    install boost_libthread 1.61.0 &
    install boost_libpython 1.61.0 &
    install freetype 2.6.5 libfreetype &
    install harfbuzz 1.3.0 libharfbuzz &
    for job in $(jobs -p)
    do
        wait $job || let "FAIL+=1"
    done
    if [[ "$FAIL" != "0" ]]; then
        exit ${FAIL}
    fi
}

MASON_LINKED_ABS=$(pwd)/mason_packages/.link
MASON_LINKED_REL=./mason_packages/.link
export C_INCLUDE_PATH="${MASON_LINKED_ABS}/include"
export CPLUS_INCLUDE_PATH="${MASON_LINKED_ABS}/include"
export LIBRARY_PATH="${MASON_LINKED_ABS}/lib"

function make_config() {
    echo "
CXX = '$CXX'
CC = '$CC'
CUSTOM_CXXFLAGS = '-D_GLIBCXX_USE_CXX11_ABI=0'
RUNTIME_LINK = 'static'
INPUT_PLUGINS = 'all'
PATH = '${MASON_LINKED_REL}/bin'
PKG_CONFIG_PATH = '${MASON_LINKED_REL}/lib/pkgconfig'
PATH_REMOVE = '/usr:/usr/local'
PATH_REPLACE = '$HOME/build/mapbox/mason/mason_packages:./mason_packages'
BOOST_INCLUDES = '${MASON_LINKED_REL}/include'
BOOST_LIBS = '${MASON_LINKED_REL}/lib'
ICU_INCLUDES = '${MASON_LINKED_REL}/include'
ICU_LIBS = '${MASON_LINKED_REL}/lib'
HB_INCLUDES = '${MASON_LINKED_REL}/include'
HB_LIBS = '${MASON_LINKED_REL}/lib'
PNG_INCLUDES = '${MASON_LINKED_REL}/include/libpng16'
PNG_LIBS = '${MASON_LINKED_REL}/lib'
JPEG_INCLUDES = '${MASON_LINKED_REL}/include'
JPEG_LIBS = '${MASON_LINKED_REL}/lib'
TIFF_INCLUDES = '${MASON_LINKED_REL}/include'
TIFF_LIBS = '${MASON_LINKED_REL}/lib'
WEBP_INCLUDES = '${MASON_LINKED_REL}/include'
WEBP_LIBS = '${MASON_LINKED_REL}/lib'
PROJ_INCLUDES = '${MASON_LINKED_REL}/include'
PROJ_LIBS = '${MASON_LINKED_REL}/lib'
PG_INCLUDES = '${MASON_LINKED_REL}/include'
PG_LIBS = '${MASON_LINKED_REL}/lib'
FREETYPE_INCLUDES = '${MASON_LINKED_REL}/include/freetype2'
FREETYPE_LIBS = '${MASON_LINKED_REL}/lib'
SVG_RENDERER = True
CAIRO_INCLUDES = '${MASON_LINKED_REL}/include'
CAIRO_LIBS = '${MASON_LINKED_REL}/lib'
SQLITE_INCLUDES = '${MASON_LINKED_REL}/include'
SQLITE_LIBS = '${MASON_LINKED_REL}/lib'
BENCHMARK = True
CPP_TESTS = True
PGSQL2SQLITE = True
XMLPARSER = 'ptree'
SVG2PNG = True
"
}

# NOTE: the `mapnik-settings.env` is used by test/run (which is run by `make test`)
function setup_runtime_settings() {
    echo "export PROJ_LIB=${MASON_LINKED_ABS}/share/proj" > mapnik-settings.env
    echo "export ICU_DATA=${MASON_LINKED_ABS}/share/icu/${ICU_VERSION}" >> mapnik-settings.env
    echo "export GDAL_DATA=${MASON_LINKED_ABS}/share/gdal" >> mapnik-settings.env
    source mapnik-settings.env
}

function main() {
    setup_mason
    install_mason_deps
    make_config > ./config.py
    setup_runtime_settings
    echo "Ready, now run:"
    echo ""
    echo "    ./configure && make"
}

main

# allow sourcing of script without
# causing the terminal to bail on error
set +eu
set +o pipefail
