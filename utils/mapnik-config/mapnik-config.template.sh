
## program below

usage()
{
    cat <<EOF
Usage: mapnik-config [OPTION]

Known values for OPTION are:

  --prefix          display mapnik prefix [default $CONFIG_PREFIX]
  --prefix=DIR      change mapnik prefix [default $CONFIG_PREFIX]
  --libs            print library linking information
  --dep-libs        print library linking information for mapnik depedencies
  --ldflags         print library paths (-L) information
  --cflags          print pre-processor and compiler flags
  --fonts           print default fonts directory
  --input-plugins   print default input plugins directory
  --json            print all config options as json object
  --help            display this help and exit
  -v --version      output version information
  --svn-revision    output svn revision information
EOF

    exit $1
}

if test $# -eq 0; then
    usage 1
fi

while test $# -gt 0; do
    case "$1" in
    -*=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
    *) optarg= ;;
    esac

    case "$1" in
    
    --prefix=*)
      prefix=$optarg
      includedir=$CONFIG_PREFIX/include
      CONFIG_MAPNIK_LIB=$CONFIG_PREFIX/lib
      ;;

    --prefix)
      echo $CONFIG_PREFIX
      ;;

    -v)
      echo $CONFIG_VERSION
      ;;

    --version)
      echo $CONFIG_VERSION
      ;;

    --json)
      echo $CONFIG_JSON
      ;;

    --svn-revision)
      echo ${CONFIG_SVN_REVISION}
      ;;

    --help)
      usage 0
      ;;

    --fonts)
      echo ${CONFIG_FONTS} 
      ;;

    --input-plugins)
      echo ${CONFIG_INPUT_PLUGINS} 
      ;;
      
    --cflags)
      echo -I${CONFIG_MAPNIK_INCLUDE} ${CONFIG_OTHER_INCLUDES}
      ;;

    --libs)
      echo -L${CONFIG_MAPNIK_LIB} -l${CONFIG_MAPNIK_LIBNAME}
      ;;

    --ldflags)
      echo ${CONFIG_MAPNIK_LDFLAGS}
      ;;

    --lib-name)
      echo ${CONFIG_MAPNIK_LIBNAME}
      ;;

    --dep-libs)
      echo ${CONFIG_DEP_LIBS}
      ;;

    *)
  # if no matches, return 'usage 1' meaning usage + 1 (error return type)
  usage 1
  exit 1
  ;;
    esac
    shift
done

exit 0
