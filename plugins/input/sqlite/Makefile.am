if HAVE_SQLITE

pkglib_LTLIBRARIES = \
	sqlite.la

sqlite_la_SOURCES = \
	sqlite_datasource.cpp\
	sqlite_featureset.cpp

sqlite_la_LIBADD = \
        ${SQLITE_LDFLAGS}

sqlite_la_CXXFLAGS = \
        ${SQLITE_CFLAGS} \
        -I../../../include

sqlite_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

endif

## File created by the gnome-build tools
