
if HAVE_LIBXML2

pkglib_LTLIBRARIES = \
	osm.la

osm_la_SOURCES = \
	osm.cpp \
	osm_datasource.cpp \
	osm_featureset.cpp \
	osmparser.cpp	

osm_la_CXXFLAGS = \
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

