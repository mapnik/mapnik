/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include <mapnik/rule.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/save_map.hpp>
#include <mapnik/map.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/text/placements/simple.hpp>
#include <mapnik/text/placements/list.hpp>
#include <mapnik/text/placements/dummy.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/image_filter.hpp>
#include <mapnik/image_filter_types.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/symbolizer_utils.hpp>
#include <mapnik/transform/transform_processor.hpp>
#include <mapnik/group/group_rule.hpp>
#include <mapnik/group/group_layout.hpp>
#include <mapnik/group/group_symbolizer_properties.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/util/variant_io.hpp>
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/optional.hpp>
#include <boost/version.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <iostream>

namespace mapnik {
using boost::optional;
using boost::property_tree::ptree;

void serialize_text_placements(ptree& node, text_placements_ptr const& p, bool explicit_defaults)
{
    text_symbolizer_properties dfl;
    p->defaults.to_xml(node, explicit_defaults, dfl);
    // Known types:
    //   - text_placements_dummy: no handling required
    //   - text_placements_simple: positions string
    //   - text_placements_list: list string

    text_placements_simple* simple = dynamic_cast<text_placements_simple*>(p.get());
    if (simple)
    {
        set_attr(node, "placement-type", "simple");
        set_attr(node, "placements", simple->get_positions());
    }

    text_placements_list* list = dynamic_cast<text_placements_list*>(p.get());
    if (list)
    {
        set_attr(node, "placement-type", "list");
        // dfl = last properties passed as default so only attributes that change are actually written
        text_symbolizer_properties* dfl2 = &(list->defaults);
        for (unsigned i = 0; i < list->size(); ++i)
        {
            ptree& placement_node = node.push_back(ptree::value_type("Placement", ptree()))->second;
            list->get(i).to_xml(placement_node, explicit_defaults, *dfl2);
            dfl2 = &(list->get(i));
        }
    }
}

void serialize_raster_colorizer(ptree& sym_node, raster_colorizer_ptr const& colorizer, bool explicit_defaults)
{
    ptree& col_node = sym_node.push_back(ptree::value_type("RasterColorizer", ptree()))->second;
    raster_colorizer dfl;
    if (colorizer->get_default_mode() != dfl.get_default_mode() || explicit_defaults)
    {
        set_attr(col_node, "default-mode", colorizer->get_default_mode().as_string());
    }
    if (colorizer->get_default_color() != dfl.get_default_color() || explicit_defaults)
    {
        set_attr(col_node, "default-color", colorizer->get_default_color());
    }
    if (colorizer->get_epsilon() != dfl.get_epsilon() || explicit_defaults)
    {
        set_attr(col_node, "epsilon", colorizer->get_epsilon());
    }

    colorizer_stops const& stops = colorizer->get_stops();
    for (auto const& stop : stops)
    {
        ptree& stop_node = col_node.push_back(ptree::value_type("stop", ptree()))->second;
        set_attr(stop_node, "value", stop.get_value());
        set_attr(stop_node, "color", stop.get_color());
        set_attr(stop_node, "mode", stop.get_mode().as_string());
        if (!stop.get_label().empty())
        {
            set_attr(stop_node, "label", stop.get_label());
        }
    }
}

void serialize_group_symbolizer_properties(ptree& sym_node,
                                           group_symbolizer_properties_ptr const& properties,
                                           bool explicit_defaults);

template<typename Meta>
class serialize_symbolizer_property
{
  public:
    serialize_symbolizer_property(Meta const& meta, boost::property_tree::ptree& node, bool explicit_defaults)
        : meta_(meta)
        , node_(node)
        , explicit_defaults_(explicit_defaults)
    {}

    void operator()(mapnik::enumeration_wrapper const& e) const
    {
        auto const& convert_fun_ptr(std::get<1>(meta_));
        if (convert_fun_ptr)
        {
            node_.put("<xmlattr>." + std::string(std::get<0>(meta_)), convert_fun_ptr(e));
        }
    }

