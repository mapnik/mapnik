
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
  --gdal-data       path to GDAL_DATA directory, if known (relevant only for packaged builds of Mapnik) (new in 3.0.0)
  --proj-lib        path to PROJ_LIB directory, if known (relevant only for packaged builds of Mapnik) (new in 3.0.0)
  --icu-data        path to ICU_DATA directory, if known (relevant only for packaged builds of Mapnik) (new in 3.0.0)
EOF

    exit $1
}

echoerr() { echo "$@" 1>&2; }

if test $# -eq 0; then
    usage 1
fi

while test $# -gt 0; do
    case "$1" in
    esac

    case "$1" in

    --help)
      usage 0
      ;;

    -h)
      usage 0
      ;;

    -v)
      echo ${CONFIG_MAPNIK_VERSION_STRING}
      ;;

    --version)
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
      echo ${CONFIG_FONTS}
      ;;

    --input-plugins)
      echo ${CONFIG_INPUT_PLUGINS}
      ;;

    --defines)
      echo ${CONFIG_MAPNIK_DEFINES}
      ;;

    --prefix)
      echo ${CONFIG_PREFIX}
      ;;

    --lib-name)
      echo ${CONFIG_MAPNIK_LIBNAME}
      ;;

    --libs)
      echo -L${CONFIG_MAPNIK_LIBPATH} -l${CONFIG_MAPNIK_LIBNAME}
      ;;

    --dep-libs)
      echo ${CONFIG_DEP_LIBS}
      ;;

    --ldflags)
      echo ${CONFIG_MAPNIK_LDFLAGS}
      ;;

    --includes)
      echo -I${CONFIG_MAPNIK_INCLUDE}
      ;;

    --dep-includes)
      echo ${CONFIG_DEP_INCLUDES}
      ;;

    --cxxflags)
      echo ${CONFIG_CXXFLAGS}
      ;;

    --cflags)
      echo -I${CONFIG_MAPNIK_INCLUDE} ${CONFIG_DEP_INCLUDES} ${CONFIG_MAPNIK_DEFINES} ${CONFIG_CXXFLAGS}
      ;;

    --cxx)
      echo ${CONFIG_CXX}
      ;;

    --all-flags)
      echo -I${CONFIG_MAPNIK_INCLUDE} ${CONFIG_DEP_INCLUDES} ${CONFIG_MAPNIK_DEFINES} ${CONFIG_CXXFLAGS} -L${CONFIG_MAPNIK_LIBPATH} -l${CONFIG_MAPNIK_LIBNAME} ${CONFIG_MAPNIK_LDFLAGS} ${CONFIG_DEP_LIBS}
      ;;

    --gdal-data)
      if [[ ${CONFIG_MAPNIK_GDAL_DATA:-unset} != "unset" ]]; then echo ${CONFIG_PREFIX}/${CONFIG_MAPNIK_GDAL_DATA}; fi;
      ;;

    --proj-lib)
      if [[ ${CONFIG_MAPNIK_PROJ_LIB:-unset} != "unset" ]]; then echo ${CONFIG_PREFIX}/${CONFIG_MAPNIK_PROJ_LIB}; fi;
      ;;

    --icu-data)
      if [[ ${CONFIG_MAPNIK_ICU_DATA:-unset} != "unset" ]]; then echo ${CONFIG_PREFIX}/${CONFIG_MAPNIK_ICU_DATA}; fi;
      ;;

    *)
  # push to stderr any invalid options
  echo "unknown option $1" 1>&2;
  ;;
    esac
    shift
done

exit 0
