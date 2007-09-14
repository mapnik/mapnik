dnl @synopsis AX_BOOST_REGEX
dnl
dnl Test for Regex library from the Boost C++ libraries. The macro
dnl requires a preceding call to AX_BOOST_BASE. Further documentation
dnl is available at <http://randspringer.de/boost/index.html>.
dnl
dnl This macro calls:
dnl
dnl   AC_SUBST(BOOST_REGEX_LIB)
dnl
dnl And sets:
dnl
dnl   HAVE_BOOST_REGEX
dnl
dnl @category InstalledPackages
dnl @category Cxx
dnl @author Thomas Porschberg <thomas@randspringer.de>
dnl @author Michael Tindal <mtindal@paradoxpoint.com>
dnl @version 2006-06-15
dnl @license AllPermissive

AC_DEFUN([AX_BOOST_REGEX],
[
	AC_ARG_WITH([boost-regex],
	AS_HELP_STRING([--with-boost-regex@<:@=special-lib@:>@],
                   [use the Regex library from boost - it is possible to specify a certain library for the linker
                        e.g. --with-boost-regex=boost_regex-gcc-mt-d-1_33_1 ]),
        [
        if test "$withval" = "no"; then
			want_boost="no"
        elif test "$withval" = "yes"; then
            want_boost="yes"
            ax_boost_user_regex_lib=""
        else
		    want_boost="yes"
        	ax_boost_user_regex_lib="$withval"
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

        AC_CACHE_CHECK(whether the Boost::Regex library is available,
					   ax_cv_boost_regex,
        [AC_LANG_PUSH([C++])
			 AC_COMPILE_IFELSE(AC_LANG_PROGRAM([[@%:@include <boost/regex.hpp>
												]],
                                   [[boost::regex r(); return 0;]]),
                   ax_cv_boost_regex=yes, ax_cv_boost_regex=no)
         AC_LANG_POP([C++])
		])
		if test "x$ax_cv_boost_regex" = "xyes"; then
			AC_DEFINE(HAVE_BOOST_REGEX,,[define if the Boost::Regex library is available])
			BN=boost_regex
            if test "x$ax_boost_user_regex_lib" = "x"; then
				for ax_lib in $BN $BN-$CC $BN-$CC-mt $BN-$CC-mt-s $BN-$CC-s \
                              lib$BN lib$BN-$CC lib$BN-$CC-mt lib$BN-$CC-mt-s lib$BN-$CC-s \
                              $BN-mgw $BN-mgw $BN-mgw-mt $BN-mgw-mt-s $BN-mgw-s ; do
				    AC_CHECK_LIB($ax_lib, main, [BOOST_REGEX_LIB="-l$ax_lib" AC_SUBST(BOOST_REGEX_LIB) link_regex="yes" break],
                                 [link_regex="no"])
  				done
            else
               for ax_lib in $ax_boost_user_regex_lib $BN-$ax_boost_user_regex_lib; do
				      AC_CHECK_LIB($ax_lib, main,
                                   [BOOST_REGEX_LIB="-l$ax_lib" AC_SUBST(BOOST_REGEX_LIB) link_regex="yes" break],
                                   [link_regex="no"])
               done
            fi
			if test "x$link_regex" = "xno"; then
				AC_MSG_ERROR(Could not link against $ax_lib !)
			fi
		fi

		CPPFLAGS="$CPPFLAGS_SAVED"
    	LDFLAGS="$LDFLAGS_SAVED"
	fi
])
