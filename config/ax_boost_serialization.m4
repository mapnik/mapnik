dnl @synopsis AX_BOOST_SERIALIZATION
dnl
dnl Test for Serialization library from the Boost C++ libraries. The
dnl macro requires a preceding call to AX_BOOST_BASE. Further
dnl documentation is available at
dnl <http://randspringer.de/boost/index.html>.
dnl
dnl This macro calls:
dnl
dnl   AC_SUBST(BOOST_SERIALIZATION_LIB)
dnl
dnl And sets:
dnl
dnl   HAVE_BOOST_SERIALIZATION
dnl
dnl @category InstalledPackages
dnl @category Cxx
dnl @author Thomas Porschberg <thomas@randspringer.de>
dnl @version 2006-06-15
dnl @license AllPermissive

AC_DEFUN([AX_BOOST_SERIALIZATION],
[
	AC_ARG_WITH([boost-serialization],
	AS_HELP_STRING([--with-boost-serialization@<:@=special-lib@:>@],
                   [use the Serialization library from boost - it is possible to specify a certain library for the linker
                        e.g. --with-boost-serialization=boost_serialization-gcc-mt-d-1_33_1 ]),
        [
        if test "$withval" = "no"; then
			want_boost="no"
        elif test "$withval" = "yes"; then
            want_boost="yes"
            ax_boost_user_serialization_lib=""
        else
		    want_boost="yes"
        	ax_boost_user_serialization_lib="$withval"
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

        AC_CACHE_CHECK(whether the Boost::Serialization library is available,
					   ax_cv_boost_serialization,
        [AC_LANG_PUSH([C++])
			 AC_COMPILE_IFELSE(AC_LANG_PROGRAM([[@%:@include <fstream>
												 @%:@include <boost/archive/text_oarchive.hpp>
                                                 @%:@include <boost/archive/text_iarchive.hpp>
												]],
                                   [[std::ofstream ofs("filename");
									boost::archive::text_oarchive oa(ofs);
									 return 0;
                                   ]]),
                   ax_cv_boost_serialization=yes, ax_cv_boost_serialization=no)
         AC_LANG_POP([C++])
		])
		if test "x$ax_cv_boost_serialization" = "xyes"; then
			AC_DEFINE(HAVE_BOOST_SERIALIZATION,,[define if the Boost::Serialization library is available])
			BN=boost_serialization
            if test "x$ax_boost_user_serialization_lib" = "x"; then
				for ax_lib in $BN $BN-$CC $BN-$CC-mt $BN-$CC-mt-s $BN-$CC-s \
                              lib$BN lib$BN-$CC lib$BN-$CC-mt lib$BN-$CC-mt-s lib$BN-$CC-s \
                              $BN-mgw $BN-mgw $BN-mgw-mt $BN-mgw-mt-s $BN-mgw-s ; do
				    AC_CHECK_LIB($ax_lib, main,
                                 [BOOST_SERIALIZATION_LIB="-l$ax_lib" AC_SUBST(BOOST_SERIALIZATION_LIB) link_serialization="yes" break],
                                 [link_serialization="no"])
  				done
            else
               for ax_lib in $ax_boost_user_serialization_lib $BN-$ax_boost_user_serialization_lib; do
				      AC_CHECK_LIB($ax_lib, main,
                                   [BOOST_SERIALIZATION_LIB="-l$ax_lib" AC_SUBST(BOOST_SERIALIZATION_LIB) link_serialization="yes" break],
                                   [link_serialization="no"])
                  done

            fi
			if test "x$link_serialization" = "xno"; then
				AC_MSG_ERROR(Could not link against $ax_lib !)
			fi
		fi

		CPPFLAGS="$CPPFLAGS_SAVED"
    	LDFLAGS="$LDFLAGS_SAVED"
	fi
])
