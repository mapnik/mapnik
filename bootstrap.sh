#!/usr/bin/env bash

#set -eu

# NOTE: requires at least bash >= 4.0
# brew install bash

: '

todo

- clang debs to s3
- docs for base setup: sudo apt-get -y install zlib1g-dev python-dev make git python-dev
- boost_python_patch
- shrink icu data
- cairo/pycairo
- clang + libc++
- pkg-config-less
- gdal shared lib?
'

declare -A DEPS
DEPS["freetype"]="2.5.4"
DEPS["harfbuzz"]="2cd5323"
DEPS["jpeg_turbo"]="1.4.0"
DEPS["libxml2"]="2.9.2"
DEPS["libpng"]="1.6.13"
DEPS["webp"]="0.4.2"
DEPS["icu"]="54.1"
DEPS["proj"]="4.8.0"
DEPS["libtiff"]="4.0.4beta"
DEPS["boost"]="1.57.0"
DEPS["boost_libsystem"]="1.57.0"
DEPS["boost_libthread"]="1.57.0"
DEPS["boost_libfilesystem"]="1.57.0"
DEPS["boost_libprogram_options"]="1.57.0"
DEPS["boost_libregex"]="1.57.0"
DEPS["boost_libpython"]="1.57.0"
DEPS["libpq"]="9.4.0"
DEPS["sqlite"]="3.8.6"
DEPS["gdal"]="1.11.1"
DEPS["expat"]="2.1.0"

CPP11_TOOLCHAIN="$(pwd)/toolchain"

function dpack() {
    if [[ ! -f $2 ]]; then
        wget -q $1/$(echo $2 | sed 's/+/%2B/g')
        dpkg -x $2 ${CPP11_TOOLCHAIN}
    fi
}

function setup_cpp11_toolchain() {
    if [[ $(uname -s) == 'Linux' ]]; then
        local PPA="https://launchpad.net/~ubuntu-toolchain-r/+archive/ubuntu/test/+files"
        # http://llvm.org/apt/precise/dists/llvm-toolchain-precise-3.5/main/binary-amd64/Packages
        # TODO: cache these for faster downloads
        local LLVM_DIST="http://llvm.org/apt/precise/pool/main/l/llvm-toolchain-3.5"
        dpack ${LLVM_DIST} clang-3.5_3.5~svn217304-1~exp1_amd64.deb
        dpack ${LLVM_DIST} libllvm3.5_3.5~svn217304-1~exp1_amd64.deb
        dpack ${LLVM_DIST} libclang-common-3.5-dev_3.5~svn215019-1~exp1_amd64.deb
        dpack ${PPA} libstdc++6_4.8.1-2ubuntu1~12.04_amd64.deb
        dpack ${PPA} libstdc++-4.8-dev_4.8.1-2ubuntu1~12.04_amd64.deb
        dpack ${PPA} libgcc-4.8-dev_4.8.1-2ubuntu1~12.04_amd64.deb
        export CPLUS_INCLUDE_PATH="${CPP11_TOOLCHAIN}/usr/include/c++/4.8:${CPP11_TOOLCHAIN}/usr/include/x86_64-linux-gnu/c++/4.8:${CPLUS_INCLUDE_PATH}"
        export LD_LIBRARY_PATH="${CPP11_TOOLCHAIN}/usr/lib/x86_64-linux-gnu:${CPP11_TOOLCHAIN}/usr/lib/gcc/x86_64-linux-gnu/4.8/:${LD_LIBRARY_PATH}"
        export LIBRARY_PATH="${LD_LIBRARY_PATH}"
        export PATH="${CPP11_TOOLCHAIN}/usr/bin":${PATH}
        export CXX="${CPP11_TOOLCHAIN}/usr/bin/clang++-3.5"
        export CC="${CPP11_TOOLCHAIN}/usr/bin/clang-3.5"
    else
        export CXX=clang++
        export CC=clang
    fi
}

function setup_mason() {
    if [[ -d ~/.mason ]]; then
        export PATH=~/.mason:$PATH
    else
        if [[ ! -d ./.mason ]]; then
            git clone --depth 1 https://github.com/mapbox/mason.git ./.mason
        fi
        export MASON_DIR=$(pwd)/.mason
        export PATH=$(pwd)/.mason:$PATH
    fi
}

function install_mason_deps() {
    MASON_PLATFORM_ID=$(mason env MASON_PLATFORM_ID)
    for DEP in "${!DEPS[@]}"; do
        if [[ ! -d ./mason_packages/${MASON_PLATFORM_ID}/${DEP} ]]; then
            mason install ${DEP} ${DEPS[$DEP]}
            mason link ${DEP} ${DEPS[$DEP]}
        fi
    done
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

    local CUSTOM_CXXFLAGS="-fvisibility=hidden -fvisibility-inlines-hidden -DU_CHARSET_IS_UTF8=1"
    local MASON_LIBS="${MASON_LINKED_REL}/lib"
    local MASON_INCLUDES="${MASON_LINKED_REL}/include"
    echo "
CXX = '$CXX'
CC = '$CC'
CUSTOM_CXXFLAGS = '-fvisibility=hidden -fvisibility-inlines-hidden -DU_CHARSET_IS_UTF8=1'
CUSTOM_LDFLAGS = '-L${MASON_LINKED_REL}/lib'
RUNTIME_LINK = 'static'
INPUT_PLUGINS = 'csv,gdal,geojson,occi,ogr,osm,pgraster,postgis,python,raster,rasterlite,shape,sqlite,topojson'
PREFIX = '/opt/mapnik-3.x'
PATH = '${MASON_LINKED_REL}/bin'
PATH_REMOVE = '/usr:/usr/local'
MAPNIK_NAME = 'mapnik_3-0-0'
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
    setup_cpp11_toolchain
    make_config
    setup_runtime_settings
}

main
