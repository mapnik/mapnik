/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
// $Id$

// mapnik
#include <mapnik/save_map.hpp>
#include <mapnik/map.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/metawriter_factory.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// stl
#include <iostream>

namespace mapnik
{
using boost::property_tree::ptree;
using boost::optional;


class serialize_symbolizer : public boost::static_visitor<>
{
public:
    serialize_symbolizer( ptree & r , bool explicit_defaults):
        rule_(r),
        explicit_defaults_(explicit_defaults) {}

    void operator () ( const  point_symbolizer & sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("PointSymbolizer", ptree()))->second;

        add_image_attributes( sym_node, sym );

        point_symbolizer dfl;
        if (sym.get_allow_overlap() != dfl.get_allow_overlap() || explicit_defaults_ )
        {
            set_attr( sym_node, "allow-overlap", sym.get_allow_overlap() );
        }
        if ( sym.get_opacity() != dfl.get_opacity() || explicit_defaults_ )
        {
            set_attr( sym_node, "opacity", sym.get_opacity() );
        }
        if ( sym.get_point_placement() != dfl.get_point_placement() || explicit_defaults_ )
        {
            set_attr( sym_node, "placement", sym.get_point_placement() );
        }
        add_metawriter_attributes(sym_node, sym);
    }

    void operator () ( const line_symbolizer & sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("LineSymbolizer", ptree()))->second;

