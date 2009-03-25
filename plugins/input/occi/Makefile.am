pkglibdir = $(libdir)/mapnik/input

if HAVE_OCCI

pkglib_LTLIBRARIES = \
	occi.la

occi_la_SOURCES = \
	occi_datasource.cpp\
	occi_datasource.hpp\
	occi_featureset.cpp\
	occi_featureset.hpp\
	spatial_classesm.cpp\
	spatial_classesm.h\
	spatial_classeso.cpp\
	spatial_classesh.h\
	occi_types.cpp\
	occi_types.hpp

occi_la_LIBADD = \
  ${OCCI_LDFLAGS}

occi_la_CXXFLAGS = \
  -Wall \
  ${PROFILING_CFLAGS} \
  ${TRACING_CFLAGS} \
  ${OCCI_CFLAGS} \
  -I../../../include

occi_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

endif

## File created by the gnome-build tools
