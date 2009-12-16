pkglibdir = $(libdir)/mapnik/input

if HAVE_GDAL

pkglib_LTLIBRARIES = \
	rasterlite.la

rasterlite_la_SOURCES = \
	rasterlite_datasource.cpp\
	rasterlite_datasource.hpp\
	rasterlite_featureset.cpp\
	rasterlite_featureset.hpp\
	rasterlite_include.hpp

rasterlite_la_LIBADD = \
        ${GDAL_LDFLAGS}

rasterlite_la_CXXFLAGS = \
  -Wall \
  ${PROFILING_CFLAGS} \
  ${TRACING_CFLAGS} \
  ${GDAL_CFLAGS} \
  -I../../../include

rasterlite_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

endif

## File created by the gnome-build tools
