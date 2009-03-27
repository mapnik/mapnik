dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(mapnik, 0.5.1)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

dnl Disable libtool 1.5 support for languages we don't use
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_GCJ_CONFIG], [:])dnl

AC_ISC_POSIX
AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC

AM_PROG_LIBTOOL

dnl Check for option to enable debug
AC_MSG_CHECKING(whether to enable debugging)
AC_ARG_ENABLE(debug,
        [  --enable-debug=[no/yes]        enables debug build (default=no)],,
                 enable_debug=no)

if [ ! test "x$enable_debug" != "xyes"]; then
  AC_DEFINE(DEBUG, 1, [Define to enable debug build])
  CXXFLAGS="-ggdb -O0 -DDEBUG"
  AC_MSG_RESULT(yes)
else
  CXXFLAGS="-O3"
  AC_MSG_RESULT(no)
fi


dnl Check for option to enable profiling
AC_MSG_CHECKING(whether to enable profiling)
AC_ARG_ENABLE(profiling,
        [  --enable-profiling=[no/yes]        enables profiling build (default=no)],,
                 enable_profiling=no)

if [ ! test "x$enable_profiling" != "xyes"]; then
  AC_DEFINE(PROFILING, 1, [Define to enable profiling build])
  PROFILING_CFLAGS="$PROFILING_CFLAGS -pg"
  AC_SUBST(PROFILING_CFLAGS)
else
  AC_MSG_RESULT(no)
fi

dnl Check for option to enable tracing
AC_MSG_CHECKING(whether to enable tracing)
AC_ARG_ENABLE(tracing,
        [  --enable-tracing=[no/yes]        enables tracing build (default=no)],,
                 enable_tracing=no)

if [ ! test "x$enable_tracing" != "xyes"]; then
  TRACING_CFLAGS="-DMAPNIK_DEBUG"
  AC_SUBST(TRACING_CFLAGS)
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

dnl Check for boost
AX_BOOST_BASE
if test "$succeeded" != "yes" ; then
  echo "Error: You need to install the boost library!"
  exit
fi

AX_BOOST_THREAD
if test "x$ax_cv_boost_thread" = "xno"; then
  echo
  echo "Error: You need to install the boost thread library!"
  echo
  exit
fi

AX_BOOST_FILESYSTEM
if test "x$ax_cv_boost_filesystem" = "xno"; then
  echo
  echo "Error: You need to install the boost filesystem library!"
  echo
  exit
fi

AX_BOOST_REGEX
if test "x$ax_cv_boost_regex" = "xno"; then
  echo
  echo "Error: You need to install the boost regex library!"
  echo
  exit
fi

AX_BOOST_SERIALIZATION
if test "x$ax_cv_boost_serialization" = "xno"; then
  echo
  echo "Error: You need to install the boost serialization library!"
  echo
  exit
fi

AX_BOOST_IOSTREAMS
if test "x$ax_cv_boost_iostreams" = "xno"; then
  echo
  echo "Error: You need to install the boost iostreams library!"
  echo
  exit
fi

AX_BOOST_PROGRAM_OPTIONS
AM_CONDITIONAL(HAVE_BOOST_PROGRAM_OPTIONS, test "x$ax_cv_boost_program_options" = "xyes")

dnl check to build python bindings
AX_BOOST_PYTHON

AM_PATH_PYTHON([2.4])

AX_LIB_POSTGRESQL
if test "x$found_postgresql" = "xyes"; then
  echo "checking whether to enable postgresql... yes"
else
  echo "checking whether to enable postgresql... no"
fi
AM_CONDITIONAL(HAVE_POSTGRESQL, test "x$found_postgresql" = "xyes")

AC_ARG_WITH(gdal-config,
[  --with-gdal-config=GDAL_CONFIG    use gdal-config specified ],
[
  GDAL_CONFIG=$withval;
  echo "using "$GDAL_CONFIG" for gdal-config";
],[
  PROG="gdal-config";
  AC_PATH_PROG(GDAL_CONFIG, $PROG, "", $PATH)
])

if test "x$GDAL_CONFIG" != "x"; then
  GDAL_CFLAGS=`$GDAL_CONFIG --cflags`
  GDAL_LIBS=`$GDAL_CONFIG --libs`
  AC_SUBST(GDAL_CFLAGS)
  AC_SUBST(GDAL_LIBS)
  found_gdal="yes"
