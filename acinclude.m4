# debug build of libxmap
AC_DEFUN([AP_ENABLE_DEBUG],
[AC_ARG_ENABLE(debug,
[AC_HELP_STRING([--enable-debug],[build a debug version of libxmap])],
[xmap_debug_build=yes
CXXFLAGS="-Wall -g -O0"],
[xmap_debug_build=no])
])

#check for freetype2 installation
AC_DEFUN([AP_CHECK_FREETYPE2_NEW],
	[AC_MSG_CHECKING([for freetype2 installation])
	PKG_CHECK_MODULES(freetype2, freetype2 >= 7.0.1)
	AC_SUBST(FREETYPE2_CFLAGS)
 AC_SUBST(FREETYPE2_LIBS)]
)

AC_DEFUN([AP_CHECK_FREETYPE2],
[AC_MSG_CHECKING([for freetype2 installation])
 AC_ARG_WITH(freetype2,
	[AC_HELP_STRING([--with-freetype2=DIR],[prefix of freetype2 installation. e.g /usr/local or /usr])],
	
 	[FREETYPE2_PREFIX=$with_freetype2],
	AC_MSG_ERROR([You must call configure with the --with-freetype2 option.
	This tells configure where to find the freetype2 library and headers.
	e.g. --with-freetype2=/usr/local or --with-freetype2=/usr])
)

AC_MSG_RESULT(yes)
AC_SUBST(FREETYPE2_PREFIX)
FREETYPE2_LIBS="-L${FREETYPE2_PREFIX}/lib -lfreetype"
FREETYPE2_CFLAGS="-I${FREETYPE2_PREFIX}/include/freetype2"
AC_SUBST(FREETYPE2_LIBS)
AC_SUBST(FREETYPE2_CFLAGS)
])

# check for postgresql installation
AC_DEFUN([AP_CHECK_POSTGRESQL],
[AC_MSG_CHECKING([for postgresql installation])
 AC_ARG_WITH(postgresql,
	[AC_HELP_STRING([--with-postgresql=DIR],[prefix of postgresql installation. e.g /usr/local or /usr])],
 	[POSTGRESQL_PREFIX=$with_postgresql],
	AC_MSG_ERROR([You must call configure with the --with-postgresql option.
	This tells configure where to find the postgresql library and headers.
	e.g. --with-postgresql=/usr/local or --with-postgresql=/usr])
)
AC_SUBST(POSTGRESQL_PREFIX)
POSTGRESQL_LIBS="-L${POSTGRESQL_PREFIX}/lib -lpq"
POSTGRESQL_CFLAGS="-I${POSTGRESQL_PREFIX}/include"
AC_SUBST(POSTGRESQL_LIBS)
AC_SUBST(POSTGRESQL_CFLAGS)
])

# check for libtiff installation
AC_DEFUN([AP_CHECK_LIBTIFF],
[AC_MSG_CHECKING([for libtiff installation])
 AC_ARG_WITH([tiff-dir],[AC_HELP_STRING(
	[--with-tiff-dir=DIR],[directory to look for tiff libraries and headers])],
	[if test "$withval" != no ; then	 
	 AC_MSG_RESULT(yes)
	  TIFF_HOME="$withval"	
	  if test "$withval" = "yes" ; then
	    TIFF_HOME="/usr/local"
	  fi
	  AC_CHECK_LIB(tiff,TIFFReadRGBAStrip,
	   AC_CHECK_LIB(tiff,TIFFReadRGBATile, [HAVE_LIBTIFF="yes"] [AC_SUBST([TIFF_LDFLAGS],["-L${TIFF_HOME} -ltiff"])],
	   [AC_MSG_RESULT(no)],)
	  ,[AC_MSG_RESULT(no)],)
	 else
	   AC_MSG_RESULT(no)
	   AC_MSG_ERROR([sorry, libtiff needed try --with-tiff-dir=<DIR>])
	 fi 
	],
	[
	  AC_MSG_RESULT(yes)
          AC_CHECK_LIB(tiff,TIFFReadRGBAStrip,
	    [AC_CHECK_LIB(tiff,TIFFReadRGBATile,[HAVE_LIBTIFF="yes"] [AC_SUBST([TIFF_LDFLAGS],["-ltiff"])],
	   [AC_MSG_ERROR([sorry, libtiff needed try --with-tiff-dir=<DIR>])],)]
	  ,[AC_MSG_ERROR([sorry, libtiff needed try --with-tiff-dir=<DIR>])],)
	])
])

