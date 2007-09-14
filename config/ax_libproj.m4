AC_DEFUN([AX_CHECK_PROJ],
[

dnl libjpeg check function

AC_CHECK_LIB(proj,
             pj_init_plus,
             [PROJ_LIBS="-lproj" link_proj="yes"],
             [link_proj="no"]
            )

if test "x$link_proj" != "xno"; then
  AC_CHECK_HEADER(proj_api.h,
                  [PROJ_CFLAGS="" link_proj="yes"],
                  [link_proj="no"]
                 )
fi

if test "x$link_proj" = "xno"; then
  AC_MSG_ERROR(Could not link against libproj !)
fi

AC_SUBST(PROJ_LIBS)
AC_SUBST(PROJ_CFLAGS)

])dnl