else
  found_gdal="no"
fi
AM_CONDITIONAL(HAVE_GDAL, test "x$GDAL_CONFIG" != "x")


AX_CHECK_TIFF
if test "x$link_tiff" = "xno"; then
  exit
fi


AX_CHECK_JPEG
if test "x$link_jpeg" = "xno"; then
  exit
fi

AX_CHECK_PROJ
if test "x$link_proj" = "xno"; then
  exit
fi

AX_CHECK_LTDL
if test "x$link_ltdl" = "xno"; then
  exit
fi

## test for optional sqlite3 support
AX_LIB_SQLITE3
if [ test "x$success" = "xyes"]; then
  found_sqlite3="yes"
fi
AM_CONDITIONAL(HAVE_SQLITE3, test "x$success" = "xyes")

PKG_CHECK_MODULES(PNG, libpng)
AC_SUBST(PNG_CFLAGS)
AC_SUBST(PNG_LIBS)

PKG_CHECK_MODULES(FREETYPE2, freetype2)
AC_SUBST(FREETYPE2_CFLAGS)
AC_SUBST(FREETYPE2_LIBS)

dnl Check for option to enable included-agg
AC_MSG_CHECKING(whether to enable included libagg building)
AC_ARG_ENABLE(included_agg,
        [  --enable-included-agg=[no/yes]        enables included libagg build (default=yes)],,
                 enable_included_agg=yes)

AM_CONDITIONAL(BUILD_AGG, test "x$enable_included_agg" = "xyes")

if [ test "x$enable_included_agg" = "xyes"]; then
  AGG_LIBS=-L../agg/src
  AGG_CFLAGS=-I../agg/include
  AC_SUBST(AGG_LIBS)
  AC_SUBST(AGG_CFLAGS)
  
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  PKG_CHECK_MODULES(AGG, libagg)
fi


dnl Check for option to enable libxml2
AC_MSG_CHECKING(whether to enable libxml2 support)
AC_ARG_ENABLE(libxml2,
        [  --enable-libxml2=[no/yes]        enables libxml2 support (default=no)],,
                 enable_libxml2=no)

AM_CONDITIONAL(HAVE_LIBXML2, test "x$enable_libxml2" = "xyes")

if [ test "x$enable_libxml2" = "xyes"]; then
  AC_MSG_RESULT(yes)
  PKG_CHECK_MODULES(LIBXML2, libxml-2.0)
  AC_DEFINE(HAVE_LIBXML2, 1,"")
else
  AC_MSG_RESULT(no)
fi

dnl Check for option to enable cairo
AC_MSG_CHECKING(whether to enable cairo support)
AC_ARG_ENABLE(cairo,
        [  --enable-cairo=[no/yes]        enables cairo support (default=yes)],,
                 enable_cairo=yes)

AM_CONDITIONAL(HAVE_CAIRO, test "x$enable_cairo" = "xyes")

if [ test "x$enable_cairo" = "xyes"]; then
  AC_MSG_RESULT(yes)
  PKG_CHECK_MODULES(CAIRO, cairomm-1.0 >= 1.4.4)
  dnl AC_DEFINE(HAVE_CAIRO, 1,"")
else
  AC_MSG_RESULT(no)
fi

#dnl Set PACKAGE SOURCE DIR in config.h.
packagesrcdir=`cd $srcdir && pwd`

#dnl Set PACKAGE PREFIX
if test "x${prefix}" = "xNONE"; then
  packageprefix=${ac_default_prefix}
else
  packageprefix=${prefix}
fi

#dnl Set PACKAGE DIRS in config.h
packagedatadir=share/${PACKAGE}
packagedocdir=doc/${PACKAGE}
packagepixmapsdir=share/pixmaps/${PACKAGE}
packagelibdir=lib/${PACKAGE}
systemfontsdir=share/fonts

#dnl Subst PACKAGE_DATA_DIR.
NO_PREFIX_PACKAGE_DATA_DIR="${packagedatadir}"
AC_SUBST(NO_PREFIX_PACKAGE_DATA_DIR)
PACKAGE_DATA_DIR="${packageprefix}/${packagedatadir}"
AC_SUBST(PACKAGE_DATA_DIR)

