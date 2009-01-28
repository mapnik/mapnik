if HAVE_OGR

pkglib_LTLIBRARIES = \
	ogr.la

ogr_la_SOURCES = \
	ogr_datasource.cpp\
	ogr_featureset.cpp

ogr_la_LIBADD = \
        ${OGR_LDFLAGS}

ogr_la_CXXFLAGS = \
        ${OGR_CFLAGS} \
        -I../../../include

ogr_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

endif

## File created by the gnome-build tools
