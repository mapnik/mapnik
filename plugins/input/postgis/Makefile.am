pkglibdir = $(libdir)/mapnik/input

if HAVE_POSTGRESQL

pkglib_LTLIBRARIES = \
	postgis.la

postgis_la_SOURCES = \
	connection.hpp\
	connection_manager.hpp\
	corsorresultset.hpp\
	postgis.cpp\
	postgis.hpp\
	postgisfs.cpp\
	property_index.hpp\
	resultset.hpp

postgis_la_LIBADD = \
	${POSTGRESQL_LDFLAGS}

postgis_la_CXXFLAGS = \
	-Wall \
	${PROFILING_CFLAGS} \
	${TRACING_CFLAGS} \
	${POSTGRESQL_CFLAGS} \
	-I../../../include

postgis_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

endif

## File created by the gnome-build tools
