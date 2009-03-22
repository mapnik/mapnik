
pkglib_LTLIBRARIES = \
	shape.la

shape_la_SOURCES = \
	dbffile.cpp\
	dbffile.hpp\
	shape.cpp\
	shape.hpp\
	shape_featureset.cpp\
	shape_featureset.hpp\
	shapefile.cpp\
	shapefile.hpp\
	shape_index_featureset.cpp\
	shape_index_featureset.hpp\
	shape_io.cpp\
	shape_io.hpp\
	shp_index.hpp

shape_la_CXXFLAGS = \
  -Wall \
  ${PROFILING_CFLAGS} \
  ${TRACING_CFLAGS} \
	-I../../../include

shape_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

## File created by the gnome-build tools

