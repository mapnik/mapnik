#Copyright (C) 2005 by Artem Pavlenko

lib_LTLIBRARIES = libmapnik.la
nobase_include_HEADERS = attribute_container.hh attribute.hh color.hh config.hh coord.hh coord_array.hh vertex_vector.hh ctrans.hh datasource.hh datasource_cache.hh factory.hh feature.hh filter.hh envelope.hh gamma.hh geometry.hh geom_util.hh graphics.hh image_data.hh image_reader.hh image_util.hh layer.hh line_aa.hh map.hh mapnik.hh memory.hh params.hh plugin.hh line_symbolizer.hh polygon_symbolizer.hh image_symbolizer.hh pool.hh ptr.hh raster_feature.hh raster.hh render.hh scanline_aa.hh scanline.hh style.hh style_cache.hh style_factory.hh symbolizer.hh text.hh utils.hh vector_feature.hh vertex.hh vertex_transform.hh wkb.hh 

libmapnik_la_SOURCES = attribute_container.cc attribute_container.hh attribute.hh color.cc color.hh coord.hh coord_array.hh vertex_vector.hh ctrans.hh datasource.hh datasource_cache.cc datasource_cache.hh envelope.cc envelope.hh factory.hh feature.cc feature.hh filter.hh geometry.hh gamma.hh geom_util.hh graphics.cc graphics.hh image_data.hh image_reader.hh image_util.cc image_util.hh layer.cc layer.hh line_aa.cc line_aa.hh map.cc map.hh mapnik.hh memory.cc memory.hh params.cc params.hh plugin.cc plugin.hh png_reader.cc line_symbolizer.hh polygon_symbolizer.hh image_symbolizer.hh pool.hh ptr.hh raster.cc raster_feature.cc raster_feature.hh raster.hh render.cc render.hh scanline_aa.cc scanline_aa.hh scanline.cc scanline.hh style.cc style.hh style_cache.cc style_cache.hh style_factory.hh symbolizer.hh text.cc text.hh utils.hh vector_feature.cc vector_feature.hh vertex.hh vertex_transform.hh wkb.cc wkb.hh

if BUILD_TIFF_READER
   libmapnik_la_SOURCES += tiff_reader.cc 
endif

libmapnik_la_CPPFLAGS = $(FREETYPE2_CFLAGS)
libmapnik_la_CPPFLAGS += @INCLTDL@
libmapnik_la_LIBADD = @LIBLTDL@ -lpthread -lpng $(FREETYPE2_LIBS) -lz -lm
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

