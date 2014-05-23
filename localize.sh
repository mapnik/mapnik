#!/bin/bash
UNAME=$(uname -s)
export CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [ ${UNAME} = 'Darwin' ]; then
    export DYLD_LIBRARY_PATH="${CURRENT_DIR}/src/"
else
    export LD_LIBRARY_PATH="${CURRENT_DIR}/src/"
fi
export PYTHONPATH="${CURRENT_DIR}/bindings/python/":${PYTHONPATH}
export MAPNIK_FONT_DIRECTORY="${CURRENT_DIR}/fonts/dejavu-fonts-ttf-2.33/ttf/"
export MAPNIK_INPUT_PLUGINS_DIRECTORY="${CURRENT_DIR}/plugins/input/"
export PATH="${CURRENT_DIR}/utils/mapnik-config/":${PATH}
export PATH="${CURRENT_DIR}/utils/nik2img":${PATH}