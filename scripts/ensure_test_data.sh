#!/usr/bin/env bash

set -eu
set -o pipefail

VERSION=$(git describe)

if [[ -d .git ]]; then
    git submodule update --init
else
    if [[ ! -d ./test/data-visual ]]; then
        echo "Downloading visual test data from https://github.com/mapnik/test-data-visual/archive/${VERSION}.tar.gz"
        curl -L -s https://github.com/mapnik/test-data-visual/archive/${VERSION}.tar.gz | tar zxf - --strip-components=1 -C test/data-visual/
    fi
    if [[ ! -d ./test/data ]]; then
        echo "Downloading unit test data from https://github.com/mapnik/test-data/archive/${VERSION}.tar.gz"
        curl -L -s https://github.com/mapnik/test-data/archive/${VERSION}.tar.gz | tar zxf - --strip-components=1 -C test/data/
    fi
fi