dnl Subst PACKAGE_DOC_DIR.
NO_PREFIX_PACKAGE_DOC_DIR="${packagedocdir}"
AC_SUBST(NO_PREFIX_PACKAGE_DOC_DIR)
PACKAGE_DOC_DIR="${packageprefix}/${packagedocdir}"
AC_SUBST(PACKAGE_DOC_DIR)

dnl Subst PACKAGE_PIXMAPS_DIR.
NO_PREFIX_PACKAGE_PIXMAPS_DIR="${packagepixmapsdir}"
AC_SUBST(NO_PREFIX_PACKAGE_PIXMAPS_DIR)
PACKAGE_PIXMAPS_DIR="${packageprefix}/${packagepixmapsdir}"
AC_SUBST(PACKAGE_PIXMAPS_DIR)

dnl Subst PACKAGE_LIB_DIR
NO_PREFIX_PACKAGE_LIB_DIR="${packagelibmapsdir}"
AC_SUBST(NO_PREFIX_PACKAGE_LIB_DIR)
PACKAGE_LIB_DIR="${packageprefix}/${packagelibdir}"
AC_SUBST(PACKAGE_LIB_DIR)

dnl Subst SYSTEM_FONTS_DIR
NO_PREFIX_SYSTEM_FONTS_DIR="${systemfontsdir}"
AC_SUBST(NO_PREFIX_SYSTEM_FONTS_DIR)
SYSTEM_FONTS_DIR="${packageprefix}/${systemfontsdir}"
AC_SUBST(SYSTEM_FONTS_DIR)

AC_DEFINE_UNQUOTED(SYSTEM_FONTS_DIR, "${packageprefix}/${systemfontsdir}","")
AC_DEFINE_UNQUOTED(PACKAGE_LIB_DIR, "${packageprefix}/${packagelibdir}","")
AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${packageprefix}/${packagedatadir}","")
AC_DEFINE_UNQUOTED(SYSTEM_DATA_DIR, "${packageprefix}/${systemdatadir}","")
AC_DEFINE_UNQUOTED(PACKAGE_DOC_DIR, "${packageprefix}/${packagedocdir}","")
AC_DEFINE_UNQUOTED(PACKAGE_PIXMAPS_DIR, "${packageprefix}/${packagepixmapsdir}","")
AC_DEFINE_UNQUOTED(SYSTEM_PIXMAPS_DIR, "${packageprefix}/${systempixmapsdir}","")
AC_DEFINE_UNQUOTED(PACKAGE_SOURCE_DIR, "${packagesrcdir}","")

AC_OUTPUT([
Makefile
include/Makefile
include/mapnik/Makefile
agg/Makefile
agg/src/Makefile
agg/include/Makefile
bindings/Makefile
bindings/python/Makefile
bindings/python/mapnik/Makefile
bindings/python/mapnik/ogcserver/Makefile
bindings/python/mapnik/paths.py
plugins/Makefile
plugins/input/Makefile
plugins/input/gdal/Makefile
plugins/input/postgis/Makefile
plugins/input/raster/Makefile
plugins/input/shape/Makefile
plugins/input/osm/Makefile
plugins/input/ogr/Makefile
plugins/input/sqlite/Makefile
plugins/input/kismet/Makefile
src/Makefile
mapnik.pc
mapnik-uninstalled.pc
])

echo
echo "Build configuration:"
echo "--------------------"
echo
echo "Library support:"
echo "cairo ......................... $enable_cairo" 
echo "build included agg library..... $enable_included_agg"
echo "libxml2 loader ................ $enable_libxml2"
echo
echo "Plugin support:"
echo "build plugin (input/postgis)... $found_postgresql" 
echo "build plugin (input/gdal)...... $found_gdal" 
echo "build plugin (input/ogr)....... $found_gdal" 
echo "build plugin (input/osm)....... $enable_libxml2" 
echo "build plugin (input/sqlite).... $found_sqlite3" 
echo
echo "Debugging support:"
echo "debugger (gdb)................. $enable_debug" 
echo "profiling (gprof).............. $enable_profiling"
echo "tracing (log output)........... $enable_tracing"

dnl echo "build shapeindex application... $ax_cv_boost_program_options" 

echo
