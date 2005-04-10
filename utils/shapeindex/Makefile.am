bin_PROGRAMS = shapeindex
shapeindex_SOURCES = quadtree.cc quadtree.hh shapeindex.cc
shapeindex_CXXFLAGS = -I$(srcdir)/../../include -I$(srcdir)/../../src/shape
SHAPELIB  = ../../src/shape/.libs/shape.a
MAPNIKLIB = ../../src/.libs/libmapnik.a
shapeindex_LDADD = $(SHAPELIB) $(MAPNIKLIB)
shapeindex_LDFLAGS =