        const stroke & strk =  sym.get_stroke();
        add_stroke_attributes(sym_node, strk);
        add_metawriter_attributes(sym_node, sym);
    }
        
    void operator () ( const line_pattern_symbolizer & sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("LinePatternSymbolizer",
                              ptree()))->second;

        add_image_attributes( sym_node, sym );
        add_metawriter_attributes(sym_node, sym);
    }

    void operator () ( const polygon_symbolizer & sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("PolygonSymbolizer", ptree()))->second;
        polygon_symbolizer dfl;

        if ( sym.get_fill() != dfl.get_fill() || explicit_defaults_ )
        {
            set_attr( sym_node, "fill", sym.get_fill() );
        }
        if ( sym.get_opacity() != dfl.get_opacity() || explicit_defaults_ )
        {
            set_attr( sym_node, "fill-opacity", sym.get_opacity() );
        }
        if ( sym.get_gamma() != dfl.get_gamma() || explicit_defaults_ )
        {
            set_attr( sym_node, "gamma", sym.get_gamma() );
        }
        add_metawriter_attributes(sym_node, sym);
    }

    void operator () ( const polygon_pattern_symbolizer & sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("PolygonPatternSymbolizer",
                              ptree()))->second;
        polygon_pattern_symbolizer dfl(parse_path(""));

        if ( sym.get_alignment() != dfl.get_alignment() || explicit_defaults_ )
        {
            set_attr( sym_node, "alignment", sym.get_alignment() );
        }

        add_image_attributes( sym_node, sym );
        add_metawriter_attributes(sym_node, sym);
    }

    void operator () ( const raster_symbolizer & sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("RasterSymbolizer", ptree()))->second;
        raster_symbolizer dfl;

        if ( sym.get_mode() != dfl.get_mode() || explicit_defaults_ )
        {
            set_attr( sym_node, "mode", sym.get_mode() );
        }

        if ( sym.get_scaling() != dfl.get_scaling() || explicit_defaults_ )
        {
            set_attr( sym_node, "scaling", sym.get_scaling() );
        }
        
        if ( sym.get_opacity() != dfl.get_opacity() || explicit_defaults_ )
        {
            set_attr( sym_node, "opacity", sym.get_opacity() );
        }

        if (sym.get_colorizer()) {
            serialize_raster_colorizer(sym_node, sym.get_colorizer(),
                                       explicit_defaults_);
        }
        //Note: raster_symbolizer doesn't support metawriters
    }

    void operator () ( const shield_symbolizer & sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("ShieldSymbolizer",
                              ptree()))->second;

        add_font_attributes( sym_node, sym);
        add_image_attributes( sym_node, sym);
        add_metawriter_attributes(sym_node, sym);

        // pseudo-default-construct a shield_symbolizer. It is used
        // to avoid printing of attributes with default values without
        // repeating the default values here.
        // maybe add a real, explicit default-ctor?

        
        shield_symbolizer dfl(expression_ptr(), "<no default>", 0, color(0,0,0), path_expression_ptr());
        
        if (sym.get_unlock_image() != dfl.get_unlock_image() || explicit_defaults_ )
        {
            set_attr( sym_node, "unlock-image", sym.get_unlock_image() );
        }
        if (sym.get_no_text() != dfl.get_no_text() || explicit_defaults_ )
        {
            set_attr( sym_node, "no-text", sym.get_no_text() );
        }
        if (sym.get_text_opacity() != dfl.get_text_opacity() || explicit_defaults_ )
        {
            set_attr( sym_node, "text-opacity", sym.get_text_opacity() );
        }
        position displacement = sym.get_shield_displacement();
        if ( displacement.get<0>() != dfl.get_shield_displacement().get<0>() || explicit_defaults_ )
        {
            set_attr( sym_node, "shield-dx", displacement.get<0>() );
        }
        if ( displacement.get<1>() != dfl.get_shield_displacement().get<1>() || explicit_defaults_ )
        {
            set_attr( sym_node, "shield-dy", displacement.get<1>() );
        }

    }

    void operator () ( const text_symbolizer & sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("TextSymbolizer",
                              ptree()))->second;

        add_font_attributes( sym_node, sym);
        add_metawriter_attributes(sym_node, sym);
    }

    void operator () ( const building_symbolizer & sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("BuildingSymbolizer", ptree()))->second;
        building_symbolizer dfl;

        if ( sym.get_fill() != dfl.get_fill() || explicit_defaults_ )
        {
            set_attr( sym_node, "fill", sym.get_fill() );
        }
        if ( sym.get_opacity() != dfl.get_opacity() || explicit_defaults_ )
        {
            set_attr( sym_node, "fill-opacity", sym.get_opacity() );
        }
        if ( sym.height() != dfl.height() || explicit_defaults_ )
        {
            set_attr( sym_node, "height", sym.height() );
        }
        add_metawriter_attributes(sym_node, sym);
    }

    void operator () ( markers_symbolizer const& sym)
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("MarkersSymbolizer", ptree()))->second;
        markers_symbolizer dfl(parse_path("")); //TODO: Parameter?
        std::string const& filename = path_processor_type::to_string( *sym.get_filename());
        if ( ! filename.empty() ) {
            set_attr( sym_node, "file", filename );
        }
        if (sym.get_allow_overlap() != dfl.get_allow_overlap() || explicit_defaults_)
        {
            set_attr( sym_node, "allow-overlap", sym.get_allow_overlap() );
        }
        if (sym.get_spacing() != dfl.get_spacing() || explicit_defaults_)
        {
            set_attr( sym_node, "spacing", sym.get_spacing() );
        }
        if (sym.get_max_error() != dfl.get_max_error() || explicit_defaults_)
        {
            set_attr( sym_node, "max-error", sym.get_max_error() );
        }
        if (sym.get_fill() != dfl.get_fill() || explicit_defaults_)
        {
            set_attr( sym_node, "fill", sym.get_fill() );
        }
        if (sym.get_opacity() != dfl.get_opacity() || explicit_defaults_)
        {
            set_attr( sym_node, "opacity", sym.get_opacity() );
        }
        if (sym.get_width() != dfl.get_width() || explicit_defaults_)
        {
            set_attr( sym_node, "width", sym.get_width() );
        }
        if (sym.get_height() != dfl.get_height() || explicit_defaults_)
        {
            set_attr( sym_node, "height", sym.get_height() );
        }
        if (sym.get_marker_type() != dfl.get_marker_type() || explicit_defaults_)
        {
            set_attr( sym_node, "marker-type", sym.get_marker_type() );
        }
        if (sym.get_marker_placement() != dfl.get_marker_placement() || explicit_defaults_)
        {
            set_attr( sym_node, "placement", sym.get_marker_placement() );
        }
        std::string tr_str = sym.get_transform_string();
        if (tr_str != "matrix(1, 0, 0, 1, 0, 0)" || explicit_defaults_ )
        {
            set_attr( sym_node, "transform", tr_str );
        }

        const stroke & strk =  sym.get_stroke();
        add_stroke_attributes(sym_node, strk);

        add_metawriter_attributes(sym_node, sym);
    }

    void operator () ( glyph_symbolizer const& sym)
    {
        ptree &node = rule_.push_back(
            ptree::value_type("GlyphSymbolizer", ptree())
            )->second;
                
        glyph_symbolizer dfl("<no default>", expression_ptr());

        // face_name
        set_attr( node, "face-name", sym.get_face_name() );    

        // char
        if (sym.get_char()) {
            const std::string &str =
                to_expression_string(*sym.get_char());
            set_attr( node, "char", str );
        }

        // angle
        if (sym.get_angle()) {
            const std::string &str =
                to_expression_string(*sym.get_angle());
            set_attr( node, "angle", str );
        }

        // value
        if (sym.get_value()) {
            const std::string &str =
                to_expression_string(*sym.get_value());
            set_attr( node, "value", str );
        }

        // size
        if (sym.get_size()) {
            const std::string &str =
                to_expression_string(*sym.get_size());
            set_attr( node, "size", str );
        }

        // color
        if (sym.get_color()) {
            const std::string &str =
                to_expression_string(*sym.get_color());
            set_attr( node, "color", str );
        }

        // colorizer
        if (sym.get_colorizer()) {
            serialize_raster_colorizer(node, sym.get_colorizer(),
                                       explicit_defaults_);
        }

        // allow_overlap
        if (sym.get_allow_overlap() != dfl.get_allow_overlap() || explicit_defaults_ )
        {
            set_attr( node, "allow-overlap", sym.get_allow_overlap() );
        }
        // avoid_edges
        if (sym.get_avoid_edges() != dfl.get_avoid_edges() || explicit_defaults_ )
        {
            set_attr( node, "avoid-edges", sym.get_avoid_edges() );
        }

        // displacement
        position displacement = sym.get_displacement();
        if ( displacement.get<0>() != dfl.get_displacement().get<0>() || explicit_defaults_ )
        {
            set_attr( node, "dx", displacement.get<0>() );
        }
        if ( displacement.get<1>() != dfl.get_displacement().get<1>() || explicit_defaults_ )
        {
            set_attr( node, "dy", displacement.get<1>() );
        }

        // halo fill & radius
        const color & c = sym.get_halo_fill();
        if ( c != dfl.get_halo_fill() || explicit_defaults_ )
        {
            set_attr( node, "halo-fill", c );
        }
        
        if (sym.get_halo_radius() != dfl.get_halo_radius() || explicit_defaults_ )
        {
            set_attr( node, "halo-radius", sym.get_halo_radius() );
        }

        // angle_mode
        if (sym.get_angle_mode() != dfl.get_angle_mode() || explicit_defaults_ )
        {
            set_attr( node, "angle-mode", sym.get_angle_mode() );
        }
        add_metawriter_attributes(node, sym);
    }

