#!/bin/bash
# TODO - use rpath to avoid needing this to run tests locally
export CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [ $(uname -s) = 'Darwin' ]; then
    export DYLD_LIBRARY_PATH="${CURRENT_DIR}/src/":${DYLD_LIBRARY_PATH}
else
    export LD_LIBRARY_PATH="${CURRENT_DIR}/src/":${LD_LIBRARY_PATH}
fi