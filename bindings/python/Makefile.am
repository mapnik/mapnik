
pkglib_LTLIBRARIES = \
	_mapnik.la

_mapnik_la_SOURCES = \
	mapnik_color.cpp\
	mapnik_coord.cpp\
	mapnik_datasource.cpp\
	mapnik_datasource_cache.cpp \
	mapnik_enumeration.hpp \
	mapnik_envelope.cpp \
	mapnik_feature.cpp \
	mapnik_featureset.cpp \
	mapnik_filter.cpp \
	mapnik_font_engine.cpp \
	mapnik_geometry.cpp \
	mapnik_image.cpp \
	mapnik_image_view.cpp \
	mapnik_layer.cpp \
	mapnik_line_pattern_symbolizer.cpp \
	mapnik_line_symbolizer.cpp \
	mapnik_map.cpp \
	mapnik_parameters.cpp \
	mapnik_point_symbolizer.cpp \
	mapnik_polygon_pattern_symbolizer.cpp \
	mapnik_polygon_symbolizer.cpp \
	mapnik_projection.cpp \
	mapnik_proj_transform.cpp \
	mapnik_python.cpp \
	mapnik_query.cpp \
	mapnik_raster_symbolizer.cpp \
	mapnik_rule.cpp \
	mapnik_shield_symbolizer.cpp \
	mapnik_stroke.cpp \
	mapnik_style.cpp \
	mapnik_text_symbolizer.cpp \
	python_cairo.cpp \
	python_optional.hpp

_mapnik_la_LIBADD = \
	${PYTHON_LIB} \
	${BOOST_PYTHON_LIB} \
	${FREETYPE2_LIBS}

_mapnik_la_CXXFLAGS = \
	-I../../include \
	-I${PYTHON_INCLUDE_DIR} \
	${FREETYPE2_CFLAGS}

## File created by the gnome-build tools

_mapnik_la_LDFLAGS = \
	-module
