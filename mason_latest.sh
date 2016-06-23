#!/usr/bin/env bash

MASON_NAME=mapnik
MASON_VERSION=latest
MASON_LIB_FILE=lib/libmapnik-wkt.a

# warning: may break when https://github.com/mapbox/mason/issues/141 lands
# hence we are pinned for now to older mason in bootstrap.sh
. ./.mason/mason.sh

set -eu

function mason_load_source {
    export MASON_BUILD_PATH=$(pwd)
}

function mason_compile {
    HERE=$(pwd)
    make install
    # this is to adapt to when mapnik is not installed in MASON_PREFIX
    # originally (to make it easier to publish locally as a stopgap)
    MAPNIK_PREFIX=$(mapnik-config --prefix)
    if [[ ${MAPNIK_PREFIX} != ${MASON_PREFIX} ]]; then
        mkdir -p ${MASON_PREFIX}/lib
        mkdir -p ${MASON_PREFIX}/include
        mkdir -p ${MASON_PREFIX}/bin
        cp -r ${MAPNIK_PREFIX}/lib/*mapnik* ${MASON_PREFIX}/lib/
        cp -r ${MAPNIK_PREFIX}/include/mapnik ${MASON_PREFIX}/include/
        cp -r ${MAPNIK_PREFIX}/bin/mapnik* ${MASON_PREFIX}/bin/
        cp -r ${MAPNIK_PREFIX}/bin/shapeindex ${MASON_PREFIX}/bin/
    fi
    if [[ $(uname -s) == 'Darwin' ]]; then
        install_name_tool -id @loader_path/lib/libmapnik.dylib ${MASON_PREFIX}"/lib/libmapnik.dylib";
        PLUGINDIR=${MASON_PREFIX}"/lib/mapnik/input/*.input";
        for f in $PLUGINDIR; do
            echo $f;
            echo $(basename $f);
            install_name_tool -id plugins/input/$(basename $f) $f;
            install_name_tool -change "${MAPNIK_PREFIX}/lib/libmapnik.dylib" @loader_path/../../lib/libmapnik.dylib $f;
        done;
        BINDIR=${MASON_PREFIX}"/bin/*";
        for f in $BINDIR; do
            echo $f;
            echo $(basename $f);
            if [[ $(file $f) =~ 'Mach-O' ]]; then
                install_name_tool -id bin/$(basename $f) $f;
                install_name_tool -change "${MAPNIK_PREFIX}/lib/libmapnik.dylib" @loader_path/../lib/libmapnik.dylib $f;
            fi
        done;
    fi;
    python -c "data=open('$MASON_PREFIX/bin/mapnik-config','r').read();open('$MASON_PREFIX/bin/mapnik-config','w').write(data.replace('$HERE','.'))"
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
