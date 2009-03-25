pkglibdir = $(libdir)/mapnik/input

if HAVE_SQLITE3

pkglib_LTLIBRARIES = \
	sqlite.la

sqlite_la_SOURCES = \
	sqlite_datasource.cpp\
	sqlite_datasource.hpp\
	sqlite_featureset.cpp\
	sqlite_featureset.hpp\
	sqlite_types.hpp

sqlite_la_LIBADD = \
  ${SQLITE3_LDFLAGS}

sqlite_la_CXXFLAGS = \
  -Wall \
  ${PROFILING_CFLAGS} \
  ${TRACING_CFLAGS} \
  ${SQLITE3_CFLAGS} \
  -I../../../include

sqlite_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

endif

## File created by the gnome-build tools
