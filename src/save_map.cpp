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
#include <mapnik/image_util.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/expression_string.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
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
                    set_attr( sym_node, "allow_overlap", sym.get_allow_overlap() );
                }
                if ( sym.get_opacity() != dfl.get_opacity() || explicit_defaults_ )
                {
                    set_attr( sym_node, "opacity", sym.get_opacity() );
                }
            }

            void operator () ( const line_symbolizer & sym )
            {
                ptree & sym_node = rule_.push_back(
                        ptree::value_type("LineSymbolizer", ptree()))->second;
                const stroke & strk = sym.get_stroke();
                stroke dfl = stroke();

                if ( strk.get_color() != dfl.get_color() || explicit_defaults_ )
                {
                    set_css( sym_node, "stroke", strk.get_color() );
                }
                if ( strk.get_width() != dfl.get_width() || explicit_defaults_ )
                {
                    set_css( sym_node, "stroke-width", strk.get_width() );
                }
                if ( strk.get_opacity() != dfl.get_opacity() || explicit_defaults_ )
                {
                    set_css( sym_node, "stroke-opacity", strk.get_opacity() );
                }
                if ( strk.get_line_join() != dfl.get_line_join() || explicit_defaults_ )
                {
                    set_css( sym_node, "stroke-linejoin", strk.get_line_join() );
                }
                if ( strk.get_line_cap() != dfl.get_line_cap() || explicit_defaults_ )
                {
                    set_css( sym_node, "stroke-linecap", strk.get_line_cap() );
                }
                if ( ! strk.get_dash_array().empty() )
                {
                    std::ostringstream os;
                    const dash_array & dashes = strk.get_dash_array();
                    for (unsigned i = 0; i < dashes.size(); ++i) {
                        os << dashes[i].first << ", " << dashes[i].second;
                        if ( i + 1 < dashes.size() ) os << ", ";
                    }
                    set_css( sym_node, "stroke-dasharray", os.str() );
                }
            }

            void operator () ( const line_pattern_symbolizer & sym )
            {
                ptree & sym_node = rule_.push_back(
                        ptree::value_type("LinePatternSymbolizer",
                        ptree()))->second;

                add_image_attributes( sym_node, sym );
            }

            void operator () ( const polygon_symbolizer & sym )
            {
                ptree & sym_node = rule_.push_back(
                        ptree::value_type("PolygonSymbolizer", ptree()))->second;
                polygon_symbolizer dfl;

                if ( sym.get_fill() != dfl.get_fill() || explicit_defaults_ )
                {
                    set_css( sym_node, "fill", sym.get_fill() );
                }
                if ( sym.get_opacity() != dfl.get_opacity() || explicit_defaults_ )
                {
                    set_css( sym_node, "fill-opacity", sym.get_opacity() );
                }
                if ( sym.get_gamma() != dfl.get_gamma() || explicit_defaults_ )
                {
                    set_css( sym_node, "gamma", sym.get_gamma() );
                }
            }

            void operator () ( const polygon_pattern_symbolizer & sym )
            {
                ptree & sym_node = rule_.push_back(
                        ptree::value_type("PolygonPatternSymbolizer",
                        ptree()))->second;

                add_image_attributes( sym_node, sym );
            }

            void operator () ( const raster_symbolizer & sym )
            {
                ptree & sym_node = rule_.push_back(
                        ptree::value_type("RasterSymbolizer", ptree()))->second;
                raster_symbolizer dfl;

                if ( sym.get_mode() != dfl.get_mode() || explicit_defaults_ )
                {
                    set_css( sym_node, "mode", sym.get_mode() );
                }
                if ( sym.get_scaling() != dfl.get_scaling() || explicit_defaults_ )
                {
                    set_css( sym_node, "scaling", sym.get_scaling() );
                }
                if ( sym.get_opacity() != dfl.get_opacity() || explicit_defaults_ )
                {
                    set_css( sym_node, "opacity", sym.get_opacity() );
                }
            }

            void operator () ( const shield_symbolizer & sym )
            {
                ptree & sym_node = rule_.push_back(
                        ptree::value_type("ShieldSymbolizer",
                        ptree()))->second;

                add_font_attributes( sym_node, sym);
                add_image_attributes( sym_node, sym);

                // pseudo-default-construct a shield_symbolizer. It is used
                // to avoid printing of attributes with default values without
                // repeating the default values here.
                // maybe add a real, explicit default-ctor?

                //FIXME pls
                /*
                shield_symbolizer sym_dfl("<no default>", "<no default>", 0, color(0,0,0), "<no default>", "<no default>");
                if (sym.get_unlock_image() != sym_dfl.get_unlock_image() || explicit_defaults_ )
                {
                    set_attr( sym_node, "unlock_image", sym.get_unlock_image() );
                }
                if (sym.get_no_text() != sym_dfl.get_no_text() || explicit_defaults_ )
                {
                    set_attr( sym_node, "no_text", sym.get_no_text() );
                }
                */
            }

            void operator () ( const text_symbolizer & sym )
            {
                ptree & sym_node = rule_.push_back(
                        ptree::value_type("TextSymbolizer",
                        ptree()))->second;

                add_font_attributes( sym_node, sym);

            }

            void operator () ( const building_symbolizer & sym )
            {
                ptree & sym_node = rule_.push_back(
                        ptree::value_type("BuildingSymbolizer", ptree()))->second;
                building_symbolizer dfl;

                if ( sym.get_fill() != dfl.get_fill() || explicit_defaults_ )
                {
                    set_css( sym_node, "fill", sym.get_fill() );
                }
                if ( sym.get_opacity() != dfl.get_opacity() || explicit_defaults_ )
                {
                    set_css( sym_node, "fill-opacity", sym.get_opacity() );
                }
                if ( sym.height() != dfl.height() || explicit_defaults_ )
                {
                    set_css( sym_node, "height", sym.height() );
                }
            }

          void operator () ( markers_symbolizer const& )
          {
             // FIXME!!!!!
          }
        private:
            serialize_symbolizer();
            void add_image_attributes(ptree & node, const symbolizer_with_image & sym)
            {
                const std::string & filename = path_processor_type::to_string( *sym.get_filename());
                if ( ! filename.empty() ) {
                    set_attr( node, "file", filename );
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
                    set_attr( node, "face_name", face_name );
                }
                const std::string & fontset_name = sym.get_fontset().get_name();
                if ( ! fontset_name.empty() ) {
                    set_attr( node, "fontset_name", fontset_name );
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
                    set_attr( node, "vertical_alignment", sym.get_vertical_alignment() );
                }

                if (sym.get_halo_radius() != dfl.get_halo_radius() || explicit_defaults_ )
                {
                    set_attr( node, "halo_radius", sym.get_halo_radius() );
                }
                const color & c = sym.get_halo_fill();
                if ( c != dfl.get_halo_fill() || explicit_defaults_ )
                {
                    set_attr( node, "halo_fill", c );
                }
                if (sym.get_text_ratio() != dfl.get_text_ratio() || explicit_defaults_ )
                {
                    set_attr( node, "text_ratio", sym.get_text_ratio() );
                }
                if (sym.get_wrap_width() != dfl.get_wrap_width() || explicit_defaults_ )
                {
                    set_attr( node, "wrap_width", sym.get_wrap_width() );
                }
                if (sym.get_wrap_before() != dfl.get_wrap_before() || explicit_defaults_ )
                {
                    set_attr( node, "wrap_before", sym.get_wrap_before() );
                }
                if (sym.get_wrap_char() != dfl.get_wrap_char() || explicit_defaults_ )
                {
                    set_attr( node, "wrap_character", std::string(1, sym.get_wrap_char()) );
                }
                if (sym.get_text_convert() != dfl.get_text_convert() || explicit_defaults_ )
                {
                    set_attr( node, "text_convert", sym.get_text_convert() );
                }
                if (sym.get_line_spacing() != dfl.get_line_spacing() || explicit_defaults_ )
                {
                    set_attr( node, "line_spacing", sym.get_line_spacing() );
                }
                if (sym.get_character_spacing() != dfl.get_character_spacing() || explicit_defaults_ )
                {
                    set_attr( node, "character_spacing", sym.get_character_spacing() );
                }
                if (sym.get_label_spacing() != dfl.get_label_spacing() || explicit_defaults_ )
                {
                    set_attr( node, "spacing", sym.get_label_spacing() );
                }
                if (sym.get_minimum_distance() != dfl.get_minimum_distance() || explicit_defaults_ )
                {
                    set_attr( node, "min_distance", sym.get_minimum_distance() );
                }
                if (sym.get_allow_overlap() != dfl.get_allow_overlap() || explicit_defaults_ )
                {
                    set_attr( node, "allow_overlap", sym.get_allow_overlap() );
                }
                if (sym.get_avoid_edges() != dfl.get_avoid_edges() || explicit_defaults_ )
                {
                    set_attr( node, "avoid_edges", sym.get_avoid_edges() );
                }
                if (sym.get_opacity() != dfl.get_opacity() || explicit_defaults_ )
                {
                    set_attr( node, "opacity", sym.get_opacity() );
                }
                if (sym.get_horizontal_alignment() != dfl.get_horizontal_alignment() || explicit_defaults_ )
                {
                    set_attr( node, "horizontal_alignment", sym.get_horizontal_alignment() );
                }
                if (sym.get_justify_alignment() != dfl.get_justify_alignment() || explicit_defaults_ )
                {
                    set_attr( node, "justify_alignment", sym.get_justify_alignment() );
                }
            }
            ptree & rule_;
            bool explicit_defaults_;
    };

    void serialize_rule( ptree & style_node, const rule_type & rule, bool explicit_defaults)
    {
        ptree & rule_node = style_node.push_back(
                ptree::value_type("Rule", ptree() ))->second;

        rule_type dfl;
        if ( rule.get_name() != dfl.get_name() )
        {
            set_attr(rule_node, "name", rule.get_name());
        }
        if ( rule.get_title() != dfl.get_title() )
        {
            set_attr(rule_node, "title", rule.get_title());
        }

        if ( rule.has_else_filter() )
        {
            rule_node.push_back( ptree::value_type(
                    "ElseFilter", ptree()));
        }
        else
        {
            // filters were not comparable, perhaps should now compare expressions?
            expression_ptr const& expr = rule.get_filter();
	    std::string filter = mapnik::to_expression_string(*expr);
            std::string default_filter = mapnik::to_expression_string(*dfl.get_filter());
            
            if ( filter != default_filter)
            {
                rule_node.push_back( ptree::value_type(
                            "Filter", ptree()))->second.put_value( filter );
            }
        }

        if (rule.get_min_scale() != dfl.get_min_scale() )
        {
            ptree & min_scale = rule_node.push_back( ptree::value_type(
                    "MinScaleDenominator", ptree()))->second;
            min_scale.put_value( rule.get_min_scale() );
        }

        if (rule.get_max_scale() != dfl.get_max_scale() )
        {
            ptree & max_scale = rule_node.push_back( ptree::value_type(
                    "MaxScaleDenominator", ptree()))->second;
            max_scale.put_value( rule.get_max_scale() );
        }

        rule_type::symbolizers::const_iterator begin = rule.get_symbolizers().begin();
        rule_type::symbolizers::const_iterator end = rule.get_symbolizers().end();
        serialize_symbolizer serializer( rule_node, explicit_defaults);
        std::for_each( begin, end , boost::apply_visitor( serializer ));
    }

    void serialize_style( ptree & map_node, Map::const_style_iterator style_it, bool explicit_defaults )
    {
        const feature_type_style & style = style_it->second;
        const std::string & name = style_it->first;

        ptree & style_node = map_node.push_back(
                ptree::value_type("Style", ptree()))->second;

        set_attr(style_node, "name", name);

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
            set_attr(font_node, "face_name", *it);
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
            set_attr/*<bool>*/( layer_node, "clear_label_cache", layer.clear_label_cache() );
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

    void serialize_map(ptree & pt, Map const & map, bool explicit_defaults)
    {

        ptree & map_node = pt.push_back(ptree::value_type("Map", ptree() ))->second;

        set_attr( map_node, "srs", map.srs() );

        optional<color> c = map.background();
        if ( c )
        {
            set_attr( map_node, "bgcolor", * c );
        }

        unsigned buffer_size = map.buffer_size();
        if ( buffer_size || explicit_defaults)
        {
            set_attr( map_node, "buffer_size", buffer_size ); 
        }

        {
            Map::const_fontset_iterator it = map.fontsets().begin();
            Map::const_fontset_iterator end = map.fontsets().end();
            for (; it != end; ++it)
            {
                serialize_fontset( map_node, it);
            }
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
