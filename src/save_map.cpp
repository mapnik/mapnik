/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

// mapnik
#include <mapnik/layer.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/save_map.hpp>
#include <mapnik/map.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/text_placements/simple.hpp>
#include <mapnik/text_placements/list.hpp>
#include <mapnik/text_placements/dummy.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/image_filter_types.hpp>
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

    void operator () ( point_symbolizer const& sym )
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
        if (sym.get_image_transform())
        {
            std::string tr_str = sym.get_image_transform_string();
            set_attr( sym_node, "transform", tr_str );
        }
        serialize_symbolizer_base(sym_node, sym);
    }

    void operator () ( line_symbolizer const& sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("LineSymbolizer", ptree()))->second;

        const stroke & strk =  sym.get_stroke();
        add_stroke_attributes(sym_node, strk);

        line_symbolizer dfl;
        if ( sym.get_rasterizer() != dfl.get_rasterizer() || explicit_defaults_ )
        {
            set_attr( sym_node, "rasterizer", sym.get_rasterizer() );
        }
        serialize_symbolizer_base(sym_node, sym);
    }

    void operator () ( line_pattern_symbolizer const& sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("LinePatternSymbolizer",
                              ptree()))->second;

        add_image_attributes( sym_node, sym );
        serialize_symbolizer_base(sym_node, sym);
    }

    void operator () ( polygon_symbolizer const& sym )
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
        if ( sym.get_gamma_method() != dfl.get_gamma_method() || explicit_defaults_ )
        {
            set_attr( sym_node, "gamma-method", sym.get_gamma_method() );
        }
        serialize_symbolizer_base(sym_node, sym);
    }

    void operator () ( polygon_pattern_symbolizer const& sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("PolygonPatternSymbolizer",
                              ptree()))->second;
        polygon_pattern_symbolizer dfl(parse_path(""));

        if ( sym.get_alignment() != dfl.get_alignment() || explicit_defaults_ )
        {
            set_attr( sym_node, "alignment", sym.get_alignment() );
        }
        if ( sym.get_gamma() != dfl.get_gamma() || explicit_defaults_ )
        {
            set_attr( sym_node, "gamma", sym.get_gamma() );
        }
        if ( sym.get_gamma_method() != dfl.get_gamma_method() || explicit_defaults_ )
        {
            set_attr( sym_node, "gamma-method", sym.get_gamma_method() );
        }
        add_image_attributes( sym_node, sym );
        serialize_symbolizer_base(sym_node, sym);
    }

    void operator () ( raster_symbolizer const& sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("RasterSymbolizer", ptree()))->second;
        raster_symbolizer dfl;

        if ( sym.get_scaling_method() != dfl.get_scaling_method() || explicit_defaults_ )
        {
            set_attr( sym_node, "scaling", *scaling_method_to_string(sym.get_scaling_method()) );
        }

        if ( sym.get_opacity() != dfl.get_opacity() || explicit_defaults_ )
        {
            set_attr( sym_node, "opacity", sym.get_opacity() );
        }

        if ( sym.get_mesh_size() != dfl.get_mesh_size() || explicit_defaults_ )
        {
            set_attr( sym_node, "mesh-size", sym.get_mesh_size() );
        }

        if (sym.get_colorizer())
        {
            serialize_raster_colorizer(sym_node, sym.get_colorizer(),
                                       explicit_defaults_);
        }

        boost::optional<bool> premultiplied = sym.premultiplied();
        if (premultiplied)
        {
            set_attr( sym_node, "premultiplied", *sym.premultiplied());
        }

        serialize_symbolizer_base(sym_node, sym);
    }

    void operator () ( shield_symbolizer const& sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("ShieldSymbolizer",
                              ptree()))->second;

        add_font_attributes(sym_node, sym);
        add_image_attributes(sym_node, sym);

        // pseudo-default-construct a shield_symbolizer. It is used
        // to avoid printing of attributes with default values without
        // repeating the default values here.
        // maybe add a real, explicit default-ctor?

        shield_symbolizer dfl;

        if (sym.get_unlock_image() != dfl.get_unlock_image() || explicit_defaults_)
        {
            set_attr(sym_node, "unlock-image", sym.get_unlock_image());
        }

        if (sym.get_placement_options()->defaults.format.text_opacity !=
                dfl.get_placement_options()->defaults.format.text_opacity || explicit_defaults_)
        {
            set_attr(sym_node, "text-opacity", sym.get_placement_options()->defaults.format.text_opacity);
        }
        position displacement = sym.get_shield_displacement();
        if (displacement.first != dfl.get_shield_displacement().first || explicit_defaults_)
        {
            set_attr(sym_node, "shield-dx", displacement.first);
        }
        if (displacement.second != dfl.get_shield_displacement().second || explicit_defaults_)
        {
            set_attr(sym_node, "shield-dy", displacement.second);
        }
        if (sym.get_image_transform())
        {
            std::string tr_str = sym.get_image_transform_string();
            set_attr( sym_node, "transform", tr_str );
        }
        serialize_symbolizer_base(sym_node, sym);
    }

    void operator () ( text_symbolizer const& sym )
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("TextSymbolizer",
                              ptree()))->second;

        add_font_attributes( sym_node, sym);
        serialize_symbolizer_base(sym_node, sym);
    }

    void operator () ( building_symbolizer const& sym )
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
        if (sym.height())
        {
            set_attr( sym_node, "height", mapnik::to_expression_string(*sym.height()) );
        }
        serialize_symbolizer_base(sym_node, sym);
    }

    void operator () ( markers_symbolizer const& sym)
    {
        ptree & sym_node = rule_.push_back(
            ptree::value_type("MarkersSymbolizer", ptree()))->second;
        markers_symbolizer dfl(parse_path("")); //TODO: Parameter?
        if (sym.get_filename())
        {
            std::string filename = path_processor_type::to_string(*sym.get_filename());
            set_attr( sym_node, "file", filename );
        }
        if (sym.get_allow_overlap() != dfl.get_allow_overlap() || explicit_defaults_)
        {
            set_attr( sym_node, "allow-overlap", sym.get_allow_overlap() );
        }
        if (sym.get_ignore_placement() != dfl.get_ignore_placement() || explicit_defaults_)
        {
            set_attr( sym_node, "ignore-placement", sym.get_ignore_placement() );
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
        if (sym.get_fill_opacity() != dfl.get_fill_opacity() || explicit_defaults_)
        {
            set_attr( sym_node, "fill-opacity", sym.get_fill_opacity() );
        }
        if (sym.get_opacity() != dfl.get_opacity() || explicit_defaults_)
        {
            set_attr( sym_node, "opacity", sym.get_opacity() );
        }
        if (sym.get_width() != dfl.get_width() || explicit_defaults_)
        {
            set_attr( sym_node, "width", to_expression_string(*sym.get_width()) );
        }
        if (sym.get_height() != dfl.get_height() || explicit_defaults_)
        {
            set_attr( sym_node, "height", to_expression_string(*sym.get_height()) );
        }
        if (sym.get_marker_placement() != dfl.get_marker_placement() || explicit_defaults_)
        {
            set_attr( sym_node, "placement", sym.get_marker_placement() );
        }
        if (sym.get_image_transform())
        {
            std::string tr_str = sym.get_image_transform_string();
            set_attr( sym_node, "transform", tr_str );
        }

        boost::optional<stroke> const& strk = sym.get_stroke();
        if (strk)
        {
            add_stroke_attributes(sym_node, *strk);
        }

        serialize_symbolizer_base(sym_node, sym);
    }

    template <typename Symbolizer>
    void operator () ( Symbolizer const& sym)
    {
        // not-supported
#ifdef MAPNIK_DEBUG
        MAPNIK_LOG_WARN(save_map) << typeid(sym).name() << " is not supported";
#endif
    }

private:
    serialize_symbolizer();

    void serialize_symbolizer_base(ptree & node, symbolizer_base const& sym)
    {
        symbolizer_base dfl = symbolizer_base();
        if (sym.get_transform())
        {
            std::string tr_str = sym.get_transform_string();
            set_attr( node, "geometry-transform", tr_str );
        }
        if (sym.clip() != dfl.clip() || explicit_defaults_)
        {
            set_attr( node, "clip", sym.clip() );
        }
        if (sym.simplify_algorithm() != dfl.simplify_algorithm() || explicit_defaults_)
        {
            set_attr( node, "simplify-algorithm", *simplify_algorithm_to_string(sym.simplify_algorithm()) );
        }
        if (sym.simplify_tolerance() != dfl.simplify_tolerance() || explicit_defaults_)
        {
            set_attr( node, "simplify-tolerance", sym.simplify_tolerance() );
        }
        if (sym.smooth() != dfl.smooth() || explicit_defaults_)
        {
            set_attr( node, "smooth", sym.smooth() );
        }
        if (sym.comp_op() != dfl.comp_op() || explicit_defaults_)
        {
            set_attr( node, "comp-op", *comp_op_to_string(sym.comp_op()) );
        }
    }

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
            if (stops[i].get_label()!=std::string(""))
                set_attr(stop_node, "label", stops[i].get_label());
        }

    }

    void add_image_attributes(ptree & node, symbolizer_with_image const& sym)
    {
        if (sym.get_filename())
        {
            std::string filename = path_processor_type::to_string( *sym.get_filename());
            set_attr( node, "file", filename );
        }
        if (sym.get_opacity() != 1.0 || explicit_defaults_ )
        {
            set_attr( node, "opacity", sym.get_opacity() );
        }
    }

    void add_font_attributes(ptree & node, const text_symbolizer & sym)
    {
        text_placements_ptr p = sym.get_placement_options();
        p->defaults.to_xml(node, explicit_defaults_);
        /* Known types:
           - text_placements_dummy: no handling required
           - text_placements_simple: positions string
           - text_placements_list: list string
        */
        text_placements_simple *simple = dynamic_cast<text_placements_simple *>(p.get());
        text_placements_list *list = dynamic_cast<text_placements_list *>(p.get());
        if (simple) {
            set_attr(node, "placement-type", "simple");
            set_attr(node, "placements", simple->get_positions());
        }
        if (list) {
            set_attr(node, "placement-type", "list");
            unsigned i;
            //dfl = last properties passed as default so only attributes that change are actually written
            text_symbolizer_properties *dfl = &(list->defaults);
            for (i=0; i < list->size(); i++) {
                ptree &placement_node = node.push_back(ptree::value_type("Placement", ptree()))->second;
                list->get(i).to_xml(placement_node, explicit_defaults_, *dfl);
                dfl = &(list->get(i));
            }
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
        if ( strk.get_gamma_method() != dfl.get_gamma_method() || explicit_defaults_ )
        {
            set_attr( node, "stroke-gamma-method", strk.get_gamma_method() );
        }
        if ( strk.dash_offset() != dfl.dash_offset() || explicit_defaults_ )
        {
            set_attr( node, "stroke-dashoffset", strk.dash_offset());
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

    if ( r.has_else_filter() )
    {
        rule_node.push_back( ptree::value_type(
                                 "ElseFilter", ptree()));
    }
    else if ( r.has_also_filter() )
    {
        rule_node.push_back( ptree::value_type(
                                 "AlsoFilter", ptree()));
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
    feature_type_style const& style = style_it->second;
    std::string const& name = style_it->first;

    ptree & style_node = map_node.push_back(
        ptree::value_type("Style", ptree()))->second;

    set_attr(style_node, "name", name);

    feature_type_style dfl;
    filter_mode_e filter_mode = style.get_filter_mode();
    if (filter_mode != dfl.get_filter_mode() || explicit_defaults)
    {
        set_attr(style_node, "filter-mode", filter_mode);
    }

    double opacity = style.get_opacity();
    if (opacity != dfl.get_opacity() || explicit_defaults)
    {
        set_attr(style_node, "opacity", opacity);
    }

    boost::optional<composite_mode_e> comp_op = style.comp_op();
    if (comp_op)
    {
        set_attr(style_node, "comp-op", *comp_op_to_string(*comp_op));
    }
    else if (explicit_defaults)
    {
        set_attr(style_node, "comp-op", "src-over");
    }

    if (style.image_filters().size() > 0)
    {
        std::string filters_str;
        std::back_insert_iterator<std::string> sink(filters_str);
        if (generate_image_filters(sink, style.image_filters()))
        {
            set_attr(style_node, "image-filters", filters_str);
        }
    }

    if (style.direct_image_filters().size() > 0)
    {
        std::string filters_str;
        std::back_insert_iterator<std::string> sink(filters_str);
        if (generate_image_filters(sink, style.direct_image_filters()))
        {
            set_attr(style_node, "direct-image-filters", filters_str);
        }
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
    font_set const& fontset = fontset_it->second;
    std::string const& name = fontset_it->first;

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

class serialize_type : public boost::static_visitor<>
{
public:
    serialize_type( boost::property_tree::ptree & node):
        node_(node) {}

    void operator () ( int val ) const
    {
        node_.put("<xmlattr>.type", "int" );
    }

    void operator () ( double val ) const
    {
        node_.put("<xmlattr>.type", "float" );
    }

    void operator () ( std::string const& val ) const
    {
        node_.put("<xmlattr>.type", "string" );
    }

    void operator () ( mapnik::value_null val ) const
    {
        node_.put("<xmlattr>.type", "string" );
    }

private:
    boost::property_tree::ptree & node_;
};

void serialize_parameters( ptree & map_node, mapnik::parameters const& params)
{
    if (params.size()) {
        ptree & params_node = map_node.push_back(
            ptree::value_type("Parameters", ptree()))->second;

        parameters::const_iterator it = params.begin();
        parameters::const_iterator end = params.end();
        for (; it != end; ++it)
        {
            boost::property_tree::ptree & param_node = params_node.push_back(
                boost::property_tree::ptree::value_type("Parameter",
                                                        boost::property_tree::ptree()))->second;
            param_node.put("<xmlattr>.name", it->first );
            param_node.put_value( it->second );
            boost::apply_visitor(serialize_type(param_node),it->second);
        }
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

    if ( layer.srs() != "" )
    {
        set_attr( layer_node, "srs", layer.srs() );
    }

    if ( !layer.active() || explicit_defaults )
    {
        set_attr/*<bool>*/( layer_node, "status", layer.active() );
    }

    if ( layer.clear_label_cache() || explicit_defaults )
    {
        set_attr/*<bool>*/( layer_node, "clear-label-cache", layer.clear_label_cache() );
    }

    if ( layer.min_zoom() )
    {
        set_attr( layer_node, "minzoom", layer.min_zoom() );
    }

    if ( layer.max_zoom() != std::numeric_limits<double>::max() )
    {
        set_attr( layer_node, "maxzoom", layer.max_zoom() );
    }

    if ( layer.queryable() || explicit_defaults )
    {
        set_attr( layer_node, "queryable", layer.queryable() );
    }

    if ( layer.cache_features() || explicit_defaults )
    {
        set_attr/*<bool>*/( layer_node, "cache-features", layer.cache_features() );
    }

    if ( layer.group_by() != "" || explicit_defaults )
    {
        set_attr( layer_node, "group-by", layer.group_by() );
    }

    int buffer_size = layer.buffer_size();
    if ( buffer_size || explicit_defaults)
    {
        set_attr( layer_node, "buffer-size", buffer_size );
    }

    optional<box2d<double> > const& maximum_extent = layer.maximum_extent();
    if ( maximum_extent)
    {
        std::ostringstream s;
        s << std::setprecision(16)
          << maximum_extent->minx() << "," << maximum_extent->miny() << ","
          << maximum_extent->maxx() << "," << maximum_extent->maxy();
        set_attr( layer_node, "maximum-extent", s.str() );
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

    int buffer_size = map.buffer_size();
    if ( buffer_size || explicit_defaults)
    {
        set_attr( map_node, "buffer-size", buffer_size );
    }

    std::string const& base_path = map.base_path();
    if ( !base_path.empty() || explicit_defaults)
    {
        set_attr( map_node, "base", base_path );
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

    serialize_parameters( map_node, map.get_extra_parameters());

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
