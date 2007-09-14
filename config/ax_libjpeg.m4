AC_DEFUN([AX_CHECK_JPEG],
[

dnl libjpeg check function

AC_CHECK_LIB(jpeg,
             jpeg_set_defaults,
             [JPEG_LIBS="-ljpeg" link_jpeg="yes"],
             [link_jpeg="no"]
            )

if test "x$link_jpeg" != "xno"; then
  AC_CHECK_HEADER(jpeglib.h,
                  [JPEG_CFLAGS="" link_jpeg="yes"],
                  [link_jpeg="no"]
                 )
fi

if test "x$link_jpeg" = "xno"; then
  AC_MSG_ERROR(Could not link against libjpeg !)
fi

AC_SUBST(JPEG_LIBS)
AC_SUBST(JPEG_CFLAGS)

])dnl

