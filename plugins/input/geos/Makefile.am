pkglibdir = $(libdir)/mapnik/input

if HAVE_GEOS

pkglib_LTLIBRARIES = \
	geos.la

geos_la_SOURCES = \
	geos_datasource.cpp \
	geos_datasource.hpp \
	geos_featureset.cpp \
	geos_featureset.hpp \
	geos_converter.cpp \
	geos_converter.hpp \
	geos_feature_ptr.hpp

geos_la_LIBADD = \
  ${GEOS_LDFLAGS}

geos_la_CXXFLAGS = \
  -Wall \
  ${PROFILING_CFLAGS} \
  ${TRACING_CFLAGS} \
  ${GEOS_CFLAGS} \
  -I../../../include

geos_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

endif

## File created by the gnome-build tools