private:
    serialize_symbolizer();
    
    void serialize_raster_colorizer(ptree & sym_node,
                                    raster_colorizer_ptr const& colorizer,
                                    bool explicit_defaults)
    {
        ptree & col_node = sym_node.push_back(
            ptree::value_type("RasterColorizer", ptree() ))->second;

        set_attr(col_node, "default-mode", colorizer->get_default_mode());
        set_attr(col_node, "default-color", colorizer->get_default_color());
        set_attr(col_node, "epsilon", colorizer->get_epsilon());

        unsigned i;
        colorizer_stops const &stops = colorizer->get_stops();
        for (i=0; i<stops.size(); i++) {
            ptree &stop_node = col_node.push_back( ptree::value_type("stop", ptree()) )->second;
            set_attr(stop_node, "value", stops[i].get_value());
            set_attr(stop_node, "color", stops[i].get_color());
            set_attr(stop_node, "mode", stops[i].get_mode().as_string());
        }

    }
    
    void add_image_attributes(ptree & node, const symbolizer_with_image & sym)
    {
        std::string const& filename = path_processor_type::to_string( *sym.get_filename());
        if ( ! filename.empty() ) {
            set_attr( node, "file", filename );
        }
        if (sym.get_opacity() != 1.0 || explicit_defaults_ )
        {
            set_attr( node, "opacity", sym.get_opacity() );
        }
        
        std::string tr_str = sym.get_transform_string();
        if (tr_str != "matrix(1, 0, 0, 1, 0, 0)" || explicit_defaults_ )
        {
            set_attr( node, "transform", tr_str );
        }

    }
    void add_font_attributes(ptree & node, const text_symbolizer & sym)
    {
        expression_ptr const& expr = sym.get_name();
        const std::string & name = to_expression_string(*expr);
                
        if ( ! name.empty() ) {
            set_attr( node, "name", name );
        }
        const std::string & face_name = sym.get_face_name();
        if ( ! face_name.empty() ) {
            set_attr( node, "face-name", face_name );
        }
        const std::string & fontset_name = sym.get_fontset().get_name();
        if ( ! fontset_name.empty() ) {
            set_attr( node, "fontset-name", fontset_name );
        }

        set_attr( node, "size", sym.get_text_size() );
        set_attr( node, "fill", sym.get_fill() );

        // pseudo-default-construct a text_symbolizer. It is used
        // to avoid printing ofattributes with default values without
        // repeating the default values here.
        // maybe add a real, explicit default-ctor?
        // FIXME
        text_symbolizer dfl(expression_ptr(), "<no default>",
                            0, color(0,0,0) );

        position displacement = sym.get_displacement();
        if ( displacement.get<0>() != dfl.get_displacement().get<0>() || explicit_defaults_ )
        {
            set_attr( node, "dx", displacement.get<0>() );
        }
        if ( displacement.get<1>() != dfl.get_displacement().get<1>() || explicit_defaults_ )
        {
            set_attr( node, "dy", displacement.get<1>() );
        }

        if (sym.get_label_placement() != dfl.get_label_placement() || explicit_defaults_ )
        {
            set_attr( node, "placement", sym.get_label_placement() );
        }

        if (sym.get_vertical_alignment() != dfl.get_vertical_alignment() || explicit_defaults_ )
        {
            set_attr( node, "vertical-alignment", sym.get_vertical_alignment() );
        }

        if (sym.get_halo_radius() != dfl.get_halo_radius() || explicit_defaults_ )
        {
            set_attr( node, "halo-radius", sym.get_halo_radius() );
        }
        const color & c = sym.get_halo_fill();
        if ( c != dfl.get_halo_fill() || explicit_defaults_ )
        {
            set_attr( node, "halo-fill", c );
        }
        if (sym.get_text_ratio() != dfl.get_text_ratio() || explicit_defaults_ )
        {
            set_attr( node, "text-ratio", sym.get_text_ratio() );
        }
        if (sym.get_wrap_width() != dfl.get_wrap_width() || explicit_defaults_ )
        {
            set_attr( node, "wrap-width", sym.get_wrap_width() );
        }
        if (sym.get_wrap_before() != dfl.get_wrap_before() || explicit_defaults_ )
        {
            set_attr( node, "wrap-before", sym.get_wrap_before() );
        }
        if (sym.get_wrap_char() != dfl.get_wrap_char() || explicit_defaults_ )
        {
            set_attr( node, "wrap-character", std::string(1, sym.get_wrap_char()) );
        }
        if (sym.get_text_transform() != dfl.get_text_transform() || explicit_defaults_ )
        {
            set_attr( node, "text-transform", sym.get_text_transform() );
        }
        if (sym.get_line_spacing() != dfl.get_line_spacing() || explicit_defaults_ )
        {
            set_attr( node, "line-spacing", sym.get_line_spacing() );
        }
        if (sym.get_character_spacing() != dfl.get_character_spacing() || explicit_defaults_ )
        {
            set_attr( node, "character-spacing", sym.get_character_spacing() );
        }
        if (sym.get_label_position_tolerance() != dfl.get_label_position_tolerance() || explicit_defaults_ )
        {
            set_attr( node, "label-position-tolerance", sym.get_label_position_tolerance() );
        }
        if (sym.get_label_spacing() != dfl.get_label_spacing() || explicit_defaults_ )
        {
            set_attr( node, "spacing", sym.get_label_spacing() );
        }
        if (sym.get_minimum_distance() != dfl.get_minimum_distance() || explicit_defaults_ )
        {
            set_attr( node, "minimum-distance", sym.get_minimum_distance() );
        }
        if (sym.get_minimum_padding() != dfl.get_minimum_padding() || explicit_defaults_ )
        {
            set_attr( node, "minimum-padding", sym.get_minimum_padding() );
        }
        if (sym.get_allow_overlap() != dfl.get_allow_overlap() || explicit_defaults_ )
        {
            set_attr( node, "allow-overlap", sym.get_allow_overlap() );
        }
        if (sym.get_avoid_edges() != dfl.get_avoid_edges() || explicit_defaults_ )
        {
            set_attr( node, "avoid-edges", sym.get_avoid_edges() );
        }
        // for shield_symbolizer this is later overridden
        if (sym.get_text_opacity() != dfl.get_text_opacity() || explicit_defaults_ )
        {
            set_attr( node, "opacity", sym.get_text_opacity() );
        }
        if (sym.get_max_char_angle_delta() != dfl.get_max_char_angle_delta() || explicit_defaults_ )
        {
            set_attr( node, "max-char-angle-delta", sym.get_max_char_angle_delta() );
        }
        if (sym.get_horizontal_alignment() != dfl.get_horizontal_alignment() || explicit_defaults_ )
        {
            set_attr( node, "horizontal-alignment", sym.get_horizontal_alignment() );
        }
        if (sym.get_justify_alignment() != dfl.get_justify_alignment() || explicit_defaults_ )
        {
            set_attr( node, "justify-alignment", sym.get_justify_alignment() );
        }
    }


    void add_stroke_attributes(ptree & node, const stroke & strk)
    {

        stroke dfl = stroke();
        
        if ( strk.get_color() != dfl.get_color() || explicit_defaults_ )
        {
            set_attr( node, "stroke", strk.get_color() );
        }
        if ( strk.get_width() != dfl.get_width() || explicit_defaults_ )
        {
            set_attr( node, "stroke-width", strk.get_width() );
        }
        if ( strk.get_opacity() != dfl.get_opacity() || explicit_defaults_ )
        {
            set_attr( node, "stroke-opacity", strk.get_opacity() );
        }
        if ( strk.get_line_join() != dfl.get_line_join() || explicit_defaults_ )
        {
            set_attr( node, "stroke-linejoin", strk.get_line_join() );
        }
        if ( strk.get_line_cap() != dfl.get_line_cap() || explicit_defaults_ )
        {
            set_attr( node, "stroke-linecap", strk.get_line_cap() );
        }
        if ( strk.get_gamma() != dfl.get_gamma() || explicit_defaults_ )
        {
            set_attr( node, "stroke-gamma", strk.get_gamma());
        }
        if ( strk.dash_offset() != dfl.dash_offset() || explicit_defaults_ )
        {
            set_attr( node, "stroke-dash-offset", strk.dash_offset());
        }
        if ( ! strk.get_dash_array().empty() )
        {
            std::ostringstream os;
            const dash_array & dashes = strk.get_dash_array();
            for (unsigned i = 0; i < dashes.size(); ++i) {
                os << dashes[i].first << ", " << dashes[i].second;
                if ( i + 1 < dashes.size() ) os << ", ";
            }
            set_attr( node, "stroke-dasharray", os.str() );
        }
                
    }
    void add_metawriter_attributes(ptree &node, symbolizer_base const& sym)
    {
        if (!sym.get_metawriter_name().empty() || explicit_defaults_) {
            set_attr(node, "meta-writer", sym.get_metawriter_name());
        }
        if (!sym.get_metawriter_properties_overrides().empty() || explicit_defaults_) {
            set_attr(node, "meta-output", sym.get_metawriter_properties_overrides().to_string());
        }
    }

    ptree & rule_;
    bool explicit_defaults_;
};

