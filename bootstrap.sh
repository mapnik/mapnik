#!/usr/bin/env bash

#set -eu

: '

todo

- gdal shared lib / avoid dlclose atexit crash
- clang debs to s3
- docs for base setup: sudo apt-get -y install zlib1g-dev python-dev make git python-dev
- shrink icu data
- cairo/pycairo
- pkg-config-less
'

function setup_mason() {
    if [[ ! -d ./.mason ]]; then
        git clone --depth 1 https://github.com/mapbox/mason.git ./.mason
    else
        echo "Updating to latest mason"
        (cd ./.mason && git pull)
    fi
    export MASON_DIR=$(pwd)/.mason
    if [[ $(uname -s) == 'Linux' ]]; then source ./.mason/scripts/setup_cpp11_toolchain.sh; fi
    export PATH=$(pwd)/.mason:$PATH
    export CXX=${CXX:-clang++}
    export CC=${CC:-clang}
}

function ip() {
    if [[ ! -d ./mason_packages/${3}/${1}/ ]]; then
        mason install $1 $2
        mason link $1 $2
    fi
}

function install_mason_deps() {
    MASON_PLATFORM_ID=$(mason env MASON_PLATFORM_ID)
    ip freetype 2.5.4 $MASON_PLATFORM_ID
    ip harfbuzz 2cd5323 $MASON_PLATFORM_ID
    ip jpeg_turbo 1.4.0 $MASON_PLATFORM_ID
    ip libxml2 2.9.2 $MASON_PLATFORM_ID
    ip libpng 1.6.13 $MASON_PLATFORM_ID
    ip webp 0.4.2 $MASON_PLATFORM_ID
    ip icu 54.1 $MASON_PLATFORM_ID
    ip proj 4.8.0 $MASON_PLATFORM_ID
    ip libtiff 4.0.4beta $MASON_PLATFORM_ID
    ip boost 1.57.0 $MASON_PLATFORM_ID
    ip boost_libsystem 1.57.0 $MASON_PLATFORM_ID
    ip boost_libthread 1.57.0 $MASON_PLATFORM_ID
    ip boost_libfilesystem 1.57.0 $MASON_PLATFORM_ID
    ip boost_libprogram_options 1.57.0 $MASON_PLATFORM_ID
    ip boost_libregex 1.57.0 $MASON_PLATFORM_ID
    ip boost_libpython 1.57.0 $MASON_PLATFORM_ID
    ip libpq 9.4.0 $MASON_PLATFORM_ID
    ip sqlite 3.8.6 $MASON_PLATFORM_ID
    ip gdal 1.11.1 $MASON_PLATFORM_ID
    ip expat 2.1.0 $MASON_PLATFORM_ID
    ip pixman 0.32.6 $MASON_PLATFORM_ID
    ip cairo 1.12.18 $MASON_PLATFORM_ID
}

function setup_nose() {
    if [[ ! -d $(pwd)/nose-1.3.4 ]]; then
        wget -q https://pypi.python.org/packages/source/n/nose/nose-1.3.4.tar.gz
        tar -xzf nose-1.3.4.tar.gz
    fi
    export PYTHONPATH=$(pwd)/nose-1.3.4:${PYTHONPATH}
}

function make_config() {
    local MASON_LINKED_REL=./mason_packages/.link
    export PKG_CONFIG_PATH="${MASON_LINKED_REL}/lib/pkgconfig"
    export C_INCLUDE_PATH="${MASON_LINKED_REL}/include"
    export CPLUS_INCLUDE_PATH="${MASON_LINKED_REL}/include"
    export LIBRARY_PATH="${MASON_LINKED_REL}/lib"
    export PATH="${MASON_LINKED_REL}/bin":${PATH}

    echo "
CXX = '$CXX'
CC = '$CC'
CUSTOM_CXXFLAGS = '-fvisibility=hidden -fvisibility-inlines-hidden -DU_CHARSET_IS_UTF8=1'
CUSTOM_LDFLAGS = '-L${MASON_LINKED_REL}/lib'
RUNTIME_LINK = 'static'
INPUT_PLUGINS = 'all'
PREFIX = '/opt/mapnik-3.x'
PATH = '${MASON_LINKED_REL}/bin'
PATH_REMOVE = '/usr:/usr/local'
PATH_REPLACE = '/Users/travis/build/mapbox/mason/mason_packages:./mason_packages'
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
FREETYPE_INCLUDES = '${MASON_LINKED_REL}/include/freetype2'
FREETYPE_LIBS = '${MASON_LINKED_REL}/lib'
XML2_INCLUDES = '${MASON_LINKED_REL}/include/libxml2'
XML2_LIBS = '${MASON_LINKED_REL}/lib'
SVG_RENDERER = True
CAIRO_INCLUDES = '${MASON_LINKED_REL}/include'
CAIRO_LIBS = '${MASON_LINKED_REL}/lib'
SQLITE_INCLUDES = '${MASON_LINKED_REL}/include'
SQLITE_LIBS = '${MASON_LINKED_REL}/lib'
FRAMEWORK_PYTHON = False
CPP_TESTS = True
PGSQL2SQLITE = True
BINDINGS = 'python'
XMLPARSER = 'ptree'
SVG2PNG = True
SAMPLE_INPUT_PLUGINS = True
" > ./config.py
}

function setup_runtime_settings() {
    local MASON_LINKED_ABS=$(pwd)/mason_packages/.link
    export PROJ_LIB=${MASON_LINKED_ABS}/share/proj/
    export ICU_DATA=${MASON_LINKED_ABS}/share/icu/54.1/
    export GDAL_DATA=${MASON_LINKED_ABS}/share/gdal
}

function main() {
    setup_mason
    install_mason_deps
    setup_nose
    make_config
    setup_runtime_settings
    echo "Ready, now run:"
    echo ""
    echo "    ./configure && make"
}

main
