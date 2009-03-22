
pkglib_LTLIBRARIES = \
	raster.la

raster_la_SOURCES = \
	raster_datasource.cpp\
	raster_datasource.hpp\
	raster_featureset.cpp\
	raster_featureset.hpp\
	raster_info.cpp\
	raster_info.hpp

raster_la_CXXFLAGS = \
  -Wall \
  ${PROFILING_CFLAGS} \
  ${TRACING_CFLAGS} \
	-I../../../include

raster_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

## File created by the gnome-build tools

