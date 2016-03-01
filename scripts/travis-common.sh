#! /bin/bash

# elapsed_minutes
#   - outputs the number of minutes elapsed since this file was sourced
# elapsed_minutes OP VALUE
#   - shortcut for: test `elapsed_minutes` OP VALUE
our_start_time=$(date +%s)
elapsed_minutes () {
    local now=$(date +%s)
    local elapsed=$(( (now - our_start_time) / 60 ))
    case $# in
        0) echo $elapsed;;
        *) test $elapsed "$@";;
    esac
}

# enabled VALUE
#   - if VALUE is empty or falsy, returns 1 (false)
#   - otherwise returns 0 (true)
# enabled VALUE COMMAND ...
#   - if VALUE is empty or falsy, returns 0 (true)
#   - otherwise runs COMMAND and returns its result
enabled () {
    local value="$1"; shift
    case $value in
        ''|'0'|[Ff]alse|[Nn]o) test $# -ne 0;;
        *) test $# -eq 0 || "$@";;
    esac
}

# good
#   - if all build commands executed so far succeeded, returns 0
#   - otherwise returns 1
#   - this tests the conjunction of all previous command results,
#     not just the immediately preceding one
good () {
    return ${TRAVIS_TEST_RESULT:-0}
}

# on NAME
#   - if NAME == $TRAVIS_OS_NAME, returns 0 (true)
#   - otherwise returns 1 (false)
# on NAME COMMAND ...
#   - if NAME == $TRAVIS_OS_NAME, runs COMMAND and returns its result
#   - otherwise returns 0 (true)
on () {
    local name="$1"; shift
    case $name in
        $TRAVIS_OS_NAME) test $# -eq 0 || "$@";;
        *) test $# -ne 0;;
    esac
}

git_submodule_update () {
    git submodule update "$@" && return
    # failed, search pull requests for matching commits
    git submodule foreach \
        '
        test "$sha1" = "`git rev-parse HEAD`" ||
        git ls-remote origin "refs/pull/*/head" |
        while read hash ref; do
            if test "$hash" = "$sha1"; then
                git config --add remote.origin.fetch "+$ref:$ref";
            fi
        done
        '
    # try again with added fetch refs
    git submodule update "$@"
}

# install and call pip
pip () {
    if ! which pip >/dev/null; then
        easy_install --user pip && \
        export PATH="$HOME/Library/Python/2.7/bin:$PATH"
    fi
    command pip "$@"
}

# commit_message_contains TEXT
#   - returns 0 (true) if TEXT is found in commit message
#   - case-insensitive, plain-text search, not regex
commit_message_contains () {
    git log -1 --pretty='%B' "$TRAVIS_COMMIT" | grep -qiFe "$*"
}

commit_message_parse () {
    if commit_message_contains '[skip tests]'; then
        config_override "CPP_TESTS = False"
    fi
    if commit_message_contains '[skip utils]'; then
        config_override "MAPNIK_INDEX = False"
        config_override "MAPNIK_RENDER = False"
        config_override "PGSQL2SQLITE = False"
        config_override "SHAPEINDEX = False"
        config_override "SVG2PNG = False"
    fi
}

config_override () {
    echo "Appending to config.py:" "$@"
    echo "$@" >> ./config.py
}

configure () {
    if enabled ${COVERAGE}; then
        ./configure "$@" PGSQL2SQLITE=False SVG2PNG=False SVG_RENDERER=False \
            CUSTOM_LDFLAGS='--coverage' CUSTOM_CXXFLAGS='--coverage' \
            CUSTOM_CFLAGS='--coverage' DEBUG=True
    elif enabled ${MASON_PUBLISH}; then
        export MASON_NAME=mapnik
        export MASON_VERSION=latest
        export MASON_LIB_FILE=lib/libmapnik-wkt.a
        source ./.mason/mason.sh
        ./configure "$@" PREFIX=${MASON_PREFIX} \
            PATH_REPLACE='' MAPNIK_BUNDLED_SHARE_DIRECTORY=True \
            RUNTIME_LINK='static'
    else
        ./configure "$@"
    fi
    # print final config values, sorted and indented
    sort -sk1,1 ./config.py | sed -e 's/^/	/'
}

coverage () {
    ./mason_packages/.link/bin/cpp-coveralls \
        --build-root . --gcov-options '\-lp' --exclude mason_packages \
        --exclude .sconf_temp --exclude benchmark --exclude deps \
        --exclude scons --exclude test --exclude demo --exclude docs \
        --exclude fonts --exclude utils \
        > /dev/null
}

download_test_data () {
    if commit_message_contains '[skip tests]'; then
        return 1
    else
        git_submodule_update --init --depth=10
    fi
}
