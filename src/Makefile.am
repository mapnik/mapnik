## Process this file with automake to produce Makefile.in

## Created by Anjuta

INCLUDES = 

AM_CXXFLAGS =\
	 -Wall 

lib_LTLIBRARIES = \
	libmapnik.la

if HAVE_LIBXML2
libxml2_src = \
	libxml2_loader.cpp
endif

if HAVE_CAIRO
cairo_src = \
	cairo_renderer.cpp 
endif


libmapnik_la_SOURCES = \
	agg_renderer.cpp \
	arrow.cpp \
	${cairo_src} \
	datasource_cache.cpp \
	distance.cpp \
	envelope.cpp \
	filter_factory.cpp \
	font_engine_freetype.cpp \
	font_set.cpp \
	graphics.cpp \
	image_reader.cpp \
	image_util.cpp \
	layer.cpp \
  	${libxml2_src} \
	line_pattern_symbolizer.cpp \
	load_map.cpp \
	map.cpp \
	memory.cpp \
	memory_datasource.cpp \
	params.cpp \
	placement_finder.cpp \
	plugin.cpp \
	png_reader.cpp \
	point_symbolizer.cpp \
	polygon_pattern_symbolizer.cpp \
	projection.cpp \
	proj_transform.cpp \
	save_map.cpp \
	scale_denominator.cpp \
	shield_symbolizer.cpp \
  	stroke.cpp \
  	symbolizer.cpp \
	text_symbolizer.cpp \
	tiff_reader.cpp \
	unicode.cpp \
	wkb.cpp
  

libmapnik_la_LIBADD = \
	${PNG_LIBS} \
  	${FREETYPE2_LIBS} \
  	${AGG_LIBS} \
	${BOOST_LDFLAGS} \
	${BOOST_THREAD_LIB} \
	${BOOST_REGEX_LIB} \
	${BOOST_FILESYSTEM_LIB} \
	${BOOST_IOSTREAMS_LIB} \
  	${JPEG_LIBS} \
  	${TIFF_LIBS} \
  	${PROJ_LIBS} \
  	${LTDL_LIBS} \
  	${LIBXML2_LIBS} \
	${CAIRO_LIBS}

libmapnik_la_CXXFLAGS = \
  	-I../include        \
	${PROFILING_CFLAGS} \
	${PNG_CFLAGS} \
  	${FREETYPE2_CFLAGS} \
  	${AGG_CFLAGS} \
  	${JPEG_CFLAGS} \
  	${TIFF_CFLAGS} \
  	${PROJ_CFLAGS} \
  	${LTDL_CFLAGS} \
  	${BOOST_CPPFLAGS} \
  	${LIBXML2_CFLAGS} \
	${CAIRO_CFLAGS}
	
	


