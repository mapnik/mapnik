#!/usr/bin/env bash

#set -eu

# NOTE: requires at least bash >= 4.0
# brew install bash

: '

todo

- switch to clang
- gdal shared lib
- boost_python_patch
- shrink icu data
- cairo
'

TOOLCHAIN="$(pwd)/toolchain"
PPA="https://launchpad.net/~ubuntu-toolchain-r/+archive/ubuntu/test/+files"

function setup_linux_cpp11_toolchain() {
  if [[ ! -d ${TOOLCHAIN} ]]; then
      wget ${PPA}/libstdc%2B%2B6_4.8.1-2ubuntu1~12.04_amd64.deb
      dpkg -x libstdc++6_4.8.1-2ubuntu1~12.04_amd64.deb ${TOOLCHAIN}
      wget ${PPA}/gcc-4.8_4.8.1-2ubuntu1~12.04_amd64.deb
      dpkg -x gcc-4.8_4.8.1-2ubuntu1~12.04_amd64.deb ${TOOLCHAIN}
      wget ${PPA}/g%2B%2B-4.8_4.8.1-2ubuntu1~12.04_amd64.deb
      dpkg -x g++-4.8_4.8.1-2ubuntu1~12.04_amd64.deb ${TOOLCHAIN}
      wget ${PPA}/libisl10_0.12.2-2~12.04_amd64.deb
      dpkg -x libisl10_0.12.2-2~12.04_amd64.deb ${TOOLCHAIN}
      wget ${PPA}/libcloog-isl4_0.18.2-1~12.04_amd64.deb
      dpkg -x libcloog-isl4_0.18.2-1~12.04_amd64.deb ${TOOLCHAIN}
      wget ${PPA}/libstdc%2B%2B-4.8-dev_4.8.1-2ubuntu1~12.04_amd64.deb
      dpkg -x libstdc++-4.8-dev_4.8.1-2ubuntu1~12.04_amd64.deb ${TOOLCHAIN}
      ${PPA}/gcc-4.8-base_4.8.1-2ubuntu1~12.04_amd64.deb
      dpkg -x gcc-4.8-base_4.8.1-2ubuntu1~12.04_amd64.deb ${TOOLCHAIN}
      wget ${PPA}/libgcc-4.8-dev_4.8.1-2ubuntu1~12.04_amd64.deb
      dpkg -x libgcc-4.8-dev_4.8.1-2ubuntu1~12.04_amd64.deb ${TOOLCHAIN}
      wget ${PPA}/libgcc1_4.8.1-2ubuntu1~12.04_amd64.deb
      dpkg -x libgcc1_4.8.1-2ubuntu1~12.04_amd64.deb ${TOOLCHAIN}
      wget ${PPA}/cpp-4.8_4.8.1-2ubuntu1~12.04_amd64.deb
      dpkg -x cpp-4.8_4.8.1-2ubuntu1~12.04_amd64.deb ${TOOLCHAIN}
  fi
  export CPLUS_INCLUDE_PATH="${TOOLCHAIN}/usr/include/c++/4.8:${TOOLCHAIN}/usr/include/x86_64-linux-gnu/c++/4.8:${CPLUS_INCLUDE_PATH}"
  export LD_LIBRARY_PATH="${TOOLCHAIN}/usr/lib/x86_64-linux-gnu"
}

declare -A DEPS
DEPS["freetype"]="2.5.4"
DEPS["harfbuzz"]="2cd5323"
DEPS["jpeg"]="v8d"
DEPS["libxml2"]="2.9.2"
DEPS["libpng"]="1.6.13"
DEPS["webp"]="0.4.2"
DEPS["icu"]="54.1"
DEPS["proj"]="4.8.0"
DEPS["libtiff"]="dev"
DEPS["boost"]="1.57.0"
DEPS["boost_libsystem"]="1.57.0"
DEPS["boost_libthread"]="1.57.0"
DEPS["boost_libfilesystem"]="1.57.0"
DEPS["boost_libprogram_options"]="1.57.0"
DEPS["boost_libregex"]="1.57.0"
DEPS["boost_libpython"]="1.57.0"
DEPS["gdal"]="1.11.1"
DEPS["libpq"]="9.4.0"

