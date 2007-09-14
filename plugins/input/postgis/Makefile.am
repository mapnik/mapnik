
if BUILD_POSTGRESQL

pkglib_LTLIBRARIES = \
	postgis.input.la

postgis_input_la_SOURCES = \
	postgis.cpp          \
	postgisfs.cpp

postgis_input_la_LIBADD = \
	${POSTGRESQL_LDFLAGS}

postgis_input_la_CXXFLAGS = \
	${POSTGRESQL_CFLAGS} \
	-I../../../include

postgis_input_la_LDFLAGS = \
	-module \
	-avoid-version

endif

## File created by the gnome-build tools
