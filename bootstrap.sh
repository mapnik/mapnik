#!/usr/bin/env bash

#set -eu

: '

todo

- docs for base setup: sudo apt-get -y install zlib1g-dev make git
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
    export PATH=$(pwd)/.mason:$PATH
    export CXX=${CXX:-clang++}
    export CC=${CC:-clang}
}

if [[ $(uname -s) == 'Darwin' ]]; then
    FIND_PATTERN="\/Users\/travis\/build\/mapbox\/mason"
else
    FIND_PATTERN="\/home\/travis\/build\/mapbox\/mason"
fi

REPLACE="$(pwd)"
REPLACE=${REPLACE////\\/}

function install() {
    MASON_PLATFORM_ID=$(mason env MASON_PLATFORM_ID)
    if [[ ! -d ./mason_packages/${MASON_PLATFORM_ID}/${1}/${2} ]]; then
        mason install $1 $2
        mason link $1 $2
        if [[ $3 ]]; then
            LA_FILE=$(${MASON_DIR:-~/.mason}/mason prefix $1 $2)/lib/$3.la
            if [[ -f ${LA_FILE} ]]; then
               perl -i -p -e "s/${FIND_PATTERN}/${REPLACE}/g;" ${LA_FILE}
            else
                echo "$LA_FILE not found"
            fi
        fi
    fi
}

function install_mason_deps() {
    install gdal 1.11.2 libgdal &
    install boost 1.57.0 &
    install boost_libsystem 1.57.0 &
    install boost_libthread 1.57.0 &
    install boost_libfilesystem 1.57.0 &
    install boost_libprogram_options 1.57.0 &
    install boost_libregex 1.57.0 &
    install boost_libpython 1.57.0 &
    install freetype 2.5.5 libfreetype &
    install harfbuzz 0.9.40 libharfbuzz &
    install jpeg_turbo 1.4.0 libjpeg &
    install libxml2 2.9.2 libxml2 &
    install libpng 1.6.16 libpng &
    install webp 0.4.2 libwebp &
    install icu 54.1 &
    install proj 4.8.0 libproj &
    install libtiff 4.0.4beta libtiff &
    install libpq 9.4.0 &
    install sqlite 3.8.8.1 libsqlite3 &
    install expat 2.1.0 libexpat &
    install pixman 0.32.6 libpixman-1 &
    install cairo 1.12.18 libcairo &
    wait
}

MASON_LINKED_ABS=$(pwd)/mason_packages/.link
MASON_LINKED_REL=./mason_packages/.link
export C_INCLUDE_PATH="${MASON_LINKED_ABS}/include"
export CPLUS_INCLUDE_PATH="${MASON_LINKED_ABS}/include"
export LIBRARY_PATH="${MASON_LINKED_ABS}/lib"

function make_config() {
    if [[ $(uname -s) == 'Darwin' ]]; then
        local PATH_REPLACE="/Users/travis/build/mapbox/mason/mason_packages:./mason_packages"
    else
        local PATH_REPLACE="/home/travis/build/mapbox/mason/mason_packages:./mason_packages"
    fi

    echo "
CXX = '$CXX'
CC = '$CC'
CUSTOM_CXXFLAGS = '-fvisibility=hidden -fvisibility-inlines-hidden -DU_CHARSET_IS_UTF8=1'
RUNTIME_LINK = 'static'
INPUT_PLUGINS = 'all'
PATH = '${MASON_LINKED_REL}/bin'
PKG_CONFIG_PATH = '${MASON_LINKED_REL}/lib/pkgconfig'
PATH_REMOVE = '/usr:/usr/local'
PATH_REPLACE = '${PATH_REPLACE}'
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
XML2_INCLUDES = '${MASON_LINKED_REL}/include/libxml2'
XML2_LIBS = '${MASON_LINKED_REL}/lib'
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
SAMPLE_INPUT_PLUGINS = True
" > ./config.py
}

# NOTE: the `mapnik-settings.env` is used by test/run (which is run by `make test`)
function setup_runtime_settings() {
    echo "export PROJ_LIB=${MASON_LINKED_ABS}/share/proj" > mapnik-settings.env
    echo "export ICU_DATA=${MASON_LINKED_ABS}/share/icu/54.1" >> mapnik-settings.env
    echo "export GDAL_DATA=${MASON_LINKED_ABS}/share/gdal" >> mapnik-settings.env
    source mapnik-settings.env
}

function main() {
    setup_mason
    install_mason_deps
    make_config
    setup_runtime_settings
    echo "Ready, now run:"
    echo ""
    echo "    ./configure && make"
}

main
