/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#include "boost_std_shared_shim.hpp"
/* The functions in this file produce deprecation warnings.
 * But as shield symbolizer doesn't fully support more than one
 * placement from python yet these functions are actually the
 * correct ones.
 */

#define NO_DEPRECATION_WARNINGS

// boost
#include <boost/python.hpp>
#include <boost/variant.hpp>

// mapnik
//symbolizer typdef here rather than mapnik/symbolizer.hpp
#include <mapnik/rule.hpp>
#include <mapnik/symbolizer_hash.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/path_expression.hpp>
#include "mapnik_enumeration.hpp"
#include "mapnik_svg.hpp"
#include <mapnik/graphics.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/value_error.hpp>
#include <mapnik/marker_cache.hpp> // for known_svg_prefix_
#include <mapnik/pixel_position.hpp>

using mapnik::symbolizer;
using mapnik::rule;
using mapnik::point_symbolizer;
using mapnik::line_symbolizer;
using mapnik::line_pattern_symbolizer;
using mapnik::polygon_symbolizer;
using mapnik::polygon_pattern_symbolizer;
using mapnik::raster_symbolizer;
using mapnik::shield_symbolizer;
using mapnik::text_symbolizer;
using mapnik::building_symbolizer;
using mapnik::markers_symbolizer;
using mapnik::color;
using mapnik::path_processor_type;
using mapnik::path_expression_ptr;
using mapnik::guess_type;
using mapnik::expression_ptr;
using mapnik::parse_path;

namespace {
using namespace boost::python;

tuple get_shield_displacement(const shield_symbolizer& s)
{
    mapnik::pixel_position const& pos = s.get_shield_displacement();
    return boost::python::make_tuple(pos.x, pos.y);
}

void set_shield_displacement(shield_symbolizer & s, boost::python::tuple arg)
{
    s.get_placement_options()->defaults.displacement.x = extract<double>(arg[0]);
    s.get_placement_options()->defaults.displacement.y = extract<double>(arg[1]);
}

tuple get_text_displacement(const shield_symbolizer& t)
{
    mapnik::pixel_position const& pos = t.get_placement_options()->defaults.displacement;
    return boost::python::make_tuple(pos.x, pos.y);
}

void set_text_displacement(shield_symbolizer & t, boost::python::tuple arg)
{
    t.set_displacement(extract<double>(arg[0]),extract<double>(arg[1]));
}

void set_marker_type(mapnik::markers_symbolizer & symbolizer, std::string const& marker_type)
{
    std::string filename;
    if (marker_type == "ellipse")
    {
        filename = mapnik::marker_cache::instance().known_svg_prefix_ + "ellipse";
    }
    else if (marker_type == "arrow")
    {
        filename = mapnik::marker_cache::instance().known_svg_prefix_ + "arrow";
    }
    else
    {
        throw mapnik::value_error("Unknown marker-type: '" + marker_type + "'");
    }
    symbolizer.set_filename(parse_path(filename));
}


struct get_symbolizer_type : public boost::static_visitor<std::string>
{
public:
    std::string operator () (point_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("point");
    }

    std::string operator () ( line_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("line");
    }

    std::string operator () (line_pattern_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("line_pattern");
    }

    std::string operator () (polygon_symbolizer const& sym ) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("polygon");
    }

    std::string operator () (polygon_pattern_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("polygon_pattern");
    }

    std::string operator () (raster_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("raster");
    }

    std::string operator () (shield_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("shield");
    }

    std::string operator () (text_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("text");
    }

    std::string operator () (building_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("building");
    }

    std::string operator () (markers_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("markers");
    }

    template <typename Symbolizer>
    std::string operator() ( Symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("unknown");
    }
};

std::string get_symbol_type(symbolizer const& symbol)
{
    std::string type = boost::apply_visitor( get_symbolizer_type(), symbol);
    return type;
}

const point_symbolizer& point_(symbolizer const& symbol )
{
    return boost::get<point_symbolizer>(symbol);
}

const line_symbolizer& line_( const symbolizer& symbol )
{
    return boost::get<line_symbolizer>(symbol);
}

const polygon_symbolizer& polygon_( const symbolizer& symbol )
{
    return boost::get<polygon_symbolizer>(symbol);
}

const raster_symbolizer& raster_( const symbolizer& symbol )
{
    return boost::get<raster_symbolizer>(symbol);
}

