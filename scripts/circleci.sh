#!/usr/bin/env bash

set -e -u
set -o pipefail

source bootstrap.sh
ccache --version
ccache -p || true
ccache --show-stats || true
./configure CC="clang-3.8" CXX="ccache clang++-3.8 -Qunused-arguments"
make
make test