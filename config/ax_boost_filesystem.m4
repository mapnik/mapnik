dnl @synopsis AX_BOOST_FILESYSTEM
dnl
dnl Test for Filesystem library from the Boost C++ libraries. The macro
dnl requires a preceding call to AX_BOOST_BASE. Further documentation
dnl is available at <http://randspringer.de/boost/index.html>.
dnl
dnl This macro calls:
dnl
dnl   AC_SUBST(BOOST_FILESYSTEM_LIB)
dnl
dnl And sets:
dnl
dnl   HAVE_BOOST_FILESYSTEM
dnl
dnl @category InstalledPackages
dnl @category Cxx
dnl @author Thomas Porschberg <thomas@randspringer.de>
dnl @author Michael Tindal <mtindal@paradoxpoint.com>
dnl @version 2006-06-15
dnl @license AllPermissive

AC_DEFUN([AX_BOOST_FILESYSTEM],
[
	AC_ARG_WITH([boost-filesystem],
	AS_HELP_STRING([--with-boost-filesystem@<:@=special-lib@:>@],
                   [use the Filesystem library from boost - it is possible to specify a certain library for the linker
                        e.g. --with-boost-filesystem=boost_filesystem-gcc-mt ]),
        [
        if test "$withval" = "no"; then
			want_boost="no"
        elif test "$withval" = "yes"; then
            want_boost="yes"
            ax_boost_user_filesystem_lib=""
        else
		    want_boost="yes"
        	ax_boost_user_filesystem_lib="$withval"
		fi
        ],
        [want_boost="yes"]
	)

	if test "x$want_boost" = "xyes"; then
        AC_REQUIRE([AC_PROG_CC])
		CPPFLAGS_SAVED="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
		export CPPFLAGS

		LDFLAGS_SAVED="$LDFLAGS"
		LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
		export LDFLAGS

        AC_CACHE_CHECK(whether the Boost::Filesystem library is available,
					   ax_cv_boost_filesystem,
        [AC_LANG_PUSH([C++])
         AC_COMPILE_IFELSE(AC_LANG_PROGRAM([[@%:@include <boost/filesystem/path.hpp>]],
                                   [[using namespace boost::filesystem;
                                   path my_path( "foo/bar/data.txt" );
                                   return 0;]]),
            				       ax_cv_boost_filesystem=yes, ax_cv_boost_filesystem=no)
         AC_LANG_POP([C++])
		])
		if test "x$ax_cv_boost_filesystem" = "xyes"; then
			AC_DEFINE(HAVE_BOOST_FILESYSTEM,,[define if the Boost::Filesystem library is available])
			BN=boost_filesystem
            if test "x$ax_boost_user_filesystem_lib" = "x"; then
    			for ax_lib in $BN $BN-$CC $BN-$CC-mt $BN-$CC-mt-s $BN-$CC-s \
                              lib$BN lib$BN-$CC lib$BN-$CC-mt lib$BN-$CC-mt-s lib$BN-$CC-s \
                              $BN-mgw $BN-mgw $BN-mgw-mt $BN-mgw-mt-s $BN-mgw-s ; do
				    AC_CHECK_LIB($ax_lib, main,
                                 [BOOST_FILESYSTEM_LIB="-l$ax_lib" AC_SUBST(BOOST_FILESYSTEM_LIB) link_filesystem="yes" break],
                                 [link_filesystem="no"])
  				done
            else
               for ax_lib in $ax_boost_user_filesystem_lib $BN-$ax_boost_user_filesystem_lib; do
				      AC_CHECK_LIB($ax_lib, main,
                                   [BOOST_FILESYSTEM_LIB="-l$ax_lib" AC_SUBST(BOOST_FILESYSTEM_LIB) link_filesystem="yes" break],
                                   [link_filesystem="no"])
                  done

            fi
			if test "x$link_filesystem" = "xno"; then
				AC_MSG_ERROR(Could not link against $ax_lib !)
			fi
		fi

		CPPFLAGS="$CPPFLAGS_SAVED"
    	LDFLAGS="$LDFLAGS_SAVED"
	fi
])
