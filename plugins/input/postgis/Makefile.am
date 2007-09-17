
if BUILD_POSTGRESQL

pkglib_LTLIBRARIES = \
	postgis.la

postgis_la_SOURCES = \
	postgis.cpp          \
	postgisfs.cpp

postgis_la_LIBADD = \
	${POSTGRESQL_LDFLAGS}

postgis_la_CXXFLAGS = \
	${POSTGRESQL_CFLAGS} \
	-I../../../include

postgis_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

endif

## File created by the gnome-build tools