const text_symbolizer& text_( const symbolizer& symbol )
{
    return boost::get<text_symbolizer>(symbol);
}

const shield_symbolizer& shield_( const symbolizer& symbol )
{
    return boost::get<shield_symbolizer>(symbol);
}

const line_pattern_symbolizer& line_pattern_( const symbolizer& symbol )
{
    return boost::get<line_pattern_symbolizer>(symbol);
}

const polygon_pattern_symbolizer& polygon_pattern_( const symbolizer& symbol )
{
    return boost::get<polygon_pattern_symbolizer>(symbol);
}

const building_symbolizer& building_( const symbolizer& symbol )
{
    return boost::get<building_symbolizer>(symbol);
}

const markers_symbolizer& markers_( const symbolizer& symbol )
{
    return boost::get<markers_symbolizer>(symbol);
}

struct symbolizer_hash_visitor : public boost::static_visitor<std::size_t>
{
    template <typename T>
    std::size_t operator() (T const& sym) const
    {
        return mapnik::symbolizer_hash::value(sym);
    }
};

std::size_t hash_impl(symbolizer const& sym)
{
    return boost::apply_visitor(symbolizer_hash_visitor(), sym);
}

template <typename T>
std::string get_file_impl(T const& sym)
{
    return path_processor_type::to_string(*sym.get_filename());
}

template <typename T>
void set_file_impl(T & sym, std::string const& file_expr)
{
    sym.set_filename(parse_path(file_expr));
}

}

void export_symbolizer()
{
    using namespace boost::python;

    class_<symbolizer>("Symbolizer",no_init)

        .def("type",get_symbol_type)

        .def("__hash__",hash_impl)

        .def("point",point_,
             return_value_policy<copy_const_reference>())

        .def("line",line_,
             return_value_policy<copy_const_reference>())

        .def("line_pattern",line_pattern_,
             return_value_policy<copy_const_reference>())

        .def("polygon",polygon_,
             return_value_policy<copy_const_reference>())

        .def("polygon_pattern",polygon_pattern_,
             return_value_policy<copy_const_reference>())

        .def("raster",raster_,
             return_value_policy<copy_const_reference>())

        .def("shield",shield_,
             return_value_policy<copy_const_reference>())

        .def("text",text_,
             return_value_policy<copy_const_reference>())

        .def("building",building_,
             return_value_policy<copy_const_reference>())

        .def("markers",markers_,
             return_value_policy<copy_const_reference>())
        ;
}


