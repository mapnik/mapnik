#!/usr/bin/env bash

set -eu
set -o pipefail

# for normal release leave empty
# for release candidate, add "-rcN"
export MAPNIK_VERSION=$(git describe)
export TARBALL_NAME="mapnik-${MAPNIK_VERSION}"
cd /tmp/
rm -rf ${TARBALL_NAME}
echo "Cloning ${MAPNIK_VERSION}"
git clone --depth 1 --branch ${MAPNIK_VERSION} git@github.com:mapnik/mapnik.git ${TARBALL_NAME}
cd ${TARBALL_NAME}
git checkout "tags/${MAPNIK_VERSION}"
echo "updating submodules"
# TODO: skip initializing submodule if data is already tagged
# Will require bundling variant as well
git submodule update --depth 100 --init
rm -rf deps/mapbox/variant/.git
cd test/data/
git remote set-url origin git@github.com:mapnik/test-data
echo "tagging test data"
git tag ${MAPNIK_VERSION} -a -m "tagging for ${MAPNIK_VERSION}"
git push --tags
cd ../../
echo "removing test data submodule"
rm -rf test/data/
cd test/data-visual/
git remote set-url origin git@github.com:mapnik/test-data-visual
echo "tagging visual data"
git tag ${MAPNIK_VERSION} -a -m "tagging for ${MAPNIK_VERSION}"
git push --tags
cd ../../
echo "removing visual test data submodule"
rm -rf test/data-visual/
rm -rf .git
rm -rf .gitignore
cd ../
echo "creating tarball of ${TARBALL_NAME}.tar.bz2"
tar cjf ${TARBALL_NAME}.tar.bz2 ${TARBALL_NAME}/
echo "uploading $(dirname ${TARBALL_NAME})/${TARBALL_NAME}.tar.bz2 to s3://mapnik/dist/${MAPNIK_VERSION}/"
# TODO: upload to github releases instead of s3
aws s3 cp --acl public-read ${TARBALL_NAME}.tar.bz2 s3://mapnik/dist/${MAPNIK_VERSION}/