void serialize_rule( ptree & style_node, const rule & r, bool explicit_defaults)
{
    ptree & rule_node = style_node.push_back(
        ptree::value_type("Rule", ptree() ))->second;

    rule dfl;
    if ( r.get_name() != dfl.get_name() )
    {
        set_attr(rule_node, "name", r.get_name());
    }
    if ( r.get_title() != dfl.get_title() )
    {
        set_attr(rule_node, "title", r.get_title());
    }

    if ( r.has_else_filter() )
    {
        rule_node.push_back( ptree::value_type(
                                 "ElseFilter", ptree()));
    }
    else
    {
        // filters were not comparable, perhaps should now compare expressions?
        expression_ptr const& expr = r.get_filter();
        std::string filter = mapnik::to_expression_string(*expr);
        std::string default_filter = mapnik::to_expression_string(*dfl.get_filter());
            
        if ( filter != default_filter)
        {
            rule_node.push_back( ptree::value_type(
                                     "Filter", ptree()))->second.put_value( filter );
        }
    }

    if (r.get_min_scale() != dfl.get_min_scale() )
    {
        ptree & min_scale = rule_node.push_back( ptree::value_type(
                                                     "MinScaleDenominator", ptree()))->second;
        min_scale.put_value( r.get_min_scale() );
    }

    if (r.get_max_scale() != dfl.get_max_scale() )
    {
        ptree & max_scale = rule_node.push_back( ptree::value_type(
                                                     "MaxScaleDenominator", ptree()))->second;
        max_scale.put_value( r.get_max_scale() );
    }

    rule::symbolizers::const_iterator begin = r.get_symbolizers().begin();
    rule::symbolizers::const_iterator end = r.get_symbolizers().end();
    serialize_symbolizer serializer( rule_node, explicit_defaults);
    std::for_each( begin, end , boost::apply_visitor( serializer ));
}

