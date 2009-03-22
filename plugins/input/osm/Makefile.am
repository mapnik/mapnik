
if HAVE_LIBXML2

pkglib_LTLIBRARIES = \
	osm.la

osm_la_SOURCES = \
	basiccurl.cpp\
	basiccurl.h\
	dataset_deliverer.cpp\
	dataset_deliverer.h\
	osm.cpp \
	osm.h\
	osm_datasource.cpp \
	osm_datasource.hpp \
	osm_featureset.cpp \
	osm_featureset.hpp \
	osmparser.cpp\
	osmparser.h\
	osmtagtypes.h\
	renderer.cpp

osm_la_CXXFLAGS = \
  -Wall \
  ${PROFILING_CFLAGS} \
  ${TRACING_CFLAGS} \
	-I../../../include \
	${LIBXML2_CFLAGS}

osm_la_LIBADD = \
	${LIBXML2_LIBS}

osm_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

endif

## File created by the gnome-build tools

