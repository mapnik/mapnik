#!/bin/bash

set -eu
set -o pipefail

: '

Usage:

  git tag v3.0.12-rc7 -a -m "Tagging v3.0.12-rc7"
  ./scripts/publish_release.sh

Note: before running this script you need to tag a new release or release candidate.

This script:

 - confirms that the current git checkout is a valid tag
 - Downloads a fresh checkout to a /tmp directory
 - Updates the submodules
 - Confirms that the test-data and test-data-visual is also tagged, otherwise tags them
 - Removes the test-data and test-data-visual since they are large and can be downloaded dynamically for released code
 - Creates a tarball and uploads to a DRAFT "github release"

After using this script:

 - Go to https://github.com/mapnik/mapnik/releases and confirm that the draft release looks good, then publish it.

'

function step { >&2 echo -e "\033[1m\033[36m* $1\033[0m"; }
function step_error { >&2 echo -e "\033[1m\033[31m$1\033[0m"; }

if [[ ${GITHUB_TOKEN_MAPNIK_PUBLIC_REPO:-false} == false ]]; then
    step_error "Please set GITHUB_TOKEN_MAPNIK_PUBLIC_REPO to a github token with 'public_repo' scope (create one at https://github.com/settings/tokens)"
    exit 1
fi

export MAPNIK_VERSION=$(git describe)
if [[ $(git tag -l) =~ $MAPNIK_VERSION ]]; then
    step "Success: found $MAPNIK_VERSION (result of git describe) in tags, continuing"
else
    step_error "error: $MAPNIK_VERSION (result of git describe) not in "git tag -l" output, aborting"
    step_error "You must create a valid annotated tag first, before running this ./scripts/publish_release.sh"
    exit 1
fi

# alternatively set this to `git@github.com:mapnik/mapnik.git` to pull public tag
export ROOT_GIT_CLONE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd ../ && pwd )"

export TARBALL_NAME="mapnik-${MAPNIK_VERSION}"
cd /tmp/
rm -rf ${TARBALL_NAME}
step "Cloning ${MAPNIK_VERSION}"
git clone --depth 1 --branch ${MAPNIK_VERSION} ${ROOT_GIT_CLONE} ${TARBALL_NAME}
cd ${TARBALL_NAME}
step "Checking out ${MAPNIK_VERSION}"
git checkout "tags/${MAPNIK_VERSION}"

step "checking submodules"
step "vendorizing and cleaning up mapbox variant"
git submodule update --init deps/mapbox/variant
rm -rf deps/mapbox/variant/.git
rm -f deps/mapbox/variant/*yml
rm -f deps/mapbox/variant/Jamroot

function check_and_tag() {
    REPO_DIR=$1
    REPO_NAME=$2
    cmd="curl --fail -I https://github.com/mapnik/${REPO_NAME}/releases/tag/${MAPNIK_VERSION}"
    if [[ $(${cmd}) ]]; then
        step "test data already tagged, no need to initialize submodule"
    else
        step "tagging test data"
        git submodule update --init ${REPO_DIR}
        cd ${REPO_DIR}/
        git remote set-url origin git@github.com:mapnik/${REPO_NAME}
        git tag ${MAPNIK_VERSION} -a -m "tagging for ${MAPNIK_VERSION}"
        git push --tags
        cd ../../
        step "removing test data submodule"
        rm -rf ${REPO_DIR}/
    fi

}

# test data
check_and_tag test/data test-data
# test data visual
check_and_tag test/data-visual test-data-visual

step "removing .git and .gitignore"
rm -rf .git
rm -rf .gitignore
export TARBALL_COMPRESSED=${TARBALL_NAME}.tar.bz2
echo ${MAPNIK_VERSION} > RELEASE_VERSION.md
step "creating tarball of ${TARBALL_COMPRESSED}"
cd ../
tar cjf ${TARBALL_COMPRESSED} ${TARBALL_NAME}/
step "uploading to github"
# https://developer.github.com/v3/repos/releases/#create-a-release
IS_PRERELEASE=false
if [[ ${MAPNIK_VERSION} =~ 'rc' ]] || [[ ${MAPNIK_VERSION} =~ 'alpha' ]]; then
  IS_PRERELEASE=true
fi
IS_DRAFT=true
step "creating a draft release"

export CHANGELOG_REF=$(python -c "print '${MAPNIK_VERSION}'.replace('.','').replace('v','').split('-')[0]")
export RELEASE_NOTES="Mapnik ${MAPNIK_VERSION}\r\n\r\n[Changelog](https://github.com/mapnik/mapnik/blob/${MAPNIK_VERSION}/CHANGELOG.md#${CHANGELOG_REF})"
step "release notes: $RELEASE_NOTES"

# create draft release
curl --data "{\"tag_name\": \"${MAPNIK_VERSION}\",\"target_commitish\": \"master\",\"name\": \"${MAPNIK_VERSION}\",\"body\": \"${RELEASE_NOTES}\",\"draft\": ${IS_DRAFT},\"prerelease\": ${IS_PRERELEASE}}" \
https://api.github.com/repos/mapnik/mapnik/releases?access_token=${GITHUB_TOKEN_MAPNIK_PUBLIC_REPO} \
> create_response.json
cat create_response.json
# parse out upload url and form it up to post tarball
UPLOAD_URL=$(python -c "import json;print json.load(open('create_response.json'))['upload_url'].replace('{?name,label}','?name=${TARBALL_COMPRESSED}')")
HTML_URL=$(python -c "import json;print json.load(open('create_response.json'))['html_url']")

step "upload url: $UPLOAD_URL"

# upload source tarball
curl ${UPLOAD_URL} \
-X POST \
-H "Authorization: token ${GITHUB_TOKEN_MAPNIK_PUBLIC_REPO}" \
-H "Content-Type:application/octet-stream" \
--data-binary @${TARBALL_COMPRESSED}

echo
step "Success: view your new draft release at ${HTML_URL}"
open ${HTML_URL}
echo

#step "uploading $(realpath ${TARBALL_COMPRESSED}) to s3://mapnik/dist/${MAPNIK_VERSION}/"
#aws s3 cp --acl public-read ${TARBALL_COMPRESSED} s3://mapnik/dist/${MAPNIK_VERSION}/
