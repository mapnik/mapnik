#  
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.59)
AC_INIT([mapnik],[0.1])
AC_CONFIG_HEADERS([src/config.hh])
AC_REVISION([$Revision: 1.1 $])
AC_CONFIG_SRCDIR([src/params.cc])
AM_INIT_AUTOMAKE([dist-bzip2])

AC_LIBTOOL_DLOPEN
AC_CONFIG_SUBDIRS(libltdl utils/shapeindex)

AC_LIBLTDL_CONVENIENCE
AC_PROG_LIBTOOL

AC_SUBST(INCLTDL)
AC_SUBST(LIBLTDL)
# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

AC_HEADER_STDC

# Checks for libraries.
AC_CHECK_LIB(m,floor)

AC_C_BIGENDIAN
# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([string.h sys/time.h unistd.h])
AC_CHECK_HEADER(png.h,,AC_MSG_ERROR(install png-devel))

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor gettimeofday memset pow sqrt])


AC_SUBST([pluginsdir],["${prefix}/plugins"])
AC_DEFINE_UNQUOTED([_DATASOURCE_PLUGINS_DIR],["$pluginsdir"],[datasource plugins dir])

AP_ENABLE_DEBUG
#AP_CHECK_FREETYPE2

# check if shape is enabled
AC_ARG_ENABLE([shape],AC_HELP_STRING([--enable-shape],
                            [enable shape datasource plugin (default is YES)]), [package_shape_enabled=$enableval], [package_shape_enabled=yes])

AC_CACHE_CHECK([whether to enable shape datasource],
               [package_shape_enabled], [package_shape_enabled=yes])

# check if raster datasource is enabled
AC_ARG_ENABLE([raster],AC_HELP_STRING([--enable-raster],
                            [enable raster datasource plugin (default is NO)]), [package_raster_enabled=$enableval], [package_raster_enabled=no])

AC_CACHE_CHECK([whether to enable raster datasource],
               [package_raster_enabled], [package_raster_enabled=no])

#config_files=Makefile po/Makefile m4/Makefile src/Makefile
if test "$package_raster_enabled" = yes ; then
  AP_CHECK_LIBTIFF
  #config_files="${config_files} src/raster/Makefile"
fi

#  check if postgis datasource is enabled 
AC_ARG_ENABLE([postgis],AC_HELP_STRING([--enable-postgis],
                            [enable postgis datasource plugin (default is NO)]), [package_postgis_enabled=$enableval], [package_postgis_enabled=no])

AC_CACHE_CHECK([whether to enable postgis datasource],
               [package_postgis_enabled], [package_postgis_enabled=no])

if test "$package_postgis_enabled" = yes ; then
  AP_CHECK_POSTGRESQL
fi 

AC_CONFIG_FILES([Makefile m4/Makefile src/Makefile src/shape/Makefile src/raster/Makefile src/postgis/Makefile])

AM_CONDITIONAL([BUILD_TIFF_READER],[test "$HAVE_LIBTIFF" = yes])
AM_CONDITIONAL([SHAPE_DATASOURCE],[test "$package_shape_enabled" = yes ])
AM_CONDITIONAL([RASTER_DATASOURCE],[test "$package_raster_enabled" = yes])
AM_CONDITIONAL([POSTGIS_DATASOURCE],[test "$package_postgis_enabled" = yes])

AC_OUTPUT