    void operator()(path_expression_ptr const& expr) const
    {
        if (expr)
        {
            node_.put("<xmlattr>." + std::string(std::get<0>(meta_)), path_processor::to_string(*expr));
        }
    }

    void operator()(text_placements_ptr const& expr) const
    {
        if (expr)
        {
            serialize_text_placements(node_, expr, explicit_defaults_);
        }
    }

    void operator()(raster_colorizer_ptr const& expr) const
    {
        if (expr)
        {
            serialize_raster_colorizer(node_, expr, explicit_defaults_);
        }
    }

    void operator()(transform_type const& expr) const
    {
        if (expr)
        {
            node_.put("<xmlattr>." + std::string(std::get<0>(meta_)), transform_processor_type::to_string(*expr));
        }
    }

    void operator()(expression_ptr const& expr) const
    {
        if (expr)
        {
            node_.put("<xmlattr>." + std::string(std::get<0>(meta_)), mapnik::to_expression_string(*expr));
        }
    }

    void operator()(dash_array const& dash) const
    {
        std::ostringstream os;
        for (std::size_t i = 0; i < dash.size(); ++i)
        {
            os << dash[i].first << "," << dash[i].second;
            if (i + 1 < dash.size())
                os << ",";
        }
        node_.put("<xmlattr>." + std::string(std::get<0>(meta_)), os.str());
    }

    void operator()(group_symbolizer_properties_ptr const& properties) const
    {
        if (properties)
        {
            serialize_group_symbolizer_properties(node_, properties, explicit_defaults_);
        }
    }

    template<typename T>
    void operator()(T const& val) const
    {
        node_.put("<xmlattr>." + std::string(std::get<0>(meta_)), val);
    }

  private:
    Meta const& meta_;
    boost::property_tree::ptree& node_;
    bool explicit_defaults_;
};

class serialize_symbolizer
{
  public:
    serialize_symbolizer(ptree& r, bool explicit_defaults)
        : rule_(r)
        , explicit_defaults_(explicit_defaults)
    {}

    template<typename Symbolizer>
    void operator()(Symbolizer const& sym)
    {
        ptree& sym_node = rule_.push_back(ptree::value_type(symbolizer_traits<Symbolizer>::name(), ptree()))->second;
        serialize_symbolizer_properties(sym_node, sym);
    }

  private:

    void serialize_symbolizer_properties(ptree& sym_node, symbolizer_base const& sym)
    {
        for (auto const& prop : sym.properties)
        {
            util::apply_visitor(
              serialize_symbolizer_property<property_meta_type>(get_meta(prop.first), sym_node, explicit_defaults_),
              prop.second);
        }
    }
    ptree& rule_;
    bool explicit_defaults_;
};

class serialize_group_layout
{
  public:
    serialize_group_layout(ptree& parent_node, bool explicit_defaults)
        : parent_node_(parent_node)
        , explicit_defaults_(explicit_defaults)
    {}

    void operator()(simple_row_layout const& layout) const
    {
        ptree& layout_node = parent_node_.push_back(ptree::value_type("SimpleLayout", ptree()))->second;

        simple_row_layout dfl;
        if (explicit_defaults_ || layout.get_item_margin() != dfl.get_item_margin())
        {
            set_attr(layout_node, "item-margin", layout.get_item_margin());
        }
    }

    void operator()(pair_layout const& layout) const
    {
        ptree& layout_node = parent_node_.push_back(ptree::value_type("PairLayout", ptree()))->second;

        pair_layout dfl;
        if (explicit_defaults_ || layout.get_item_margin() != dfl.get_item_margin())
        {
            set_attr(layout_node, "item-margin", layout.get_item_margin());
        }
        if (explicit_defaults_ || layout.get_max_difference() != dfl.get_max_difference())
        {
            set_attr(layout_node, "max-difference", layout.get_max_difference());
        }
    }

    template<typename T>
    void operator()(T const&) const
    {}

