#!/bin/bash

set -eu
set -o pipefail

: '

Note: before running this script you need to tag and publish a new release (it can be a draft)

Usage:

  ./scripts/test_release.sh

This script:

 - Downloads the latest release tarball from github
 - Builds it and runs tests

'

function step { >&2 echo -e "\033[1m\033[36m* $1\033[0m"; }
function step_error { >&2 echo -e "\033[1m\033[31m$1\033[0m"; }

if [[ ${GITHUB_TOKEN_MAPNIK_PUBLIC_REPO:-false} == false ]]; then
    step_error "Please set GITHUB_TOKEN_MAPNIK_PUBLIC_REPO to a github token with 'public_repo' scope (create one at https://github.com/settings/tokens)"
    exit 1
fi

export MAPNIK_VERSION="$(git describe)"
if [[ $(git tag -l) =~ ${MAPNIK_VERSION} ]]; then
    step "Success: found $MAPNIK_VERSION (result of git describe) in tags, continuing"
else
    step_error "error: $MAPNIK_VERSION (result of git describe) not in "git tag -l" output, aborting"
    step_error "You must create a valid annotated tag first, before running this ./scripts/publish_release.sh"
    exit 1
fi

curl --fail https://api.github.com/repos/mapnik/mapnik/releases -H "Authorization: token ${GITHUB_TOKEN_MAPNIK_PUBLIC_REPO}" > /tmp/mapnik-releases.json
RELEASE_ASSET_NAME=$(python -c "import json;print json.load(open('/tmp/mapnik-releases.json'))[0]['assets'][0]['name']")
if [[ ${RELEASE_ASSET_NAME} == "mapnik-${MAPNIK_VERSION}.tar.bz2" ]]; then
    step "Successfully found release asset to test: mapnik-${MAPNIK_VERSION}.tar.bz2"
else
    step_error "Error: did not find correct release asset to test: mapnik-${MAPNIK_VERSION}.tar.bz2"
    exit 1
fi

export RELEASE_ASSET_URL=$(python -c "import json;print json.load(open('/tmp/mapnik-releases.json'))[0]['assets'][0]['url']")
step "Downloading ${RELEASE_ASSET_URL}"
mkdir -p /tmp/build-mapnik-${MAPNIK_VERSION}/
rm -rf /tmp/build-mapnik-${MAPNIK_VERSION}/*
cd /tmp/build-mapnik-${MAPNIK_VERSION}/
# note: curl passes the "Authorization" header to redirects such that this breaks aws
# hence we need a two step approach here to downloading rather than depending on -L

# first a head request to get the download redirect
curl -I -f ${RELEASE_ASSET_URL} -H "Accept: application/octet-stream" -H "Authorization: token ${GITHUB_TOKEN_MAPNIK_PUBLIC_REPO}" > redirect.json
# now download from the github s3 location after stripping bogus newline
export RELEASE_ASSET_S3=$(cat redirect.json | grep location | cut -d' ' -f2 | tr '\r' ' ')
curl --retry 3 -f -S -L "${RELEASE_ASSET_S3}" -o mapnik-${MAPNIK_VERSION}.tar.bz2
tar xf mapnik-${MAPNIK_VERSION}.tar.bz2
cd mapnik-${MAPNIK_VERSION}
source bootstrap.sh
./configure CXX="$(pwd)/mason_packages/.link/bin/ccache clang++"
make
make test
