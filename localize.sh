#!/bin/bash
UNAME=$(uname -s)
CURRENT_DIR=$(pwd)

if [ ${UNAME} = 'Darwin' ]; then
    export DYLD_LIBRARY_PATH="${CURRENT_DIR}/src/":${DYLD_LIBRARY_PATH}
else
    export LD_LIBRARY_PATH="${CURRENT_DIR}/src/":${DYLD_LIBRARY_PATH}
fi
export PYTHONPATH="${CURRENT_DIR}/bindings/python/":${PYTHONPATH}
export MAPNIK_FONT_DIRECTORY="${CURRENT_DIR}/fonts/dejavu-fonts-ttf-2.33/ttf/"
export MAPNIK_INPUT_PLUGINS_DIRECTORY="${CURRENT_DIR}/plugins/input/"
export PATH="${CURRENT_DIR}/bin/":${PATH}