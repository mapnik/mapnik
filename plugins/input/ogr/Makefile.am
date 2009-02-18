if HAVE_GDAL

pkglib_LTLIBRARIES = \
	ogr.la

ogr_la_SOURCES = \
	ogr_datasource.cpp\
	ogr_featureset.cpp

ogr_la_LIBADD = \
        ${GDAL_LDFLAGS}

ogr_la_CXXFLAGS = \
        ${GDAL_CFLAGS} \
        -I../../../include

ogr_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

endif

## File created by the gnome-build tools
