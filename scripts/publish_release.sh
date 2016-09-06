#!/usr/bin/env bash

set -eu
set -o pipefail

# for normal release leave empty
# for release candidate, add "-rcN"
export MAPNIK_VERSION=$(git describe)
export TARBALL_NAME="mapnik-v${MAPNIK_VERSION}"
cd /tmp/
rm -rf ${TARBALL_NAME}
echo "Cloning v${MAPNIK_VERSION}"
git clone --depth 1 --branch v${MAPNIK_VERSION} git@github.com:mapnik/mapnik.git ${TARBALL_NAME}
cd ${TARBALL_NAME}
git checkout "tags/v${MAPNIK_VERSION}"
echo "updating submodules"
git submodule update --depth 100 --init
rm -rf deps/mapbox/variant/.git
cd test/data/
git remote set-url origin git@github.com:mapnik/test-data
git tag v${MAPNIK_VERSION} -a -m "tagging for v${MAPNIK_VERSION}"
git push --tags
cd ../../
rm -rf test/data/
cd test/data-visual/
git remote set-url origin git@github.com:mapnik/test-data-visual
git tag v${MAPNIK_VERSION} -a -m "tagging for v${MAPNIK_VERSION}"
git push --tags
cd ../../
rm -rf test/data-visual/
rm -rf .git
rm -rf .gitignore
cd ../
tar cjf ${TARBALL_NAME}.tar.bz2 ${TARBALL_NAME}/
#aws s3 cp --acl public-read ${TARBALL_NAME}.tar.bz2 s3://mapnik/dist/v${MAPNIK_VERSION}/
