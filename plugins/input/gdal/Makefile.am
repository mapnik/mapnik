
pkglib_LTLIBRARIES = \
	gdal.la

gdal_la_SOURCES = \
	gdal_datasource.cpp\
	gdal_featureset.cpp

gdal_la_LIBADD = \
        ${GDAL_LDFLAGS}

gdal_la_CXXFLAGS = \
        ${GDAL_CFLAGS} \
        -I../../../include

gdal_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shrext .input

## File created by the gnome-build tools