void serialize_style( ptree & map_node, Map::const_style_iterator style_it, bool explicit_defaults )
{
    const feature_type_style & style = style_it->second;
    const std::string & name = style_it->first;
    filter_mode_e filter_mode = style.get_filter_mode();

    ptree & style_node = map_node.push_back(
        ptree::value_type("Style", ptree()))->second;

    set_attr(style_node, "name", name);
    
    feature_type_style dfl;
    if (filter_mode != dfl.get_filter_mode() || explicit_defaults)
    {
        set_attr(style_node, "filter-mode", filter_mode);
    }

    rules::const_iterator it = style.get_rules().begin();
    rules::const_iterator end = style.get_rules().end();
    for (; it != end; ++it)
    {
        serialize_rule( style_node, * it , explicit_defaults);
    }

}

void serialize_fontset( ptree & map_node, Map::const_fontset_iterator fontset_it )
{
    const font_set & fontset = fontset_it->second;
    const std::string & name = fontset_it->first;

    ptree & fontset_node = map_node.push_back(
        ptree::value_type("FontSet", ptree()))->second;

    set_attr(fontset_node, "name", name);

    std::vector<std::string>::const_iterator it = fontset.get_face_names().begin();
    std::vector<std::string>::const_iterator end = fontset.get_face_names().end();
    for (; it != end; ++it)
    {
        ptree & font_node = fontset_node.push_back(
            ptree::value_type("Font", ptree()))->second;
        set_attr(font_node, "face-name", *it);
    }

}

