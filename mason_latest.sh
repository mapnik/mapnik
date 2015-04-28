#!/usr/bin/env bash

MASON_NAME=mapnik
MASON_VERSION=latest
MASON_LIB_FILE=lib/libmapnik-wkt.a

. ${MASON_DIR:-~/.mason}/mason.sh

function mason_load_source {
    ""
}

function mason_compile {
    HERE=$(pwd)
    python -c "data=open('$MASON_PREFIX/bin/mapnik-config','r').read();open('$MASON_PREFIX/bin/mapnik-config','w').write(data.replace('$HERE','.'))"
    mkdir -p ${MASON_PREFIX}/share/gdal
    mkdir -p ${MASON_PREFIX}/share/proj
    mkdir -p ${MASON_PREFIX}/share/icu
    cp -r $GDAL_DATA/ ${MASON_PREFIX}/share/gdal/
    cp -r $PROJ_LIB/ ${MASON_PREFIX}/share/proj/
    cp -r $ICU_DATA/*dat ${MASON_PREFIX}/share/icu/
    find ${MASON_PREFIX} -name "*.pyc" -exec rm {} \;
}

function mason_cflags {
    ""
}

function mason_ldflags {
    ""
}

function mason_clean {
    ""
}

mason_run "$@"