void export_shield_symbolizer()
{
    using namespace boost::python;
    class_< shield_symbolizer, bases<text_symbolizer> >("ShieldSymbolizer",
                                                        init<expression_ptr,
                                                        std::string const&,
                                                        unsigned, mapnik::color const&,
                                                        path_expression_ptr>()
        )
        .add_property("allow_overlap",
                      &shield_symbolizer::get_allow_overlap,
                      &shield_symbolizer::set_allow_overlap,
                      "Set/get the allow_overlap property of the label")
        .add_property("avoid_edges",
                      &shield_symbolizer::get_avoid_edges,
                      &shield_symbolizer::set_avoid_edges,
                      "Set/get the avoid_edge property of the label")
        .add_property("character_spacing",
                      &shield_symbolizer::get_character_spacing,
                      &shield_symbolizer::set_character_spacing,
                      "Set/get the character_spacing property of the label")
        .add_property("displacement",
                      &get_text_displacement,
                      &set_text_displacement)
        .add_property("face_name",
                      make_function(&shield_symbolizer::get_face_name,return_value_policy<copy_const_reference>()),
                      &shield_symbolizer::set_face_name,
                      "Set/get the face_name property of the label")
        .add_property("fill",
                      make_function(&shield_symbolizer::get_fill,return_value_policy<copy_const_reference>()),
                      &shield_symbolizer::set_fill)
        .add_property("fontset",
                      make_function(&shield_symbolizer::get_fontset,return_value_policy<copy_const_reference>()),
                      &shield_symbolizer::set_fontset)
        .add_property("force_odd_labels",
                      &shield_symbolizer::get_force_odd_labels,
                      &shield_symbolizer::set_force_odd_labels)
        .add_property("halo_fill",
                      make_function(&shield_symbolizer::get_halo_fill,return_value_policy<copy_const_reference>()),
                      &shield_symbolizer::set_halo_fill)
        .add_property("halo_radius",
                      &shield_symbolizer::get_halo_radius,
                      &shield_symbolizer::set_halo_radius)
        .add_property("horizontal_alignment",
                      &shield_symbolizer::get_horizontal_alignment,
                      &shield_symbolizer::set_horizontal_alignment,
                      "Set/get the horizontal alignment of the label")
        .add_property("justify_alignment",
                      &shield_symbolizer::get_justify_alignment,
                      &shield_symbolizer::set_justify_alignment,
                      "Set/get the text justification")
        .add_property("label_placement",
                      &shield_symbolizer::get_label_placement,
                      &shield_symbolizer::set_label_placement,
                      "Set/get the placement of the label")
        .add_property("label_position_tolerance",
                      &shield_symbolizer::get_label_position_tolerance,
                      &shield_symbolizer::set_label_position_tolerance)
        .add_property("label_spacing",
                      &shield_symbolizer::get_label_spacing,
                      &shield_symbolizer::set_label_spacing)
        .add_property("line_spacing",
                      &shield_symbolizer::get_line_spacing,
                      &shield_symbolizer::set_line_spacing)
        .add_property("max_char_angle_delta",
                      &shield_symbolizer::get_max_char_angle_delta,
                      &shield_symbolizer::set_max_char_angle_delta)
        .add_property("minimum_distance",
                      &shield_symbolizer::get_minimum_distance,
                      &shield_symbolizer::set_minimum_distance)
        .add_property("minimum_padding",
                      &shield_symbolizer::get_minimum_padding,
                      &shield_symbolizer::set_minimum_padding)
        .add_property("name",&shield_symbolizer::get_name,
                      &shield_symbolizer::set_name)
        .add_property("opacity",
                      &shield_symbolizer::get_opacity,
                      &shield_symbolizer::set_opacity,
                      "Set/get the shield opacity")
        .add_property("shield_displacement",
                      get_shield_displacement,
                      set_shield_displacement)
        .add_property("text_opacity",
                      &shield_symbolizer::get_text_opacity,
                      &shield_symbolizer::set_text_opacity,
                      "Set/get the text opacity")
        .add_property("text_transform",
                      &shield_symbolizer::get_text_transform,
                      &shield_symbolizer::set_text_transform,
                      "Set/get the text conversion method")
        .add_property("text_ratio",
                      &shield_symbolizer::get_text_ratio,
                      &shield_symbolizer::set_text_ratio)
        .add_property("text_size",
                      &shield_symbolizer::get_text_size,
                      &shield_symbolizer::set_text_size)
        .add_property("vertical_alignment",
                      &shield_symbolizer::get_vertical_alignment,
                      &shield_symbolizer::set_vertical_alignment,
                      "Set/get the vertical alignment of the label")
        .add_property("wrap_width",
                      &shield_symbolizer::get_wrap_width,
                      &shield_symbolizer::set_wrap_width)
        .add_property("wrap_character",
                      &shield_symbolizer::get_wrap_char_string,
                      &shield_symbolizer::set_wrap_char_from_string)
        .add_property("wrap_before",
                      &shield_symbolizer::get_wrap_before,
                      &shield_symbolizer::set_wrap_before)
        .add_property("unlock_image",
                      &shield_symbolizer::get_unlock_image,
                      &shield_symbolizer::set_unlock_image)
        .add_property("filename",
                      get_file_impl<shield_symbolizer>,
                      set_file_impl<shield_symbolizer>)
        .add_property("transform",
                      mapnik::get_svg_transform<shield_symbolizer>,
                      mapnik::set_svg_transform<shield_symbolizer>)
        .add_property("comp_op",
                      &shield_symbolizer::comp_op,
                      &shield_symbolizer::set_comp_op,
                      "Set/get the comp-op")
        .add_property("clip",
                      &shield_symbolizer::clip,
                      &shield_symbolizer::set_clip,
                      "Set/get the shield geometry's clipping status")
        ;
}

