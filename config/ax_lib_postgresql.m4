dnl @synopsis AX_LIB_POSTGRESQL([MINIMUM-VERSION])
dnl
dnl This macro provides tests of availability of PostgreSQL 'libpq'
dnl library of particular version or newer.
dnl
dnl AX_LIB_POSTGRESQL macro takes only one argument which is optional.
dnl If there is no required version passed, then macro does not run
dnl version test.
dnl
dnl The --with-postgresql option takes one of three possible values:
dnl
dnl no - do not check for PostgreSQL client library
dnl
dnl yes - do check for PostgreSQL library in standard locations
dnl (pg_config should be in the PATH)
dnl
dnl path - complete path to pg_config utility, use this option if
dnl pg_config can't be found in the PATH
dnl
dnl This macro calls:
dnl
dnl   AC_SUBST(POSTGRESQL_CFLAGS)
dnl   AC_SUBST(POSTGRESQL_LDFLAGS)
dnl   AC_SUBST(POSTGRESQL_VERSION)
dnl
dnl And sets:
dnl
dnl   HAVE_POSTGRESQL
dnl
dnl @category InstalledPackages
dnl @category Cxx
dnl @author Mateusz Loskot <mateusz@loskot.net>
dnl @version 2006-07-16
dnl @license AllPermissive

AC_DEFUN([AX_LIB_POSTGRESQL],
[
    AC_ARG_WITH([postgresql],
        AC_HELP_STRING([--with-postgresql=@<:@ARG@:>@],
            [use PostgreSQL library @<:@default=yes@:>@, optionally specify path to pg_config]
        ),
        [
        if test "$withval" = "no"; then
            want_postgresql="no"
        elif test "$withval" = "yes"; then
            want_postgresql="yes"
        else
            want_postgresql="yes"
            PG_CONFIG="$withval"
        fi
        ],
        [want_postgresql="yes"]
    )

    POSTGRESQL_CFLAGS=""
    POSTGRESQL_LDFLAGS=""
    POSTGRESQL_POSTGRESQL=""

    dnl
    dnl Check PostgreSQL libraries (libpq)
    dnl

    if test "$want_postgresql" = "yes"; then

        if test -z "$PG_CONFIG" -o test; then
            AC_PATH_PROG([PG_CONFIG], [pg_config], [no])
        fi

        if test "$PG_CONFIG" != "no"; then
            AC_MSG_CHECKING([for PostgreSQL libraries])

            POSTGRESQL_CFLAGS="-I`$PG_CONFIG --includedir`"
            POSTGRESQL_LDFLAGS="-L`$PG_CONFIG --libdir` -lpq"

            POSTGRESQL_VERSION=`$PG_CONFIG --version | sed -e 's#PostgreSQL ##'`

            AC_DEFINE([HAVE_POSTGRESQL], [1],
                [Define to 1 if PostgreSQL libraries are available])

            found_postgresql="yes"
            AC_MSG_RESULT([yes])
        else
            found_postgresql="no"
            AC_MSG_RESULT([no])
        fi
    fi

    dnl
    dnl Check if required version of PostgreSQL is available
    dnl


    postgresql_version_req=ifelse([$1], [], [], [$1])

    if test "$found_postgresql" = "yes" -a -n "$postgresql_version_req"; then

        AC_MSG_CHECKING([if PostgreSQL version is >= $postgresql_version_req])

        dnl Decompose required version string of PostgreSQL
        dnl and calculate its number representation
        postgresql_version_req_major=`expr $postgresql_version_req : '\([[0-9]]*\)'`
        postgresql_version_req_minor=`expr $postgresql_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
        postgresql_version_req_micro=`expr $postgresql_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$postgresql_version_req_micro" = "x"; then
            postgresql_version_req_micro="0"
        fi

        postgresql_version_req_number=`expr $postgresql_version_req_major \* 1000000 \
                                   \+ $postgresql_version_req_minor \* 1000 \
                                   \+ $postgresql_version_req_micro`

        dnl Decompose version string of installed PostgreSQL
        dnl and calculate its number representation
        postgresql_version_major=`expr $POSTGRESQL_VERSION : '\([[0-9]]*\)'`
        postgresql_version_minor=`expr $POSTGRESQL_VERSION : '[[0-9]]*\.\([[0-9]]*\)'`
        postgresql_version_micro=`expr $POSTGRESQL_VERSION : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$postgresql_version_micro" = "x"; then
            postgresql_version_micro="0"
        fi

        postgresql_version_number=`expr $postgresql_version_major \* 1000000 \
                                   \+ $postgresql_version_minor \* 1000 \
                                   \+ $postgresql_version_micro`

        postgresql_version_check=`expr $postgresql_version_number \>\= $postgresql_version_req_number`
        if test "$postgresql_version_check" = "1"; then
            AC_MSG_RESULT([yes])
        else
            AC_MSG_RESULT([no])
        fi
    fi

    AC_SUBST([POSTGRESQL_VERSION])
    AC_SUBST([POSTGRESQL_CFLAGS])
    AC_SUBST([POSTGRESQL_LDFLAGS])
])
