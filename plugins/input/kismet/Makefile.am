pkglibdir = $(libdir)/mapnik/input

pkglib_LTLIBRARIES = \
	kismet.la

kismet_la_SOURCES = \
	kismet_datasource.cpp\
	kismet_datasource.hpp\
	kismet_featureset.cpp\
	kismet_featureset.hpp\
	kismet_types.hpp

kismet_la_LIBADD = 

kismet_la_CXXFLAGS = \
  -Wall \
  ${PROFILING_CFLAGS} \
  ${TRACING_CFLAGS} \
  -I../../../include

kismet_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

## File created by the gnome-build tools