void export_polygon_symbolizer()
{
    using namespace boost::python;

    class_<polygon_symbolizer>("PolygonSymbolizer",
                               init<>("Default PolygonSymbolizer - solid fill grey"))
        .def(init<color const&>("TODO"))
        .add_property("fill",make_function
                      (&polygon_symbolizer::get_fill,
                       return_value_policy<copy_const_reference>()),
                      &polygon_symbolizer::set_fill)
        .add_property("fill_opacity",
                      &polygon_symbolizer::get_opacity,
                      &polygon_symbolizer::set_opacity)
        .add_property("gamma",
                      &polygon_symbolizer::get_gamma,
                      &polygon_symbolizer::set_gamma)
        .add_property("gamma_method",
                      &polygon_symbolizer::get_gamma_method,
                      &polygon_symbolizer::set_gamma_method,
                      "gamma correction method")
        .add_property("comp_op",
                      &polygon_symbolizer::comp_op,
                      &polygon_symbolizer::set_comp_op,
                      "Set/get the polygon comp-op")
        .add_property("clip",
                      &polygon_symbolizer::clip,
                      &polygon_symbolizer::set_clip,
                      "Set/get the polygon geometry's clipping status")
        .add_property("smooth",
                      &polygon_symbolizer::smooth,
                      &polygon_symbolizer::set_smooth,
                      "Set/get the polygon geometry's smooth value")
        .add_property("simplify_tolerance",
                      &polygon_symbolizer::simplify_tolerance,
                      &polygon_symbolizer::set_simplify_tolerance,
                      "simplfication tolerance measure")
        .def("__hash__", hash_impl)
        ;

}

void export_polygon_pattern_symbolizer()
{
    using namespace boost::python;

    mapnik::enumeration_<mapnik::pattern_alignment_e>("pattern_alignment")
        .value("LOCAL",mapnik::LOCAL_ALIGNMENT)
        .value("GLOBAL",mapnik::GLOBAL_ALIGNMENT)
        ;

    class_<polygon_pattern_symbolizer>("PolygonPatternSymbolizer",
                                       init<path_expression_ptr>("<path_expression_ptr>"))
        .add_property("alignment",
                      &polygon_pattern_symbolizer::get_alignment,
                      &polygon_pattern_symbolizer::set_alignment,
                      "Set/get the alignment of the pattern")
        .add_property("transform",
                      mapnik::get_svg_transform<polygon_pattern_symbolizer>,
                      mapnik::set_svg_transform<polygon_pattern_symbolizer>)
        .add_property("filename",
                      &get_file_impl<polygon_pattern_symbolizer>,
                      &set_file_impl<polygon_pattern_symbolizer>)
        .add_property("opacity",
                      &polygon_pattern_symbolizer::get_opacity,
                      &polygon_pattern_symbolizer::set_opacity)
        .add_property("gamma",
                      &polygon_pattern_symbolizer::get_gamma,
                      &polygon_pattern_symbolizer::set_gamma)
        .add_property("gamma_method",
                      &polygon_pattern_symbolizer::get_gamma_method,
                      &polygon_pattern_symbolizer::set_gamma_method,
                      "Set/get the gamma correction method of the polygon")
        .add_property("comp_op",
                      &polygon_pattern_symbolizer::comp_op,
                      &polygon_pattern_symbolizer::set_comp_op,
                      "Set/get the pattern comp-op")
        .add_property("clip",
                      &polygon_pattern_symbolizer::clip,
                      &polygon_pattern_symbolizer::set_clip,
                      "Set/get the pattern geometry's clipping status")
        .add_property("smooth",
                      &polygon_pattern_symbolizer::smooth,
                      &polygon_pattern_symbolizer::set_smooth,
                      "Set/get the pattern geometry's smooth value")
        ;
}


