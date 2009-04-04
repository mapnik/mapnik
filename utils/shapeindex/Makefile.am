if HAVE_BOOST_PROGRAM_OPTIONS

bin_PROGRAMS = \
	shapeindex

shapeindex_SOURCES = \
	quadtree.hpp\
	shapeindex.cpp \
	../../plugins/input/shape/shapefile.cpp

shapeindex_LDFLAGS = \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	../../src/libmapnik.la \
	${AGG_LIBS}

shapeindex_DEPENDENCIES = \
	../../src/libmapnik.la

shapeindex_CXXFLAGS = \
        ${PROFILING_CFLAGS} \
        ${TRACING_CFLAGS} \
	-I../../include \
	-I../../plugins/input/shape \
	${AGG_CFLAGS}

endif

## File created by the gnome-build tools

