#!/usr/bin/env bash

#set -eu

: '

todo

- gdal shared lib / avoid dlclose atexit crash
- clang debs to s3
- docs for base setup: sudo apt-get -y install zlib1g-dev python-dev make git python-dev
- shrink icu data
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

function install() {
    MASON_PLATFORM_ID=$(mason env MASON_PLATFORM_ID)
    if [[ ! -d ./mason_packages/${MASON_PLATFORM_ID}/${1}/ ]]; then
        mason install $1 $2
        mason link $1 $2
    fi
}

function install_mason_deps() {
    install freetype 2.5.4
    install harfbuzz 2cd5323
    install jpeg_turbo 1.4.0
    install libxml2 2.9.2
    install libpng 1.6.16
    install webp 0.4.2
    install icu 54.1
    install proj 4.8.0
    install libtiff 4.0.4beta
    install boost 1.57.0
    install boost_libsystem 1.57.0
    install boost_libthread 1.57.0
    install boost_libfilesystem 1.57.0
    install boost_libprogram_options 1.57.0
    install boost_libregex 1.57.0
    install boost_libpython 1.57.0
    install libpq 9.4.0
    install sqlite 3.8.8.1
    install gdal 1.11.1
    install expat 2.1.0
    install pixman 0.32.6
    install cairo 1.12.18
}

function setup_nose() {
    if [[ ! -d $(pwd)/nose-1.3.4 ]]; then
        curl -s -O https://pypi.python.org/packages/source/n/nose/nose-1.3.4.tar.gz
        tar -xzf nose-1.3.4.tar.gz
    fi
    export PYTHONPATH=$(pwd)/nose-1.3.4:${PYTHONPATH}
}

function make_config() {
    local MASON_LINKED_REL=./mason_packages/.link
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
PATH = '${MASON_LINKED_REL}/bin'
PKG_CONFIG_PATH = '${MASON_LINKED_REL}/lib/pkgconfig'
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
BENCHMARK = True
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