void export_raster_symbolizer()
{
    using namespace boost::python;

    class_<raster_symbolizer>("RasterSymbolizer",
                              init<>("Default ctor"))

        .add_property("mode",
                      make_function(&raster_symbolizer::get_mode,return_value_policy<copy_const_reference>()),
                      &raster_symbolizer::set_mode,
                      "Get/Set merging mode. (deprecated, use comp_op instead)\n"
            )
        .add_property("comp_op",
                      &raster_symbolizer::comp_op,
                      &raster_symbolizer::set_comp_op,
                      "Set/get the raster comp-op"
            )
        .add_property("scaling",
                      &raster_symbolizer::get_scaling_method,
                      &raster_symbolizer::set_scaling_method,
                      "Get/Set scaling algorithm.\n"
                      "\n"
                      "Usage:\n"
                      "\n"
                      ">>> from mapnik import RasterSymbolizer\n"
                      ">>> r = RasterSymbolizer()\n"
                      ">>> r.scaling = 'mapnik.scaling_method.GAUSSIAN'\n"
            )

        .add_property("opacity",
                      &raster_symbolizer::get_opacity,
                      &raster_symbolizer::set_opacity,
                      "Get/Set opacity.\n"
                      "\n"
                      "Usage:\n"
                      "\n"
                      ">>> from mapnik import RasterSymbolizer\n"
                      ">>> r = RasterSymbolizer()\n"
                      ">>> r.opacity = .5\n"
            )
        .add_property("colorizer",
                      &raster_symbolizer::get_colorizer,
                      &raster_symbolizer::set_colorizer,
                      "Get/Set the RasterColorizer used to color data rasters.\n"
                      "\n"
                      "Usage:\n"
                      "\n"
                      ">>> from mapnik import RasterSymbolizer, RasterColorizer\n"
                      ">>> r = RasterSymbolizer()\n"
                      ">>> r.colorizer = RasterColorizer()\n"
                      ">>> for value, color in [\n"
                      "...     (0, \"#000000\"),\n"
                      "...     (10, \"#ff0000\"),\n"
                      "...     (40, \"#00ff00\"),\n"
                      "... ]:\n"
                      "...      r.colorizer.append_band(value, color)\n"
            )
        .add_property("filter_factor",
                      &raster_symbolizer::get_filter_factor,
                      &raster_symbolizer::set_filter_factor,
                      "Get/Set the filter factor used by the datasource.\n"
                      "\n"
                      "This is used by the Raster or Gdal datasources to pre-downscale\n"
                      "images using overviews.\n"
                      "Higher numbers can sometimes cause much better scaled image\n"
                      "output, at the cost of speed.\n"
                      "\n"
                      "Examples:\n"
                      " -1.0 : (Default) A suitable value will be determined from the\n"
                      "        chosen scaling method during rendering.\n"
                      "  1.0 : The datasource will take care of all the scaling\n"
                      "        (using nearest neighbor interpolation)\n"
                      "  2.0 : The datasource will scale the datasource to\n"
                      "        2.0x the desired size, and mapnik will scale the rest\n"
                      "        of the way using the interpolation defined in self.scaling.\n"
            )
        .add_property("mesh_size",
                      &raster_symbolizer::get_mesh_size,
                      &raster_symbolizer::set_mesh_size,
                      "Get/Set warping mesh size.\n"
                      "Larger values result in faster warping times but might "
                      "result in distorted maps.\n"
                      "\n"
                      "Usage:\n"
                      "\n"
                      ">>> from mapnik import RasterSymbolizer\n"
                      ">>> r = RasterSymbolizer()\n"
                      ">>> r.mesh_size = 32\n"
            )
        .add_property("premultiplied",
                      &raster_symbolizer::premultiplied,
                      &raster_symbolizer::set_premultiplied,
                      "Get/Set premultiplied status of the source image.\n"
                      "Can be used to override what the source data reports (when in error)\n"
                      "\n"
                      "Usage:\n"
                      "\n"
                      ">>> from mapnik import RasterSymbolizer\n"
                      ">>> r = RasterSymbolizer()\n"
                      ">>> r.premultiplied = False\n"
            )
        ;
}

void export_point_symbolizer()
{
    using namespace boost::python;

    mapnik::enumeration_<mapnik::point_placement_e>("point_placement")
        .value("CENTROID",mapnik::CENTROID_POINT_PLACEMENT)
        .value("INTERIOR",mapnik::INTERIOR_POINT_PLACEMENT)
        ;

    class_<point_symbolizer>("PointSymbolizer",
                             init<>("Default Point Symbolizer - 4x4 black square"))
        .def (init<mapnik::path_expression_ptr>("<path expression ptr>"))
        .add_property("filename",
                      &get_file_impl<point_symbolizer>,
                      &set_file_impl<point_symbolizer>)
        .add_property("allow_overlap",
                      &point_symbolizer::get_allow_overlap,
                      &point_symbolizer::set_allow_overlap)
        .add_property("opacity",
                      &point_symbolizer::get_opacity,
                      &point_symbolizer::set_opacity)
        .add_property("ignore_placement",
                      &point_symbolizer::get_ignore_placement,
                      &point_symbolizer::set_ignore_placement)
        .add_property("placement",
                      &point_symbolizer::get_point_placement,
                      &point_symbolizer::set_point_placement,
                      "Set/get the placement of the point")
        .add_property("transform",
                      mapnik::get_svg_transform<point_symbolizer>,
                      mapnik::set_svg_transform<point_symbolizer>)
        .add_property("comp_op",
                      &point_symbolizer::comp_op,
                      &point_symbolizer::set_comp_op,
                      "Set/get the comp-op")
        ;
}

