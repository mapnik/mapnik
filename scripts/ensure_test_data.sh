#!/usr/bin/env bash

set -eu
set -o pipefail

if [[ -f RELEASE_VERSION.md ]]; then
    VERSION=$(cat RELEASE_VERSION.md)
    if [[ ! -d ./test/data ]]; then
        echo "Downloading unit test data from https://github.com/mapnik/test-data/archive/${VERSION}.tar.gz"
        mkdir -p test/data/
        curl -f -L -s https://github.com/mapnik/test-data/archive/${VERSION}.tar.gz | tar zxf - --strip-components=1 -C test/data/
    fi
    if [[ ! -d ./test/data-visual ]]; then
        echo "Downloading visual test data from https://github.com/mapnik/test-data-visual/archive/${VERSION}.tar.gz"
        mkdir -p test/data-visual/
        curl -f -L -s https://github.com/mapnik/test-data-visual/archive/${VERSION}.tar.gz | tar zxf - --strip-components=1 -C test/data-visual/
    fi
elif [[ -d .git ]]; then
    git submodule update --init test/
else
    echo "Expected either git clone directory (with .git) or release tarball with `RELEASE_VERSION.md` in root"
    exit 1
fi