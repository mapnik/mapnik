#!/bin/bash

# TODO - use rpath to avoid needing this to run tests locally

export CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [[ $(uname -s) == 'Darwin' ]]; then
    export DYLD_LIBRARY_PATH="${CURRENT_DIR}/src/":${DYLD_LIBRARY_PATH:-""}
elif [[ $(uname -s) == 'Linux' ]]; then
    export LD_LIBRARY_PATH="${CURRENT_DIR}/src/":${LD_LIBRARY_PATH:-""}
fi

export PATH=$(pwd)/utils/mapnik-render/:${PATH}
export PATH=$(pwd)/utils/mapnik-index/:${PATH}
export PATH=$(pwd)/utils/mapnik-config/:${PATH}
export PATH=$(pwd)/utils/shapeindex/:${PATH}

# mapnik-settings.env is an optional file to store
# environment variables that should be used before
# running tests like PROJ_LIB, GDAL_DATA, and ICU_DATA
# These do not normally need to be set except when
# building against binary versions of dependencies like
# done via bootstrap.sh
if [[ -f mapnik-settings.env ]]; then
    echo "Inheriting from mapnik-settings.env"
    source mapnik-settings.env
fi
