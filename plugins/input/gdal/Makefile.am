pkglibdir = $(libdir)/mapnik/input

if HAVE_GDAL

pkglib_LTLIBRARIES = \
	gdal.la

gdal_la_SOURCES = \
	gdal_datasource.cpp\
	gdal_datasource.hpp\
	gdal_featureset.cpp\
	gdal_featureset.hpp

gdal_la_LIBADD = \
        ${GDAL_LDFLAGS}

gdal_la_CXXFLAGS = \
  -Wall \
  ${PROFILING_CFLAGS} \
  ${TRACING_CFLAGS} \
  ${GDAL_CFLAGS} \
  -I../../../include

gdal_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

endif

## File created by the gnome-build tools
