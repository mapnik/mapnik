#!/usr/bin/env bash

MASON_NAME=mapnik
MASON_VERSION=latest
MASON_LIB_FILE=lib/libmapnik-wkt.a

. ${MASON_DIR:-~/.mason}/mason.sh

function mason_load_source {
    export MASON_BUILD_PATH=$(pwd)
}

function mason_compile {
    HERE=$(pwd)
    make install
    if [[ `uname` == 'Darwin' ]]; then
        install_name_tool -id @loader_path/libmapnik.dylib ${MASON_PREFIX}"/lib/libmapnik.dylib";
        PLUGINDIR=${MASON_PREFIX}"/lib/mapnik/input/*.input";
        for f in $PLUGINDIR; do
            echo $f;
            echo `basename $f`;
            install_name_tool -id plugins/input/`basename $f` $f;
            install_name_tool -change ${MASON_PREFIX}"/lib/libmapnik.dylib" @loader_path/../../libmapnik.dylib $f;
        done;
    fi;
    python -c "data=open('$MASON_PREFIX/bin/mapnik-config','r').read();open('$MASON_PREFIX/bin/mapnik-config','w').write(data.replace('$HERE','.'))"
    mkdir -p ${MASON_PREFIX}/share/icu
    cp -r $GDAL_DATA ${MASON_PREFIX}/share/
    cp -r $PROJ_LIB ${MASON_PREFIX}/share/
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
    echo "Done"
}

mason_run "$@"
