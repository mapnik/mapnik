#! /usr/bin/env bash

set -eu

RUNTIME_PREFIX=$(cd -- "${BASH_SOURCE%"${BASH_SOURCE##*/}"}.." && pwd)

## CONFIG variables substituted from build script

CONFIG_MAPNIK_NAME="mapnik"
CONFIG_MAPNIK_VERSION="unknown"
CONFIG_MAPNIK_VERSION_STRING="unknown"
CONFIG_GIT_REVISION="N/A"
CONFIG_GIT_DESCRIBE="${CONFIG_MAPNIK_VERSION_STRING}"

CONFIG_PREFIX="/usr/local"
CONFIG_LIBDIR_SCHEMA="lib"
CONFIG_LIB_DIR_NAME="mapnik"
CONFIG_MAPNIK_LIB_BASE="${CONFIG_PREFIX}/${CONFIG_LIBDIR_SCHEMA}"
CONFIG_MAPNIK_LIB_DIR="${CONFIG_MAPNIK_LIB_BASE}/${CONFIG_LIB_DIR_NAME}"
CONFIG_MAPNIK_INPUT_PLUGINS="${CONFIG_MAPNIK_LIB_DIR}/input"
CONFIG_MAPNIK_FONTS="${CONFIG_MAPNIK_LIB_DIR}/fonts"

CONFIG_CXX="c++"
CONFIG_CXXFLAGS=
CONFIG_DEFINES=
CONFIG_LDFLAGS=
CONFIG_DEP_INCLUDES=
CONFIG_DEP_LIBS=
CONFIG_QUERIED_GDAL_DATA=
CONFIG_QUERIED_PROJ_LIB=
CONFIG_QUERIED_ICU_DATA=

## D.R.Y. variables

DRY_INCLUDES="-I${RUNTIME_PREFIX}/include -I${RUNTIME_PREFIX}/include/mapnik/agg -I${RUNTIME_PREFIX}/include/mapnik/deps"
DRY_CFLAGS="${DRY_INCLUDES} ${CONFIG_DEP_INCLUDES} ${CONFIG_DEFINES} ${CONFIG_CXXFLAGS}"
DRY_LIBS="-L${RUNTIME_PREFIX}/${CONFIG_LIBDIR_SCHEMA} -l${CONFIG_MAPNIK_NAME}"

## program below

usage()
{
    cat <<EOF
Usage: mapnik-config [OPTION]

Known values for OPTION are:

  -h --help         display this help and exit
  -v --version      version information (MAPNIK_VERSION_STRING)
  --version-number  version number (MAPNIK_VERSION) (new in 2.2.0)
  --git-revision    git hash from "git rev-list --max-count=1 HEAD"
  --git-describe    git decribe output (new in 2.2.0)
  --fonts           default fonts directory
  --input-plugins   default input plugins directory
  --defines         pre-processor defines for Mapnik build (new in 2.2.0)
  --prefix          Mapnik prefix [default $CONFIG_PREFIX]
  --lib-name        Mapnik library name
  --libs            library linking information
  --dep-libs        library linking information for Mapnik dependencies
  --ldflags         library paths (-L) information
  --includes        include paths (-I) for Mapnik headers (new in 2.2.0)
  --dep-includes    include paths (-I) for Mapnik dependencies (new in 2.2.0)
  --cxxflags        c++ compiler flags and pre-processor defines (new in 2.2.0)
  --cflags          all include paths, compiler flags, and pre-processor defines (for back-compatibility)
  --cxx             c++ compiler used to build mapnik (new in 2.2.0)
  --all-flags       all compile and link flags (new in 2.2.0)
  --gdal-data       path to GDAL_DATA directory, if detected at build time (new in 3.0.16)
  --proj-lib        path to PROJ_LIB directory, if detected at build time (new in 3.0.16)
  --icu-data        path to ICU_DATA directory, if detected at build time (new in 3.0.16)
EOF

    exit $1
}

if test $# -eq 0; then
    usage 1
fi

while test $# -gt 0; do
    case "$1" in

    -h| --help)
      usage 0
      ;;

    -v| --version)
      echo ${CONFIG_MAPNIK_VERSION_STRING}
      ;;

    --version-number)
      echo ${CONFIG_MAPNIK_VERSION}
      ;;

    --git-revision)
      echo ${CONFIG_GIT_REVISION}
      ;;

    --git-describe)
      echo ${CONFIG_GIT_DESCRIBE}
      ;;

    --fonts)
      printf '%s\n' "${CONFIG_MAPNIK_FONTS/#"$CONFIG_PREFIX"/$RUNTIME_PREFIX}"
      ;;

    --input-plugins)
      printf '%s\n' "${CONFIG_MAPNIK_INPUT_PLUGINS/#"$CONFIG_PREFIX"/$RUNTIME_PREFIX}"
      ;;

    --defines)
      printf '%s\n' "${CONFIG_DEFINES}"
      ;;

    --prefix)
      printf '%s\n' "${RUNTIME_PREFIX}"
      ;;

    --lib-name)
      printf '%s\n' "${CONFIG_MAPNIK_NAME}"
      ;;

    --libs)
      printf '%s\n' "${DRY_LIBS}"
      ;;

    --dep-libs)
      printf '%s\n' "${CONFIG_DEP_LIBS}"
      ;;

    --ldflags)
      printf '%s\n' "${CONFIG_LDFLAGS}"
      ;;

    --includes)
      printf '%s\n' "${DRY_INCLUDES}"
      ;;

    --dep-includes)
      printf '%s\n' "${CONFIG_DEP_INCLUDES}"
      ;;

    --cxxflags)
      printf '%s\n' "${CONFIG_CXXFLAGS}"
      ;;

    --cflags)
      printf '%s\n' "${DRY_CFLAGS}"
      ;;

    --cxx)
      printf '%s\n' "${CONFIG_CXX}"
      ;;

    --all-flags)
      printf '%s\n' "${DRY_CFLAGS} ${DRY_LIBS} ${CONFIG_LDFLAGS} ${CONFIG_DEP_LIBS}"
      ;;

    --gdal-data)
      printf "%s${CONFIG_QUERIED_GDAL_DATA:+\\n}" "${CONFIG_QUERIED_GDAL_DATA}"
      ;;

    --proj-lib)
      printf "%s${CONFIG_QUERIED_PROJ_LIB:+\\n}" "${CONFIG_QUERIED_PROJ_LIB}"
      ;;

    --icu-data)
      printf "%s${CONFIG_QUERIED_ICU_DATA:+\\n}" "${CONFIG_QUERIED_ICU_DATA}"
      ;;

    *)
      # push to stderr any invalid options
      echo "unknown option $1" >&2
      ;;
    esac
    shift
done

exit 0
