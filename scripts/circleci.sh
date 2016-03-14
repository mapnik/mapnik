#!/usr/bin/env bash

set -e -u
set -o pipefail

source bootstrap.sh
CCACHE=$(pwd)/mason_packages/.link/bin/ccache
${CCACHE} --version
${CCACHE} -p || true
${CCACHE} --show-stats || true
./configure CC="clang-${LLVM_VERSION}" CXX="${CCACHE} clang++-${LLVM_VERSION} -Qunused-arguments"
make
make test