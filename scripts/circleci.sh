#!/usr/bin/env bash

set -e -u
set -o pipefail

source bootstrap.sh
ccache --version
ccache -p || true
ccache --show-stats || true
./configure CC="clang-${LLVM_VERSION}" CXX="ccache clang++-${LLVM_VERSION} -Qunused-arguments"
make
make test