void serialize_datasource( ptree & layer_node, datasource_ptr datasource)
{
    ptree & datasource_node = layer_node.push_back(
        ptree::value_type("Datasource", ptree()))->second;

    parameters::const_iterator it = datasource->params().begin();
    parameters::const_iterator end = datasource->params().end();
    for (; it != end; ++it)
    {
        boost::property_tree::ptree & param_node = datasource_node.push_back(
            boost::property_tree::ptree::value_type("Parameter",
                                                    boost::property_tree::ptree()))->second;
        param_node.put("<xmlattr>.name", it->first );
        param_node.put_value( it->second );

    }
}

void serialize_layer( ptree & map_node, const layer & layer, bool explicit_defaults )
{
    ptree & layer_node = map_node.push_back(
        ptree::value_type("Layer", ptree()))->second;
    if ( layer.name() != "" )
    {
        set_attr( layer_node, "name", layer.name() );
    }

    if ( layer.abstract() != "" )
    {
        set_attr( layer_node, "abstract", layer.abstract() );
    }

    if ( layer.title() != "" )
    {
        set_attr( layer_node, "title", layer.title() );
    }

    if ( layer.srs() != "" )
    {
        set_attr( layer_node, "srs", layer.srs() );
    }

    if ( !layer.isActive() || explicit_defaults )
    {
        set_attr/*<bool>*/( layer_node, "status", layer.isActive() );
    }
        
    if ( layer.clear_label_cache() || explicit_defaults )
    {        
        set_attr/*<bool>*/( layer_node, "clear-label-cache", layer.clear_label_cache() );
    }

    if ( layer.getMinZoom() )
    {
        set_attr( layer_node, "minzoom", layer.getMinZoom() );
    }

    if ( layer.getMaxZoom() != std::numeric_limits<double>::max() )
    {
        set_attr( layer_node, "maxzoom", layer.getMaxZoom() );
    }

    if ( layer.isQueryable() || explicit_defaults )
    {
        set_attr( layer_node, "queryable", layer.isQueryable() );
    }

    if ( layer.cache_features() || explicit_defaults )
    {        
        set_attr/*<bool>*/( layer_node, "cache-features", layer.cache_features() );
    }

    std::vector<std::string> const& style_names = layer.styles();
    for (unsigned i = 0; i < style_names.size(); ++i)
    {
        boost::property_tree::ptree & style_node = layer_node.push_back(
            boost::property_tree::ptree::value_type("StyleName",
                                                    boost::property_tree::ptree()))->second;
        style_node.put_value( style_names[i] );
    }

    datasource_ptr datasource = layer.datasource();
    if ( datasource )
    {
        serialize_datasource( layer_node, datasource );
    }
}

