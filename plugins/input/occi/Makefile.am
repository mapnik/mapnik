if HAVE_OCCI

pkglib_LTLIBRARIES = \
	occi.la

occi_la_SOURCES = \
	occi_datasource.cpp\
	occi_featureset.cpp\
	spatial_classesm.cpp\
	spatial_classeso.cpp

occi_la_LIBADD = \
        ${OCCI_LDFLAGS}

occi_la_CXXFLAGS = \
        ${OCCI_CFLAGS} \
        -I../../../include

occi_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

endif

## File created by the gnome-build tools
