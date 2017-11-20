#!/bin/bash

set -eu
set -o pipefail
shopt -s nullglob

: '

Ensure no GLIBCXX_3.4.2x symbols are present in the binary
if ENABLE_GLIBC_WORKAROUND is set.

If symbols >= 3.4.20 then it means the binaries would not run on ubuntu trusty without upgrading libstdc++

'

FINAL_RETURN_CODE=0

function check() {
    local RESULT=0
    nm ${1} | grep "GLIBCXX_3.4.2[0-9]" > /tmp/out.txt || RESULT=$?
    if [[ ${RESULT} != 0 ]]; then
        echo "Success: GLIBCXX_3.4.2[0-9] symbols not present in binary (as expected)"
    else
        echo "$(cat /tmp/out.txt | c++filt)"
        if [[ ${ENABLE_GLIBC_WORKAROUND:-false} == true ]]; then
            FINAL_RETURN_CODE=1
        fi
    fi
}

for i in src/libmapnik*; do
    echo "checking $i"
    check $i
done

exit ${FINAL_RETURN_CODE}
