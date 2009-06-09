pkglibdir = $(libdir)/mapnik/input

if HAVE_GDAL

pkglib_LTLIBRARIES = \
	ogr.la

ogr_la_SOURCES = \
	ogr_datasource.cpp\
	ogr_datasource.hpp\
	ogr_featureset.cpp\
	ogr_featureset.hpp\
	ogr_converter.cpp\
	ogr_converter.hpp\
	ogr_index.hpp\
	ogr_index_featureset.hpp\
	ogr_feature_ptr.hpp

ogr_la_LIBADD = \
  ${GDAL_LDFLAGS}

ogr_la_CXXFLAGS = \
  -Wall \
  ${PROFILING_CFLAGS} \
  ${TRACING_CFLAGS} \
  ${GDAL_CFLAGS} \
  -I../../../include

ogr_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

endif

## File created by the gnome-build tools
