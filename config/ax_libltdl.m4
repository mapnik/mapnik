AC_DEFUN([AX_CHECK_LTDL],
[

dnl libjpeg check function

AC_CHECK_LIB(ltdl,
             lt_dlopen,
             [LTDL_LIBS="-lltdl" link_ltdl="ltdl"],
             [link_jpeg="no"]
            )

if test "x$link_ltdl" != "xno"; then
  AC_CHECK_HEADER(ltdl.h,
                  [LTDL_CFLAGS="" link_ltdl="ltdl"],
                  [link_ltdl="no"]
                 )
fi

if test "x$link_ltdl" = "xno"; then
  AC_MSG_ERROR(Could not link against libltdl !)
fi

AC_SUBST(LTDL_LIBS)
AC_SUBST(LTDL_CFLAGS)

])dnl

