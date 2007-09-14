AC_DEFUN([AX_CHECK_TIFF],
[

dnl libtiff check function

AC_CHECK_LIB(tiff,
             TIFFClose,
             [TIFF_LIBS="-ltiff" link_tiff="yes"],
             [link_tiff="no"]
            )

if test "x$link_tiff" != "xno"; then
  AC_CHECK_HEADER(tiff.h,
                  [TIFF_CFLAGS="" link_tiff="yes"],
                  [link_tiff="no"]
                 )
fi

if test "x$link_tiff" = "xno"; then
  AC_MSG_ERROR(Could not link against libtiff !)
fi

AC_SUBST(TIFF_LIBS)
AC_SUBST(TIFF_CFLAGS)

])dnl