void export_markers_symbolizer()
{
    using namespace boost::python;

    mapnik::enumeration_<mapnik::marker_placement_e>("marker_placement")
        .value("POINT_PLACEMENT",mapnik::MARKER_POINT_PLACEMENT)
        .value("INTERIOR_PLACEMENT",mapnik::MARKER_INTERIOR_PLACEMENT)
        .value("LINE_PLACEMENT",mapnik::MARKER_LINE_PLACEMENT)
        ;

    mapnik::enumeration_<mapnik::marker_multi_policy_e>("marker_multi_policy")
        .value("EACH",mapnik::MARKER_EACH_MULTI)
        .value("WHOLE",mapnik::MARKER_WHOLE_MULTI)
        .value("LARGEST",mapnik::MARKER_LARGEST_MULTI)
        ;

    class_<markers_symbolizer>("MarkersSymbolizer",
                               init<>("Default Markers Symbolizer - circle"))
        .def (init<mapnik::path_expression_ptr>("<path expression ptr>"))
        .add_property("filename",
                      &get_file_impl<markers_symbolizer>,
                      &set_file_impl<markers_symbolizer>)
        .add_property("marker_type",
                      &get_file_impl<markers_symbolizer>,
                      &set_marker_type)
        .add_property("allow_overlap",
                      &markers_symbolizer::get_allow_overlap,
                      &markers_symbolizer::set_allow_overlap)
        .add_property("spacing",
                      &markers_symbolizer::get_spacing,
                      &markers_symbolizer::set_spacing)
        .add_property("max_error",
                      &markers_symbolizer::get_max_error,
                      &markers_symbolizer::set_max_error)
        .add_property("opacity",
                      &markers_symbolizer::get_opacity,
                      &markers_symbolizer::set_opacity,
                      "Set/get the overall opacity")
        .add_property("fill_opacity",
                      &markers_symbolizer::get_fill_opacity,
                      &markers_symbolizer::set_fill_opacity,
                      "Set/get the fill opacity")
        .add_property("ignore_placement",
                      &markers_symbolizer::get_ignore_placement,
                      &markers_symbolizer::set_ignore_placement)
        .add_property("transform",
                      &mapnik::get_svg_transform<markers_symbolizer>,
                      &mapnik::set_svg_transform<markers_symbolizer>)
        .add_property("width",
                      make_function(&markers_symbolizer::get_width,
                                    return_value_policy<copy_const_reference>()),
                      &markers_symbolizer::set_width,
                      "Set/get the marker width")
        .add_property("height",
                      make_function(&markers_symbolizer::get_height,
                                    return_value_policy<copy_const_reference>()),
                      &markers_symbolizer::set_height,
                      "Set/get the marker height")
        .add_property("fill",
                      &markers_symbolizer::get_fill,
                      &markers_symbolizer::set_fill,
                      "Set/get the marker fill color")
        .add_property("stroke",
                      &markers_symbolizer::get_stroke,
                      &markers_symbolizer::set_stroke,
                      "Set/get the marker stroke (outline)")
        .add_property("placement",
                      &markers_symbolizer::get_marker_placement,
                      &markers_symbolizer::set_marker_placement,
                      "Set/get the marker placement")
        .add_property("multi_policy",
                      &markers_symbolizer::get_marker_multi_policy,
                      &markers_symbolizer::set_marker_multi_policy,
                      "Set/get the marker multi geometry rendering policy")
        .add_property("comp_op",
                      &markers_symbolizer::comp_op,
                      &markers_symbolizer::set_comp_op,
                      "Set/get the marker comp-op")
        .add_property("clip",
                      &markers_symbolizer::clip,
                      &markers_symbolizer::set_clip,
                      "Set/get the marker geometry's clipping status")
        .add_property("smooth",
                      &markers_symbolizer::smooth,
                      &markers_symbolizer::set_smooth,
                      "Set/get the marker geometry's smooth value")
        ;
}


