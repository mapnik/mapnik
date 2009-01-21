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

#include <mapnik/ptree_helpers.hpp>

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

    std::string guess_type( const std::string & filename )
    {
        std::string::size_type idx = filename.find_last_of(".");
        if ( idx != std::string::npos ) {
            return filename.substr( idx + 1 );
        }
        return "<unknown>";
    }
    class serialize_symbolizer : public boost::static_visitor<>
    {
        public:
            serialize_symbolizer( ptree & r ) : rule_(r) {}

            void operator () ( const  point_symbolizer & sym )
            {
                ptree & sym_node = rule_.push_back(
                        ptree::value_type("PointSymbolizer", ptree()))->second;

                add_image_attributes( sym_node, sym );
            }

            void operator () ( const line_symbolizer & sym )
            {
                ptree & sym_node = rule_.push_back(
                        ptree::value_type("LineSymbolizer", ptree()))->second;
                const stroke & strk = sym.get_stroke();
                stroke dfl = stroke();

                if ( strk.get_color() != dfl.get_color() )
                {
                    set_css( sym_node, "stroke", strk.get_color() );    
                }
                if ( strk.get_width() != dfl.get_width() )
                {
                    set_css( sym_node, "stroke-width", strk.get_width() );    
                }
                if ( strk.get_opacity() != dfl.get_opacity() )
                {
                    set_css( sym_node, "stroke-opacity", strk.get_opacity() );    
                }
                if ( strk.get_line_join() != dfl.get_line_join() )
                {
                    set_css( sym_node, "stroke-linejoin", strk.get_line_join() );    
                }
                if ( strk.get_line_cap() != dfl.get_line_cap() )
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

                if ( sym.get_fill() != dfl.get_fill() )
                {
                    set_css( sym_node, "fill", sym.get_fill() );    
                }
                if ( sym.get_opacity() != dfl.get_opacity() )
                {
                    set_css( sym_node, "fill-opacity", sym.get_opacity() );    
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
                rule_.push_back(
                        ptree::value_type("RasterSymbolizer", ptree()));
            }

            void operator () ( const shield_symbolizer & sym )
            {
                ptree & sym_node = rule_.push_back(
                        ptree::value_type("ShieldSymbolizer",
                        ptree()))->second;

                add_font_attributes( sym_node, sym);
                add_image_attributes( sym_node, sym);
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

                if ( sym.get_fill() != dfl.get_fill() )
                {
                    set_css( sym_node, "fill", sym.get_fill() );    
                }
                if ( sym.get_opacity() != dfl.get_opacity() )
                {
                    set_css( sym_node, "fill-opacity", sym.get_opacity() );    
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
                const std::string & filename = sym.get_filename();
                if ( ! filename.empty() ) {
                    set_attr( node, "file", filename );
                    set_attr( node, "type", guess_type( filename ) );

                    boost::shared_ptr<ImageData32> img = sym.get_image();
                    if ( img )
                    {
                        if ( img->width() > 0)
                        {
                            set_attr( node, "width", img->width() );
                        }
                        if ( img->height() > 0)
                        {
                            set_attr( node, "height", img->height() );
                        }
                    }

                }
            }
            void add_font_attributes(ptree & node, const text_symbolizer & sym)
            {
                const std::string & name = sym.get_name();
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
                text_symbolizer dfl("<no default>", "<no default>",
                                    0, color(0,0,0) );

                position displacement = sym.get_displacement();
                if ( displacement.get<0>() != dfl.get_displacement().get<0>() )
                {
                    set_attr( node, "dx", displacement.get<0>() );    
                }
                if ( displacement.get<1>() != dfl.get_displacement().get<1>() )
                {
                    set_attr( node, "dy", displacement.get<1>() );    
                }

                if (sym.get_label_placement() != dfl.get_label_placement() )
                {
                    set_attr( node, "placement", sym.get_label_placement() );    
                }
                if (sym.get_halo_radius() != dfl.get_halo_radius())
                {
                    set_attr( node, "halo_radius", sym.get_halo_radius() );    
                }
                const color & c = sym.get_halo_fill();
                if ( c != dfl.get_halo_fill() )
                {
                    set_attr( node, "halo_fill", c );    
                }
                if (sym.get_text_ratio() != dfl.get_text_ratio() )
                {
                    set_attr( node, "text_ratio", sym.get_text_ratio() );    
                }
                if (sym.get_wrap_width() != dfl.get_wrap_width())
                {
                    set_attr( node, "wrap_width", sym.get_wrap_width() );    
                }
                if (sym.get_label_spacing() != dfl.get_label_spacing())
                {
                    set_attr( node, "spacing", sym.get_label_spacing() );    
                }
                if (sym.get_minimum_distance() != dfl.get_minimum_distance())
                {
                    set_attr( node, "min_distance", sym.get_minimum_distance() );    
                }
                if (sym.get_allow_overlap() != dfl.get_allow_overlap() )
                {
                    set_attr( node, "allow_overlap", sym.get_allow_overlap() );    
                }
            }
            ptree & rule_;
    };

    void serialize_rule( ptree & style_node, const rule_type & rule)
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
            // filters are not comparable, so compare strings for now
            std::string filter = rule.get_filter()->to_string();
            std::string default_filter = dfl.get_filter()->to_string();
            if ( filter != default_filter)
            {
                rule_node.push_back( ptree::value_type(
                            "Filter", ptree()))->second.put_own( filter );
            }
        }

        if (rule.get_min_scale() != dfl.get_min_scale())
        {
            ptree & min_scale = rule_node.push_back( ptree::value_type(
                    "MinScaleDenominator", ptree()))->second;
            min_scale.put_own( rule.get_min_scale() );
        }

        if (rule.get_max_scale() != dfl.get_max_scale() )
        {
            ptree & max_scale = rule_node.push_back( ptree::value_type(
                    "MaxScaleDenominator", ptree()))->second;
            max_scale.put_own( rule.get_max_scale() );
        }

        symbolizers::const_iterator begin = rule.get_symbolizers().begin();
        symbolizers::const_iterator end = rule.get_symbolizers().end();
        serialize_symbolizer serializer( rule_node );
        std::for_each( begin, end , boost::apply_visitor( serializer ));
    }

    void serialize_style( ptree & map_node, Map::const_style_iterator style_it )
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
            serialize_rule( style_node, * it );    
        }

    }

    void serialize_fontset( ptree & map_node, Map::const_fontset_iterator fontset_it )
    {
        const FontSet & fontset = fontset_it->second;
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
            param_node.put_own( it->second );
                
        }
    }

    void serialize_layer( ptree & map_node, const Layer & layer ) 
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
        
        set_attr/*<bool>*/( layer_node, "status", layer.isActive() );
        set_attr/*<bool>*/( layer_node, "clear_label_cache", layer.clear_label_cache() );

        if ( layer.getMinZoom() )
        {
            set_attr( layer_node, "minzoom", layer.getMinZoom() );
        }

        if ( layer.getMaxZoom() != std::numeric_limits<double>::max() )
        {
            set_attr( layer_node, "maxzoom", layer.getMaxZoom() );
        }

        if ( layer.isQueryable() )
        {
            set_attr( layer_node, "queryable", layer.isQueryable() );
        }
        
        std::vector<std::string> const& style_names = layer.styles();
        for (unsigned i = 0; i < style_names.size(); ++i)
        {
            boost::property_tree::ptree & style_node = layer_node.push_back(
                    boost::property_tree::ptree::value_type("StyleName",
                    boost::property_tree::ptree()))->second;
            style_node.put_own( style_names[i] );
        }

        datasource_ptr datasource = layer.datasource();
        if ( datasource )
        {
            serialize_datasource( layer_node, datasource );
        }
    }

    void save_map(Map const & map, std::string const& filename)
    {
        ptree pt;

        ptree & map_node = pt.push_back(ptree::value_type("Map", ptree() ))->second;

        set_attr( map_node, "srs", map.srs() );
        
        optional<color> c = map.background();
        if ( c )
        {
            set_attr( map_node, "bgcolor", * c );    
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
            serialize_style( map_node, it);
        }

        std::vector<Layer> const & layers = map.layers();
        for (unsigned i = 0; i < layers.size(); ++i )
        {
            serialize_layer( map_node, layers[i] );
        }

        
        write_xml(filename,pt);
    }

}