if [[ -d ~/.mason ]]; then
    export PATH=~/.mason:$PATH
else
    if [[ ! -d mason ]]; then
        git clone --depth 1 https://github.com/mapbox/mason.git ./.mason
    fi
    export MASON_DIR=$(pwd)/.mason
    export PATH=$(pwd)/.mason:$PATH
fi

for DEP in "${!DEPS[@]}"; do
    mason install ${DEP} ${DEPS[$DEP]}
done

for DEP in "${!DEPS[@]}"; do
    mason link ${DEP} ${DEPS[$DEP]}
done

MASON_LINKED=$(pwd)/mason_packages/.link

export PROJ_LIB=${MASON_LINKED}/share/proj/
export ICU_DATA=${MASON_LINKED}/share/icu/54.1/
export GDAL_DATA=${MASON_LINKED}/share/gdal
export PKG_CONFIG_PATH="${MASON_LINKED}/lib/pkgconfig"
export C_INCLUDE_PATH="${MASON_LINKED}/include"
export CPLUS_INCLUDE_PATH="${MASON_LINKED}/include"
export LIBRARY_PATH="${MASON_LINKED}/lib"

if [[ $(uname -s) == 'Linux' ]]; then
  setup_linux_cpp11_toolchain
  #sudo apt-get -y install zlib1g-dev python-dev make git python-dev python-nose
fi

CXXFLAGS="-fvisibility=hidden -fvisibility-inlines-hidden -DU_CHARSET_IS_UTF8=1"
MASON_LIBS="${MASON_LINKED}/lib"
MASON_INCLUDES="${MASON_LINKED}/include"

./configure \
  CC=${TOOLCHAIN}/usr/bin/g++-4.8 \
  CXX=${TOOLCHAIN}/usr/bin/g++-4.8 \
  INPUT_PLUGINS=all \
  SAMPLE_INPUT_PLUGINS=True \
  SVG2PNG=True \
  BENCHMARK=True \
  SVG_RENDERER=True \
  PATH_REMOVE=/usr:/usr/local \
  RUNTIME_LINK=static \
  CUSTOM_LDFLAGS="-L${MASON_LIBS}" \
  CUSTOM_CXXFLAGS="${CXXFLAGS}" \
  BOOST_INCLUDES="${MASON_INCLUDES}" \
  BOOST_LIBS="${MASON_LIBS}" \
  ICU_INCLUDES="${MASON_INCLUDES}" \
  ICU_LIBS="${MASON_LIBS}" \
  FREETYPE_INCLUDES="${MASON_INCLUDES}/freetype2" \
  FREETYPE_LIBS="${MASON_LIBS}" \
  XML2_INCLUDES="${MASON_INCLUDES}/libxml2" \
  XML2_LIBS="${MASON_LIBS}" \
  PNG_INCLUDES="${MASON_INCLUDES}/libpng16" \
  PNG_LIBS="${MASON_LIBS}" \
  JPEG_INCLUDES="${MASON_INCLUDES}" \
  JPEG_LIBS="${MASON_LIBS}" \
  WEBP_INCLUDES="${MASON_INCLUDES}" \
  WEBP_LIBS="${MASON_LIBS}" \
  TIFF_INCLUDES="${MASON_INCLUDES}" \
  TIFF_LIBS="${MASON_LIBS}" \
  PROJ_INCLUDES="${MASON_INCLUDES}" \
  PROJ_LIBS="${MASON_LIBS}" \
  CAIRO_INCLUDES="${MASON_INCLUDES}" \
  CAIRO_LIBS="${MASON_LIBS}" \
  SQLITE_INCLUDES="${MASON_INCLUDES}" \
  SQLITE_LIBS="${MASON_LIBS}" \
  HB_INCLUDES="${MASON_INCLUDES}" \
  HB_LIBS="${MASON_LIBS}" \
  SQLITE_INCLUDES="${MASON_INCLUDES}" \
  SQLITE_LIBS="${MASON_LIBS}" \
  SQLITE_INCLUDES="${MASON_INCLUDES}" \
  SQLITE_LIBS="${MASON_LIBS}" \