void export_line_symbolizer()
{
    using namespace boost::python;
    mapnik::enumeration_<mapnik::line_rasterizer_e>("line_rasterizer")
        .value("FULL",mapnik::RASTERIZER_FULL)
        .value("FAST",mapnik::RASTERIZER_FAST)
        ;
    class_<line_symbolizer>("LineSymbolizer",
                            init<>("Default LineSymbolizer - 1px solid black"))
        .def(init<mapnik::stroke const&>("TODO"))
        .def(init<mapnik::color const& ,float>())
        .add_property("rasterizer",
                      &line_symbolizer::get_rasterizer,
                      &line_symbolizer::set_rasterizer,
                      "Set/get the rasterization method of the line of the point")
        .add_property("stroke",make_function
                      (&line_symbolizer::get_stroke,
                       return_value_policy<reference_existing_object>()),
                      &line_symbolizer::set_stroke)
        .add_property("simplify_tolerance",
                      &line_symbolizer::simplify_tolerance,
                      &line_symbolizer::set_simplify_tolerance,
                      "simplification tolerance measure")
        .add_property("offset",
                      &line_symbolizer::offset,
                      &line_symbolizer::set_offset,
                      "offset value")
        .add_property("comp_op",
                      &line_symbolizer::comp_op,
                      &line_symbolizer::set_comp_op,
                      "Set/get the comp-op")
        .add_property("clip",
                      &line_symbolizer::clip,
                      &line_symbolizer::set_clip,
                      "Set/get the line geometry's clipping status")
        .add_property("smooth",
                      &line_symbolizer::smooth,
                      &line_symbolizer::set_smooth,
                      "smooth value (0..1.0)")
        .def("__hash__", hash_impl)
        ;
}

void export_line_pattern_symbolizer()
{
    using namespace boost::python;

    class_<line_pattern_symbolizer>("LinePatternSymbolizer",
                                    init<path_expression_ptr>
                                    ("<image file expression>"))
        .add_property("transform",
                      mapnik::get_svg_transform<line_pattern_symbolizer>,
                      mapnik::set_svg_transform<line_pattern_symbolizer>)
        .add_property("filename",
                      &get_file_impl<line_pattern_symbolizer>,
                      &set_file_impl<line_pattern_symbolizer>)
        .add_property("offset",
                      &line_pattern_symbolizer::offset,
                      &line_pattern_symbolizer::set_offset,
                      "Set/get the offset")
        .add_property("comp_op",
                      &line_pattern_symbolizer::comp_op,
                      &line_pattern_symbolizer::set_comp_op,
                      "Set/get the comp-op")
        .add_property("clip",
                      &line_pattern_symbolizer::clip,
                      &line_pattern_symbolizer::set_clip,
                      "Set/get the line pattern geometry's clipping status")
        .add_property("smooth",
                      &line_pattern_symbolizer::smooth,
                      &line_pattern_symbolizer::set_smooth,
                      "smooth value (0..1.0)")
        ;
}

void export_debug_symbolizer()
{
    using namespace boost::python;

    mapnik::enumeration_<mapnik::debug_symbolizer_mode_e>("debug_symbolizer_mode")
        .value("COLLISION",mapnik::DEBUG_SYM_MODE_COLLISION)
        .value("VERTEX",mapnik::DEBUG_SYM_MODE_VERTEX)
        ;

    class_<mapnik::debug_symbolizer>("DebugSymbolizer",
                             init<>("Default debug Symbolizer"))
        .add_property("mode",
                      &mapnik::debug_symbolizer::get_mode,
                      &mapnik::debug_symbolizer::set_mode)
        ;
}


void export_building_symbolizer()
{
    using namespace boost::python;

    class_<building_symbolizer>("BuildingSymbolizer",
                               init<>("Default BuildingSymbolizer"))
        .add_property("fill",make_function
                      (&building_symbolizer::get_fill,
                       return_value_policy<copy_const_reference>()),
                      &building_symbolizer::set_fill)
        .add_property("fill_opacity",
                      &building_symbolizer::get_opacity,
                      &building_symbolizer::set_opacity)
        .add_property("height",
                      make_function(&building_symbolizer::height,
                                    return_value_policy<copy_const_reference>()),
                      &building_symbolizer::set_height,
                      "Set/get the building height")
        ;

}