void serialize_metawriter(ptree & map_node, Map::const_metawriter_iterator metawriter_it, bool explicit_defaults)
{
    std::string const& name = metawriter_it->first;
    metawriter_ptr const& metawriter = metawriter_it->second;

    ptree & metawriter_node = map_node.push_back(
        ptree::value_type("MetaWriter", ptree()))->second;

    set_attr(metawriter_node, "name", name);
    metawriter_save(metawriter, metawriter_node, explicit_defaults);
}

void serialize_map(ptree & pt, Map const & map, bool explicit_defaults)
{

    ptree & map_node = pt.push_back(ptree::value_type("Map", ptree() ))->second;

    set_attr( map_node, "srs", map.srs() );

    optional<color> const& c = map.background();
    if ( c )
    {
        set_attr( map_node, "background-color", * c );
    }

    optional<std::string> const& image_filename = map.background_image();
    if ( image_filename )
    {
        set_attr( map_node, "background-image", *image_filename );
    }
    
    
    unsigned buffer_size = map.buffer_size();
    if ( buffer_size || explicit_defaults)
    {
        set_attr( map_node, "buffer-size", buffer_size ); 
    }

    optional<box2d<double> > const& maximum_extent = map.maximum_extent();
    if ( maximum_extent)
    {
        std::ostringstream s;
        s << std::setprecision(16)
          << maximum_extent->minx() << "," << maximum_extent->miny() << ","
          << maximum_extent->maxx() << "," << maximum_extent->maxy();
        set_attr( map_node, "maximum-extent", s.str() ); 
    }

    {
        Map::const_fontset_iterator it = map.fontsets().begin();
        Map::const_fontset_iterator end = map.fontsets().end();
        for (; it != end; ++it)
        {
            serialize_fontset( map_node, it);
        }
    }

    parameters extra_attr = map.get_extra_attributes();
    parameters::const_iterator p_it = extra_attr.begin();
    parameters::const_iterator p_end = extra_attr.end();
    for (; p_it != p_end; ++p_it)
    {
        set_attr( map_node, p_it->first, p_it->second ); 
    }

    Map::const_style_iterator it = map.styles().begin();
    Map::const_style_iterator end = map.styles().end();
    for (; it != end; ++it)
    {
        serialize_style( map_node, it, explicit_defaults);
    }

    std::vector<layer> const & layers = map.layers();
    for (unsigned i = 0; i < layers.size(); ++i )
    {
        serialize_layer( map_node, layers[i], explicit_defaults );
    }

    Map::const_metawriter_iterator m_it = map.begin_metawriters();
    Map::const_metawriter_iterator m_end = map.end_metawriters();
    for (; m_it != m_end; ++m_it) {
        serialize_metawriter(map_node, m_it, explicit_defaults);
    }
}

void save_map(Map const & map, std::string const& filename, bool explicit_defaults)
{
    ptree pt;
    serialize_map(pt,map,explicit_defaults);
    write_xml(filename,pt,std::locale(),boost::property_tree::xml_writer_make_settings(' ',4));
}

std::string save_map_to_string(Map const & map, bool explicit_defaults)
{
    ptree pt;
    serialize_map(pt,map,explicit_defaults);
    std::ostringstream ss;
    write_xml(ss,pt,boost::property_tree::xml_writer_make_settings(' ',4));
    return ss.str();
}

}
