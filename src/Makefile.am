#Copyright (C) 2005 by Artem Pavlenko

lib_LTLIBRARIES = libmapnik.la

libmapnik_la_SOURCES = color.cc datasource_cache.cc envelope.cc   graphics.cc   image_reader.cc  image_util.cc  layer.cc line_aa.cc map.cc  memory.cc params.cc  plugin.cc png_reader.cc  render.cc  scanline_aa.cc  scanline.cc  style.cc  style_cache.cc text.cc  wkb.cc

if BUILD_TIFF_READER
   libmapnik_la_SOURCES += tiff_reader.cc
endif

SPIRIT_ROOT = "/home/artem/projects/spirit_1_8_2"

libmapnik_la_CXXFLAGS = -I$(top_srcdir)/include $(FREETYPE2_CFLAGS) 
libmapnik_la_CXXFLAGS += -I$//home/artem/projects/spirit_1_8_2 -I$//home/artem/projects/spirit_1_8_2/miniboost
libmapnik_la_CXXFLAGS += -I$//home/artem/projects/agg2/include
libmapnik_la_CXXFLAGS += @INCLTDL@
libmapnik_la_LIBADD = @LIBLTDL@ -lpthread -lpng -ljpeg $(FREETYPE2_LIBS) -lz -lm -L$//home/artem/projects/agg2/src -lagg 
libmapnik_la_LDFLAGS= -version-info 0:0:0  $(TIFF_LDFLAGS)

if SHAPE_DATASOURCE
  SHAPE_DATASOURCE_DIR = shape
endif 

if RASTER_DATASOURCE
  RASTER_DATASOURCE_DIR = raster
endif 

if POSTGIS_DATASOURCE
  POSTGIS_DATASOURCE_DIR = postgis
endif

SUBDIRS = $(SHAPE_DATASOURCE_DIR) $(RASTER_DATASOURCE_DIR) $(POSTGIS_DATASOURCE_DIR)