  private:
    ptree& parent_node_;
    bool explicit_defaults_;
};

void serialize_group_rule(ptree& parent_node, const group_rule& r, bool explicit_defaults)
{
    ptree& rule_node = parent_node.push_back(ptree::value_type("GroupRule", ptree()))->second;

    group_rule dfl;
    std::string filter = mapnik::to_expression_string(*r.get_filter());
    std::string default_filter = mapnik::to_expression_string(*dfl.get_filter());

    if (filter != default_filter)
    {
        rule_node.push_back(ptree::value_type("Filter", ptree()))->second.put_value(filter);
    }

    if (r.get_repeat_key())
    {
        std::string repeat_key = mapnik::to_expression_string(*r.get_repeat_key());
        rule_node.push_back(ptree::value_type("RepeatKey", ptree()))->second.put_value(repeat_key);
    }

    rule::symbolizers::const_iterator begin = r.get_symbolizers().begin();
    rule::symbolizers::const_iterator end = r.get_symbolizers().end();
    serialize_symbolizer serializer(rule_node, explicit_defaults);
    std::for_each(begin, end, [&serializer](symbolizer const& sym) { util::apply_visitor(std::ref(serializer), sym); });
}

void serialize_group_symbolizer_properties(ptree& sym_node,
                                           group_symbolizer_properties_ptr const& properties,
                                           bool explicit_defaults)
{
    util::apply_visitor(serialize_group_layout(sym_node, explicit_defaults), properties->get_layout());

    for (auto const& rule : properties->get_rules())
    {
        serialize_group_rule(sym_node, *rule, explicit_defaults);
    }
}

void serialize_rule(ptree& style_node, rule const& r, bool explicit_defaults)
{
    ptree& rule_node = style_node.push_back(ptree::value_type("Rule", ptree()))->second;

    rule dfl;
    if (r.get_name() != dfl.get_name())
    {
        set_attr(rule_node, "name", r.get_name());
    }

    if (r.has_else_filter())
    {
        rule_node.push_back(ptree::value_type("ElseFilter", ptree()));
    }
    else if (r.has_also_filter())
    {
        rule_node.push_back(ptree::value_type("AlsoFilter", ptree()));
    }
    else
    {
        // filters were not comparable, perhaps should now compare expressions?
        expression_ptr const& expr = r.get_filter();
        std::string filter = mapnik::to_expression_string(*expr);
        std::string default_filter = mapnik::to_expression_string(*dfl.get_filter());

        if (filter != default_filter)
        {
            rule_node.push_back(ptree::value_type("Filter", ptree()))->second.put_value(filter);
        }
    }

    if (r.get_min_scale() != dfl.get_min_scale())
    {
        ptree& min_scale = rule_node.push_back(ptree::value_type("MinScaleDenominator", ptree()))->second;
        min_scale.put_value(r.get_min_scale());
    }

    if (r.get_max_scale() != dfl.get_max_scale())
    {
        ptree& max_scale = rule_node.push_back(ptree::value_type("MaxScaleDenominator", ptree()))->second;
        max_scale.put_value(r.get_max_scale());
    }

    rule::symbolizers::const_iterator begin = r.get_symbolizers().begin();
    rule::symbolizers::const_iterator end = r.get_symbolizers().end();
    serialize_symbolizer serializer(rule_node, explicit_defaults);
    std::for_each(begin, end, [&serializer](symbolizer const& sym) { util::apply_visitor(std::ref(serializer), sym); });
}

void serialize_style(ptree& map_node, std::string const& name, feature_type_style const& style, bool explicit_defaults)
{
    ptree& style_node = map_node.push_back(ptree::value_type("Style", ptree()))->second;

    set_attr(style_node, "name", name);

    feature_type_style dfl;
    filter_mode_e filter_mode = style.get_filter_mode();
    if (filter_mode != dfl.get_filter_mode() || explicit_defaults)
    {
        set_attr(style_node, "filter-mode", filter_mode.as_string());
    }

    double opacity = style.get_opacity();
    if (opacity != dfl.get_opacity() || explicit_defaults)
    {
        set_attr(style_node, "opacity", opacity);
    }

    bool image_filters_inflate = style.image_filters_inflate();
    if (image_filters_inflate != dfl.image_filters_inflate() || explicit_defaults)
    {
        set_attr(style_node, "image-filters-inflate", image_filters_inflate);
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

    for (auto const& r : style.get_rules())
    {
        serialize_rule(style_node, r, explicit_defaults);
    }
}

void serialize_fontset(ptree& map_node, std::string const& name, font_set const& fontset)
{
    ptree& fontset_node = map_node.push_back(ptree::value_type("FontSet", ptree()))->second;

    set_attr(fontset_node, "name", name);

    for (auto const& face_name : fontset.get_face_names())
    {
        ptree& font_node = fontset_node.push_back(ptree::value_type("Font", ptree()))->second;
        set_attr(font_node, "face-name", face_name);
    }
}

void serialize_datasource(ptree& layer_node, datasource_ptr datasource)
{
    ptree& datasource_node = layer_node.push_back(ptree::value_type("Datasource", ptree()))->second;

    for (auto const& p : datasource->params())
    {
        boost::property_tree::ptree& param_node =
          datasource_node
            .push_back(boost::property_tree::ptree::value_type("Parameter", boost::property_tree::ptree()))
            ->second;
        param_node.put("<xmlattr>.name", p.first);
        param_node.put_value(p.second);
    }
}

void serialize_parameters(ptree& map_node, mapnik::parameters const& params)
{
    if (params.size())
    {
        ptree& params_node = map_node.push_back(ptree::value_type("Parameters", ptree()))->second;

        for (auto const& p : params)
        {
            boost::property_tree::ptree& param_node =
              params_node
                .push_back(boost::property_tree::ptree::value_type("Parameter", boost::property_tree::ptree()))
                ->second;
            param_node.put("<xmlattr>.name", p.first);
            param_node.put_value(p.second);
        }
    }
}

void serialize_layer(ptree& map_node, layer const& lyr, bool explicit_defaults)
{
    ptree& layer_node = map_node.push_back(ptree::value_type("Layer", ptree()))->second;

    if (lyr.name() != "")
    {
        set_attr(layer_node, "name", lyr.name());
    }

    auto const comp_op = lyr.comp_op();

    if (comp_op)
    {
        set_attr(layer_node, "comp-op", *comp_op_to_string(*comp_op));
    }
    else if (explicit_defaults)
    {
        set_attr(layer_node, "comp-op", "src-over");
    }

    if (lyr.srs() != "")
    {
        set_attr(layer_node, "srs", lyr.srs());
    }

    if (!lyr.active() || explicit_defaults)
    {
        set_attr /*<bool>*/ (layer_node, "status", lyr.active());
    }

    if (lyr.clear_label_cache() || explicit_defaults)
    {
        set_attr /*<bool>*/ (layer_node, "clear-label-cache", lyr.clear_label_cache());
    }

    if (lyr.minimum_scale_denominator() != 0 || explicit_defaults)
    {
        set_attr(layer_node, "minimum-scale-denominator", lyr.minimum_scale_denominator());
    }

    if (lyr.maximum_scale_denominator() != std::numeric_limits<double>::max() || explicit_defaults)
    {
        set_attr(layer_node, "maximum-scale-denominator", lyr.maximum_scale_denominator());
    }

    if (lyr.queryable() || explicit_defaults)
    {
        set_attr(layer_node, "queryable", lyr.queryable());
    }

    if (lyr.cache_features() || explicit_defaults)
    {
        set_attr /*<bool>*/ (layer_node, "cache-features", lyr.cache_features());
    }

    if (lyr.group_by() != "" || explicit_defaults)
    {
        set_attr(layer_node, "group-by", lyr.group_by());
    }

    boost::optional<int> const& buffer_size = lyr.buffer_size();
    if (buffer_size || explicit_defaults)
    {
        set_attr(layer_node, "buffer-size", *buffer_size);
    }

    optional<box2d<double>> const& maximum_extent = lyr.maximum_extent();
    if (maximum_extent)
    {
        std::ostringstream s;
        s << std::setprecision(16) << maximum_extent->minx() << "," << maximum_extent->miny() << ","
          << maximum_extent->maxx() << "," << maximum_extent->maxy();
        set_attr(layer_node, "maximum-extent", s.str());
    }

    for (auto const& name : lyr.styles())
    {
        boost::property_tree::ptree& style_node =
          layer_node.push_back(boost::property_tree::ptree::value_type("StyleName", boost::property_tree::ptree()))
            ->second;
        style_node.put_value(name);
    }

    datasource_ptr datasource = lyr.datasource();
    if (datasource)
    {
        serialize_datasource(layer_node, datasource);
    }

    // serialize nested layers
    for (auto const& child : lyr.layers())
    {
        serialize_layer(layer_node, child, explicit_defaults);
    }
}

void serialize_map(ptree& pt, Map const& map, bool explicit_defaults)
{
    ptree& map_node = pt.push_back(ptree::value_type("Map", ptree()))->second;

    set_attr(map_node, "srs", map.srs());

    optional<color> const& c = map.background();
    if (c)
    {
        set_attr(map_node, "background-color", *c);
    }

    optional<std::string> const& font_directory = map.font_directory();
    if (font_directory)
    {
        set_attr(map_node, "font-directory", *font_directory);
    }

    optional<std::string> const& image_filename = map.background_image();
    if (image_filename)
    {
        set_attr(map_node, "background-image", *image_filename);
    }

    composite_mode_e comp_op = map.background_image_comp_op();
    if (comp_op != src_over || explicit_defaults)
    {
        set_attr(map_node, "background-image-comp-op", *comp_op_to_string(comp_op));
    }

    double opacity = map.background_image_opacity();
    if (opacity != 1.0 || explicit_defaults)
    {
        set_attr(map_node, "background-image-opacity", opacity);
    }

    int buffer_size = map.buffer_size();
    if (buffer_size || explicit_defaults)
    {
        set_attr(map_node, "buffer-size", buffer_size);
    }

    std::string const& base_path = map.base_path();
    if (!base_path.empty() || explicit_defaults)
    {
        set_attr(map_node, "base", base_path);
    }

    optional<box2d<double>> const& maximum_extent = map.maximum_extent();
    if (maximum_extent)
    {
        std::ostringstream s;
        s << std::setprecision(16) << maximum_extent->minx() << "," << maximum_extent->miny() << ","
          << maximum_extent->maxx() << "," << maximum_extent->maxy();
        set_attr(map_node, "maximum-extent", s.str());
    }

    for (auto const& kv : map.fontsets())
    {
        serialize_fontset(map_node, kv.first, kv.second);
    }

    serialize_parameters(map_node, map.get_extra_parameters());

    for (auto const& kv : map.styles())
    {
        serialize_style(map_node, kv.first, kv.second, explicit_defaults);
    }

    for (auto const& layer : map.layers())
    {
        serialize_layer(map_node, layer, explicit_defaults);
    }
}

void save_map(Map const& map, std::string const& filename, bool explicit_defaults)
{
    ptree pt;
    serialize_map(pt, map, explicit_defaults);
#if BOOST_VERSION >= 105600
    write_xml(filename, pt, std::locale(), boost::property_tree::xml_writer_make_settings<ptree::key_type>(' ', 2));
#else
    write_xml(filename, pt, std::locale(), boost::property_tree::xml_writer_make_settings(' ', 2));
#endif
}

std::string save_map_to_string(Map const& map, bool explicit_defaults)
{
    ptree pt;
    serialize_map(pt, map, explicit_defaults);
    std::ostringstream ss;
#if BOOST_VERSION >= 105600
    write_xml(ss, pt, boost::property_tree::xml_writer_make_settings<ptree::key_type>(' ', 2));
#else
    write_xml(ss, pt, boost::property_tree::xml_writer_make_settings(' ', 2));
#endif
    return ss.str();
}

} // namespace mapnik
