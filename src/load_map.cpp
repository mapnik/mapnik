/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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
#include <mapnik/debug.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/xml_tree.hpp>
#include <mapnik/version.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/color.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/symbolizer_utils.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/xml_loader.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/transform/parse_transform.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/svg/svg_path_parser.hpp>
#include <mapnik/text/placements/registry.hpp>
#include <mapnik/text/placements/dummy.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/util/dasharray_parser.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/util/name_to_int.hpp>
#include <mapnik/image_filter_types.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/group/group_rule.hpp>
#include <mapnik/transform/transform_expression.hpp>
#include <mapnik/evaluate_global_attributes.hpp>
#include <mapnik/boolean.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/tokenizer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/static_assert.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <algorithm>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_trans_affine.h"
MAPNIK_DISABLE_WARNING_POP

using boost::tokenizer;

namespace mapnik {
using std::optional;
using util::name_to_int;
using util::operator"" _case;

class map_parser : util::noncopyable
{
  public:
    map_parser(Map& map, bool strict, std::string const& filename = "")
        : strict_(strict)
        , filename_(filename)
        , font_library_()
        , font_file_mapping_(map.get_font_file_mapping())
        , font_name_cache_()
        , file_sources_()
        , fontsets_()
        , xml_base_path_()
    {}

    void parse_map(Map& map, xml_node const& node, std::string const& base_path);

  private:
    void parse_map_include(Map& map, xml_node const& node);
    void parse_style(Map& map, xml_node const& node);

    template<typename Parent>
    void parse_layer(Parent& parent, xml_node const& node);

    void parse_symbolizer_base(symbolizer_base& sym, xml_node const& node);
    void parse_fontset(Map& map, xml_node const& node);
    bool parse_font(font_set& fset, xml_node const& f);
    void parse_rule(feature_type_style& style, xml_node const& node);
    void parse_symbolizers(rule& rule, xml_node const& node);
    void parse_point_symbolizer(rule& rule, xml_node const& node);
    void parse_line_pattern_symbolizer(rule& rule, xml_node const& node);
    void parse_polygon_pattern_symbolizer(rule& rule, xml_node const& node);
    void parse_text_symbolizer(rule& rule, xml_node const& node);
    void parse_shield_symbolizer(rule& rule, xml_node const& node);
    void parse_line_symbolizer(rule& rule, xml_node const& node);
    void parse_polygon_symbolizer(rule& rule, xml_node const& node);
    void parse_building_symbolizer(rule& rule, xml_node const& node);
    void parse_raster_symbolizer(rule& rule, xml_node const& node);
    void parse_markers_symbolizer(rule& rule, xml_node const& node);
    void parse_group_symbolizer(rule& rule, xml_node const& node);
    void parse_debug_symbolizer(rule& rule, xml_node const& node);
    void parse_dot_symbolizer(rule& rule, xml_node const& node);
    void parse_group_rule(group_symbolizer_properties& prop, xml_node const& node);
    void parse_simple_layout(group_symbolizer_properties& prop, xml_node const& node);
    void parse_pair_layout(group_symbolizer_properties& prop, xml_node const& node);
    bool parse_raster_colorizer(raster_colorizer_ptr const& rc, xml_node const& node);
    void parse_stroke(symbolizer_base& symbol, xml_node const& node);
    void ensure_font_face(std::string const& face_name);
    void find_unused_nodes(xml_node const& root);
    void find_unused_nodes_recursive(xml_node const& node, std::string& error_text);
    std::string ensure_relative_to_xml(std::optional<std::string> const& opt_path);
    void ensure_exists(std::string const& file_path);
    void check_styles(Map const& map);
    std::optional<color> get_opt_color_attr(boost::property_tree::ptree const& node, std::string const& name);

    bool strict_;
    std::string filename_;
    std::map<std::string, parameters> datasource_templates_;
    font_library font_library_;
    freetype_engine::font_file_mapping_type& font_file_mapping_;
    std::map<std::string, bool> font_name_cache_;
    std::map<std::string, std::string> file_sources_;
    std::map<std::string, font_set> fontsets_;
    std::string xml_base_path_;
};

struct allow_overlap_visitor
{
    bool operator()(point_symbolizer const&) { return true; }
    bool operator()(line_symbolizer const&) { return false; }
    bool operator()(line_pattern_symbolizer const&) { return false; }
    bool operator()(polygon_symbolizer const&) { return false; }
    bool operator()(polygon_pattern_symbolizer const&) { return false; }
    bool operator()(raster_symbolizer const&) { return false; }
    bool operator()(shield_symbolizer const&) { return false; }
    bool operator()(text_symbolizer const&) { return true; }
    bool operator()(building_symbolizer const&) { return false; }
    bool operator()(markers_symbolizer const&) { return true; }
    bool operator()(group_symbolizer const&) { return false; }
    bool operator()(debug_symbolizer const&) { return true; } // Requires the quadtree
    bool operator()(dot_symbolizer const&) { return false; }
};

// If all symbolizers declare 'allow_overlap: true' (their placement is independent
// from already placed symbols), there is no need to check collisions with subsequent
// symbolizers. To avoid building the quadtree unnecessarily we set
// the 'ignore_placement' flag
void map_apply_overlap_optimization(Map& map)
{
    bool ignore_placement = true;
    for (auto const& style : map.styles())
    {
        for (auto const& rule : style.second.get_rules())
        {
            for (auto const& sym : rule)
            {
                struct allow_overlap_visitor visitor;
                if (util::apply_visitor(visitor, sym))
                {
                    symbolizer_base const& sb = sym.get_unchecked<markers_symbolizer>();
                    auto prop_it = sb.properties.find(keys::allow_overlap);
                    if (prop_it != sb.properties.end())
                    {
                        if (prop_it->second == true)
                        {
                            continue;
                        }
                    }
                    ignore_placement = false;
                    break;
                }
            }
        }
    }

    if (ignore_placement)
    {
        for (auto& style : map.styles())
        {
            for (auto& rule : style.second.get_rules_nonconst())
            {
                for (auto& sym : rule)
                {
                    struct allow_overlap_visitor visitor;
                    if (util::apply_visitor(visitor, sym))
                    {
                        symbolizer_base& sb = sym.get_unchecked<markers_symbolizer>();
                        sb.properties[keys::ignore_placement] = true;
                    }
                }
            }
        }
        MAPNIK_LOG_DEBUG(load_map) << "setting ignore_placement=true due to all rules having allow_overlap=true";
    }
}

void load_map(Map& map, std::string const& filename, bool strict, std::string base_path)
{
    xml_tree tree;
    tree.set_filename(filename);
    read_xml(filename, tree.root());
    map_parser parser(map, strict, filename);
    parser.parse_map(map, tree.root(), base_path);
    map_apply_overlap_optimization(map);
}

void load_map_string(Map& map, std::string const& str, bool strict, std::string base_path)
{
    xml_tree tree;
    if (!base_path.empty())
    {
        read_xml_string(str, tree.root(), base_path); // accept base_path passed into function
    }
    else
    {
        read_xml_string(str, tree.root(), map.base_path()); // FIXME - this value is not fully known yet
    }
    map_parser parser(map, strict, base_path);
    parser.parse_map(map, tree.root(), base_path);
    map_apply_overlap_optimization(map);
}

void map_parser::parse_map(Map& map, xml_node const& node, std::string const& base_path)
{
    try
    {
        xml_node const& map_node = node.get_child("Map");
        try
        {
            optional<std::string> base_path_from_xml = map_node.get_opt_attr<std::string>("base");
            if (!base_path.empty())
            {
                map.set_base_path(base_path);
            }
            else if (base_path_from_xml)
            {
                map.set_base_path(*base_path_from_xml);
            }
            else if (!filename_.empty())
            {
                map.set_base_path(mapnik::util::dirname(filename_));
            }
            xml_base_path_ = map.base_path();

            optional<color> bgcolor = map_node.get_opt_attr<color>("background-color");
            if (bgcolor)
            {
                map.set_background(*bgcolor);
            }

            optional<std::string> image_filename = map_node.get_opt_attr<std::string>("background-image");
            if (image_filename)
            {
                map.set_background_image(ensure_relative_to_xml(image_filename));
            }

            optional<std::string> comp_op_name = map_node.get_opt_attr<std::string>("background-image-comp-op");
            if (comp_op_name)
            {
                optional<composite_mode_e> comp_op = comp_op_from_string(*comp_op_name);
                if (comp_op)
                {
                    map.set_background_image_comp_op(*comp_op);
                }
                else
                {
                    throw config_error("failed to parse background-image-comp-op: '" + *comp_op_name + "'");
                }
            }

            optional<double> opacity = map_node.get_opt_attr<double>("background-image-opacity");
            if (opacity)
            {
                map.set_background_image_opacity(*opacity);
            }

            std::string srs = map_node.get_attr("srs", map.srs());
            try
            {
                // create throwaway projection object here to ensure it is valid
                projection proj(srs, true);
            }
            catch (std::exception const& ex)
            {
                throw mapnik::config_error(ex.what());
            }
            map.set_srs(srs);

            optional<int> buffer_size = map_node.get_opt_attr<int>("buffer-size");
            if (buffer_size)
            {
                map.set_buffer_size(*buffer_size);
            }

            optional<std::string> maximum_extent = map_node.get_opt_attr<std::string>("maximum-extent");
            if (maximum_extent)
            {
                box2d<double> box;
                if (box.from_string(*maximum_extent))
                {
                    map.set_maximum_extent(box);
                }
                else
                {
                    std::string s_err("failed to parse Map maximum-extent '");
                    s_err += *maximum_extent + "'";
                    if (strict_)
                    {
                        throw config_error(s_err);
                    }
                    else
                    {
                        MAPNIK_LOG_ERROR(load_map) << "map_parser: " << s_err;
                    }
                }
            }

            optional<std::string> font_directory = map_node.get_opt_attr<std::string>("font-directory");
            if (font_directory)
            {
                map.set_font_directory(*font_directory);
                if (!map.register_fonts(ensure_relative_to_xml(font_directory), false))
                {
                    if (strict_)
                    {
                        throw config_error(std::string("Failed to load fonts from: ") + *font_directory);
                    }
                }
            }

            optional<std::string> min_version_string = map_node.get_opt_attr<std::string>("minimum-version");

            if (min_version_string)
            {
                boost::char_separator<char> sep(".");
                boost::tokenizer<boost::char_separator<char>> tokens(*min_version_string, sep);
                unsigned i = 0;
                bool success = false;
                int n[3];
                for (auto const& beg : tokens)
                {
                    std::string item = mapnik::util::trim_copy(beg);
                    if (!mapnik::util::string2int(item, n[i]))
                    {
                        throw config_error(std::string("Invalid version string encountered: '") + beg + "' in '" +
                                           *min_version_string + "'");
                    }
                    if (i == 2)
                    {
                        success = true;
                        break;
                    }
                    ++i;
                }
                if (success)
                {
                    if (!MAPNIK_VERSION_AT_LEAST(n[0], n[1], n[2]))
                    {
                        throw config_error(std::string("This map uses features only present in Mapnik version ") +
                                           *min_version_string + " and newer");
                    }
                }
            }
        }
        catch (config_error const& ex)
        {
            ex.append_context(map_node);
            throw;
        }

        parse_map_include(map, map_node);
    }
    catch (node_not_found const&)
    {
        throw config_error("Not a map file. Node 'Map' not found.");
    }
    find_unused_nodes(node);
    if (strict_)
    {
        check_styles(map);
    }
}

void map_parser::parse_map_include(Map& map, xml_node const& node)
{
    try
    {
        for (auto const& n : node)
        {
            if (n.is_text())
                continue;
            if (n.is("Include"))
            {
                parse_map_include(map, n);
            }
            else if (n.is("Style"))
            {
                parse_style(map, n);
            }
            else if (n.is("Layer"))
            {
                parse_layer(map, n);
            }
            else if (n.is("FontSet"))
            {
                parse_fontset(map, n);
            }
            else if (n.is("FileSource"))
            {
                file_sources_[n.get_attr<std::string>("name")] = n.get_text();
            }
            else if (n.is("Datasource"))
            {
                std::string name = n.get_attr("name", std::string("Unnamed"));
                parameters params;
                for (auto const& p : n)
                {
                    if (p.is("Parameter"))
                    {
                        params[p.get_attr<std::string>("name")] = p.get_text();
                    }
                }
                datasource_templates_[std::move(name)] = std::move(params);
            }
            else if (n.is("Parameters"))
            {
                parameters& params = map.get_extra_parameters();
                for (auto const& p : n)
                {
                    if (p.is("Parameter"))
                    {
                        std::string val = p.get_text();
                        std::string key = p.get_attr<std::string>("name");
                        mapnik::value_bool b;
                        mapnik::value_integer i;
                        mapnik::value_double d;
                        if (mapnik::util::string2int(val, i))
                            params[key] = i;
                        else if (mapnik::util::string2bool(val, b))
                            params[key] = b;
                        else if (mapnik::util::string2double(val, d))
                            params[key] = d;
                        else
                            params[key] = val;
                    }
                }
            }
        }
    }
    catch (config_error const& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_style(Map& map, xml_node const& node)
{
    std::string name("<missing name>");
    try
    {
        name = node.get_attr<std::string>("name");
        feature_type_style style;

        filter_mode_e filter_mode = node.get_attr<filter_mode_e>("filter-mode", filter_mode_enum::FILTER_ALL);
        style.set_filter_mode(filter_mode);

        // compositing
        optional<std::string> comp_op_name = node.get_opt_attr<std::string>("comp-op");
        if (comp_op_name)
        {
            optional<composite_mode_e> comp_op = comp_op_from_string(*comp_op_name);
            if (comp_op)
            {
                style.set_comp_op(*comp_op);
            }
            else
            {
                throw config_error("failed to parse comp-op: '" + *comp_op_name + "'");
            }
        }

        optional<double> opacity = node.get_opt_attr<double>("opacity");
        if (opacity)
            style.set_opacity(*opacity);

        optional<mapnik::boolean_type> image_filters_inflate =
          node.get_opt_attr<mapnik::boolean_type>("image-filters-inflate");
        if (image_filters_inflate)
        {
            style.set_image_filters_inflate(*image_filters_inflate);
        }

        // image filters
        optional<std::string> filters = node.get_opt_attr<std::string>("image-filters");
        if (filters)
        {
            if (!parse_image_filters(*filters, style.image_filters()))
            {
                throw config_error("failed to parse image-filters: '" + *filters + "'");
            }
        }

        // direct image filters (applied directly on main image buffer
        // TODO : consider creating a separate XML node e.g
        // <ImageFilter name="myfilter" op="blur emboss"/>
        //
        optional<std::string> direct_filters = node.get_opt_attr<std::string>("direct-image-filters");
        if (direct_filters)
        {
            if (!parse_image_filters(*direct_filters, style.direct_image_filters()))
            {
                throw config_error("failed to parse direct-image-filters: '" + *direct_filters + "'");
            }
        }

        style.reserve(node.size());
        // rules
        for (auto const& rule_ : node)
        {
            if (rule_.is("Rule"))
            {
                parse_rule(style, rule_);
            }
        }

        if (!map.insert_style(name, std::move(style)))
        {
            boost::optional<const feature_type_style&> dupe = map.find_style(name);
            if (strict_)
            {
                if (dupe)
                {
                    throw config_error("duplicate style name");
                }
                throw config_error("failed to insert style to the map");
            }
            else
            {
                std::string s_err("failed to insert style '");
                s_err += name + "' to the map";
                if (dupe)
                {
                    s_err += " since it was already added";
                }
                MAPNIK_LOG_ERROR(load_map) << "map_parser: " << s_err;
            }
        }
    }
    catch (config_error const& ex)
    {
        ex.append_context(std::string("in style '") + name + "'", node);
        throw;
    }
}

void map_parser::parse_fontset(Map& map, xml_node const& node)
{
    std::string name("<missing name>");
    try
    {
        name = node.get_attr<std::string>("name");
        font_set fontset(name);
        bool success = false;
        for (auto const& n : node)
        {
            if (n.is("Font"))
            {
                if (parse_font(fontset, n))
                {
                    success = true;
                }
                else
                {
                    MAPNIK_LOG_WARN(fontset)
                      << "warning: unable to find face-name '" << n.get_attr<std::string>("face-name", "")
                      << "' in FontSet '" << fontset.get_name() << "'";
                }
            }
        }

        // if not at least one face-name is valid
        if (!success)
        {
            throw mapnik::config_error("no valid fonts could be loaded");
        }

        // XXX Hack because map object isn't accessible by text_symbolizer
        // when it's parsed
        fontsets_.emplace(name, fontset);
        map.insert_fontset(name, std::move(fontset));
    }
    catch (config_error const& ex)
    {
        ex.append_context(std::string("in FontSet '") + name + "'", node);
        throw;
    }
}

bool map_parser::parse_font(font_set& fset, xml_node const& f)
{
    optional<std::string> has_face_name = f.get_opt_attr<std::string>("face-name");
    if (has_face_name)
    {
        std::string face_name = *has_face_name;
        bool found = false;
        auto itr = font_name_cache_.find(face_name);
        if (itr != font_name_cache_.end())
        {
            found = itr->second;
        }
        else
        {
            found =
              freetype_engine::can_open(face_name, font_library_, font_file_mapping_, freetype_engine::get_mapping());
            font_name_cache_.emplace(face_name, found);
        }
        if (found)
        {
            fset.add_face_name(face_name);
            return true;
        }
        else if (strict_)
        {
            throw config_error("Failed to find font face '" + face_name + "'");
        }
    }
    else
    {
        throw config_error("Must have 'face-name' set", f);
    }
    return false;
}

template<typename Parent>
void map_parser::parse_layer(Parent& parent, xml_node const& node)
{
    std::string name;
    try
    {
        optional<mapnik::boolean_type> status = node.get_opt_attr<mapnik::boolean_type>("status");

        // return early is status is off
        if (status && !(*status))
        {
            node.set_ignore(true);
            return;
        }

        name = node.get_attr("name", std::string("Unnamed"));

        // If no projection is given inherit from map
        std::string srs = node.get_attr("srs", parent.srs());
        try
        {
            // create throwaway projection object here to ensure it is valid
            projection proj(srs, true);
        }
        catch (std::exception const& ex)
        {
            throw mapnik::config_error(ex.what());
        }
        layer lyr(name, srs);

        if (status)
        {
            lyr.set_active(*status);
        }

        optional<double> minimum_scale_denom = node.get_opt_attr<double>("minimum-scale-denominator");
        if (minimum_scale_denom)
        {
            lyr.set_minimum_scale_denominator(*minimum_scale_denom);
        }
        else // back compatibility: remove at Mapnik 4.x
        {
            optional<double> min_zoom = node.get_opt_attr<double>("minzoom");
            if (min_zoom)
            {
                MAPNIK_LOG_ERROR(markers_symbolizer) << "'minzoom' is deprecated and will be removed in Mapnik 4.x, "
                                                        "use 'minimum-scale-denominator' instead (encountered in layer "
                                                     << name << ")";
                lyr.set_minimum_scale_denominator(*min_zoom);
            }
        }

        optional<double> maximum_scale_denom = node.get_opt_attr<double>("maximum-scale-denominator");
        if (maximum_scale_denom)
        {
            lyr.set_maximum_scale_denominator(*maximum_scale_denom);
        }
        else // back compatibility: remove at Mapnik 4.x
        {
            optional<double> max_zoom = node.get_opt_attr<double>("maxzoom");
            if (max_zoom)
            {
                MAPNIK_LOG_ERROR(markers_symbolizer) << "'maxzoom' is deprecated and will be removed in Mapnik 4.x, "
                                                        "use 'maximum-scale-denominator' instead (encountered in layer "
                                                     << name << ")";
                lyr.set_maximum_scale_denominator(*max_zoom);
            }
        }

        optional<mapnik::boolean_type> queryable = node.get_opt_attr<mapnik::boolean_type>("queryable");
        if (queryable)
        {
            lyr.set_queryable(*queryable);
        }

        optional<mapnik::boolean_type> clear_cache = node.get_opt_attr<mapnik::boolean_type>("clear-label-cache");
        if (clear_cache)
        {
            lyr.set_clear_label_cache(*clear_cache);
        }

        optional<mapnik::boolean_type> cache_features = node.get_opt_attr<mapnik::boolean_type>("cache-features");
        if (cache_features)
        {
            lyr.set_cache_features(*cache_features);
        }

        optional<std::string> group_by = node.get_opt_attr<std::string>("group-by");
        if (group_by)
        {
            lyr.set_group_by(*group_by);
        }

        optional<int> buffer_size = node.get_opt_attr<int>("buffer-size");
        if (buffer_size)
        {
            lyr.set_buffer_size(*buffer_size);
        }

        optional<std::string> maximum_extent = node.get_opt_attr<std::string>("maximum-extent");
        if (maximum_extent)
        {
            box2d<double> box;
            if (box.from_string(*maximum_extent))
            {
                lyr.set_maximum_extent(box);
            }
            else
            {
                std::string s_err("failed to parse Layer maximum-extent '");
                s_err += *maximum_extent + "' for '" + name + "'";
                if (strict_)
                {
                    throw config_error(s_err);
                }
                else
                {
                    MAPNIK_LOG_ERROR(load_map) << "map_parser: " << s_err;
                }
            }
        }

        // compositing
        optional<std::string> comp_op_name = node.get_opt_attr<std::string>("comp-op");
        if (comp_op_name)
        {
            optional<composite_mode_e> comp_op = comp_op_from_string(*comp_op_name);
            if (comp_op)
            {
                lyr.set_comp_op(*comp_op);
            }
            else
            {
                throw config_error("failed to parse comp-op: '" + *comp_op_name + "'");
            }
        }

        optional<double> opacity = node.get_opt_attr<double>("opacity");
        if (opacity)
            lyr.set_opacity(*opacity);

        for (auto const& child : node)
        {
            if (child.is("StyleName"))
            {
                std::string const& style_name = child.get_text();
                if (style_name.empty())
                {
                    std::string ss("StyleName is empty in Layer: '");
                    ss += lyr.name() + "'";
                    if (strict_)
                    {
                        throw config_error(ss);
                    }
                    else
                    {
                        MAPNIK_LOG_WARN(load_map) << "map_parser: " << ss;
                    }
                }
                else
                {
                    lyr.add_style(style_name);
                }
            }
            else if (child.is("Datasource"))
            {
                parameters params;
                optional<std::string> base = child.get_opt_attr<std::string>("base");
                if (base)
                {
                    std::map<std::string, parameters>::const_iterator base_itr = datasource_templates_.find(*base);
                    if (base_itr != datasource_templates_.end())
                    {
                        params = base_itr->second;
                    }
                    else
                    {
                        MAPNIK_LOG_ERROR(datasource)
                          << "Datasource template '" << *base << "' not found for layer '" << name << "'";
                    }
                }

                for (auto const& n : child)
                {
                    if (n.is("Parameter"))
                    {
                        params[n.get_attr<std::string>("name")] = n.get_text();
                    }
                }

                const auto base_param = params.get<std::string>("base");
                const auto file_param = params.get<std::string>("file");

                if (base_param.has_value())
                {
                    params["base"] = ensure_relative_to_xml(base_param);
                }

                else if (file_param.has_value())
                {
                    params["file"] = ensure_relative_to_xml(file_param);
                }

                // now we are ready to create datasource
                try
                {
                    std::shared_ptr<datasource> ds = datasource_cache::instance().create(params);
                    lyr.set_datasource(ds);
                }
                catch (std::exception const& ex)
                {
                    throw config_error(ex.what());
                }
                catch (...)
                {
                    throw config_error("Unknown exception occurred attempting to create datasoure for layer '" +
                                       lyr.name() + "'");
                }
            }
            else if (child.is("Layer"))
            {
                parse_layer(lyr, child);
            }
        }
        parent.add_layer(std::move(lyr));
    }
    catch (config_error const& ex)
    {
        if (!name.empty())
        {
            ex.append_context(std::string(" encountered during parsing of layer '") + name + "'", node);
        }
        throw;
    }
}

void map_parser::parse_rule(feature_type_style& style, xml_node const& node)
{
    std::string name;
    try
    {
        name = node.get_attr("name", std::string());
        rule rule(name);

        xml_node const* child = node.get_opt_child("Filter");
        if (child)
        {
            rule.set_filter(child->get_value<expression_ptr>());
        }

        if (node.has_child("ElseFilter"))
        {
            rule.set_else(true);
        }

        if (node.has_child("AlsoFilter"))
        {
            rule.set_also(true);
        }

        child = node.get_opt_child("MinScaleDenominator");
        if (child)
        {
            rule.set_min_scale(child->get_value<double>());
        }

        child = node.get_opt_child("MaxScaleDenominator");
        if (child)
        {
            rule.set_max_scale(child->get_value<double>());
        }

        parse_symbolizers(rule, node);
        style.add_rule(std::move(rule));
    }
    catch (config_error const& ex)
    {
        if (!name.empty())
        {
            ex.append_context(std::string("in rule '") + name + "'", node);
        }
        throw;
    }
}

void map_parser::parse_symbolizers(rule& rule, xml_node const& node)
{
    rule.reserve(node.size());
    for (auto const& sym_node : node)
    {
        switch (name_to_int(sym_node.name().c_str()))
        {
            case "PointSymbolizer"_case:
                parse_point_symbolizer(rule, sym_node);
                sym_node.set_processed(true);
                break;
            case "LinePatternSymbolizer"_case:
                parse_line_pattern_symbolizer(rule, sym_node);
                sym_node.set_processed(true);
                break;
            case "PolygonPatternSymbolizer"_case:
                parse_polygon_pattern_symbolizer(rule, sym_node);
                sym_node.set_processed(true);
                break;
            case "TextSymbolizer"_case:
                parse_text_symbolizer(rule, sym_node);
                sym_node.set_processed(true);
                break;
            case "ShieldSymbolizer"_case:
                parse_shield_symbolizer(rule, sym_node);
                sym_node.set_processed(true);
                break;
            case "LineSymbolizer"_case:
                parse_line_symbolizer(rule, sym_node);
                sym_node.set_processed(true);
                break;
            case "PolygonSymbolizer"_case:
                parse_polygon_symbolizer(rule, sym_node);
                sym_node.set_processed(true);
                break;
            case "BuildingSymbolizer"_case:
                parse_building_symbolizer(rule, sym_node);
                sym_node.set_processed(true);
                break;
            case "RasterSymbolizer"_case:
                parse_raster_symbolizer(rule, sym_node);
                sym_node.set_processed(true);
                break;
            case "MarkersSymbolizer"_case:
                parse_markers_symbolizer(rule, sym_node);
                sym_node.set_processed(true);
                break;
            case "GroupSymbolizer"_case:
                parse_group_symbolizer(rule, sym_node);
                sym_node.set_processed(true);
                break;
            case "DebugSymbolizer"_case:
                parse_debug_symbolizer(rule, sym_node);
                sym_node.set_processed(true);
                break;
            case "DotSymbolizer"_case:
                parse_dot_symbolizer(rule, sym_node);
                sym_node.set_processed(true);
                break;

            default:
                break;
        }
    }
}

void map_parser::parse_symbolizer_base(symbolizer_base& sym, xml_node const& node)
{
    set_symbolizer_property<symbolizer_base, double>(sym, keys::simplify_tolerance, node);
    set_symbolizer_property<symbolizer_base, double>(sym, keys::smooth, node);
    set_symbolizer_property<symbolizer_base, value_bool>(sym, keys::clip, node);
    set_symbolizer_property<symbolizer_base, composite_mode_e>(sym, keys::comp_op, node);
    set_symbolizer_property<symbolizer_base, transform_type>(sym, keys::geometry_transform, node);
    set_symbolizer_property<symbolizer_base, simplify_algorithm_e>(sym, keys::simplify_algorithm, node);
    set_symbolizer_property<symbolizer_base, smooth_algorithm_enum>(sym, keys::smooth_algorithm, node);
    set_symbolizer_property<symbolizer_base, double>(sym, keys::extend, node);
}

void map_parser::parse_point_symbolizer(rule& rule, xml_node const& node)
{
    try
    {
        optional<std::string> file = node.get_opt_attr<std::string>("file");
        optional<std::string> base = node.get_opt_attr<std::string>("base");

        point_symbolizer sym;
        parse_symbolizer_base(sym, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::opacity, node);
        set_symbolizer_property<symbolizer_base, value_bool>(sym, keys::allow_overlap, node);
        set_symbolizer_property<symbolizer_base, value_bool>(sym, keys::ignore_placement, node);
        set_symbolizer_property<symbolizer_base, point_placement_enum>(sym, keys::point_placement_type, node);
        set_symbolizer_property<symbolizer_base, transform_type>(sym, keys::image_transform, node);
        if (file && !file->empty())
        {
            if (base)
            {
                std::map<std::string, std::string>::const_iterator itr = file_sources_.find(*base);
                if (itr != file_sources_.end())
                {
                    *file = itr->second + "/" + *file;
                }
            }
            *file = ensure_relative_to_xml(file);
            std::string filename = *file;
            ensure_exists(filename);
            put(sym, keys::file, parse_path(filename));
        }

        rule.append(std::move(sym));
    }
    catch (config_error const& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_dot_symbolizer(rule& rule, xml_node const& node)
{
    try
    {
        dot_symbolizer sym;
        set_symbolizer_property<symbolizer_base, color>(sym, keys::fill, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::opacity, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::width, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::height, node);
        set_symbolizer_property<symbolizer_base, composite_mode_e>(sym, keys::comp_op, node);
        rule.append(std::move(sym));
    }
    catch (config_error const& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_markers_symbolizer(rule& rule, xml_node const& node)
{
    try
    {
        std::string filename("");
        optional<std::string> file = node.get_opt_attr<std::string>("file");
        optional<std::string> base = node.get_opt_attr<std::string>("base");

        if (file && !file->empty())
        {
            if (base)
            {
                std::map<std::string, std::string>::const_iterator itr = file_sources_.find(*base);
                if (itr != file_sources_.end())
                {
                    *file = itr->second + "/" + *file;
                }
            }

            filename = ensure_relative_to_xml(file);
        }

        optional<std::string> marker_type = node.get_opt_attr<std::string>("marker-type");
        if (marker_type)
        {
            // TODO - revisit whether to officially deprecate marker-type
            // https://github.com/mapnik/mapnik/issues/1427
            // MAPNIK_LOG_WARN(markers_symbolizer) << "'marker-type' is deprecated and will be removed in Mapnik 3.x,
            // use file='shape://<type>' to specify known svg shapes";
            // back compatibility with Mapnik 2.0.0
            if (!marker_type->empty() && filename.empty())
            {
                if (*marker_type == "ellipse")
                {
                    filename = marker_cache::instance().known_svg_prefix_ + "ellipse";
                }
                else if (*marker_type == "arrow")
                {
                    filename = marker_cache::instance().known_svg_prefix_ + "arrow";
                }
            }
        }

        markers_symbolizer sym;
        parse_symbolizer_base(sym, node);
        if (!filename.empty())
        {
            ensure_exists(filename);
            put(sym, keys::file, parse_path(filename));
        }
        set_symbolizer_property<symbolizer_base, double>(sym, keys::opacity, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::fill_opacity, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::spacing, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::spacing_offset, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::max_error, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::offset, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::width, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::height, node);
        set_symbolizer_property<symbolizer_base, value_bool>(sym, keys::allow_overlap, node);
        set_symbolizer_property<symbolizer_base, value_bool>(sym, keys::avoid_edges, node);
        set_symbolizer_property<symbolizer_base, value_bool>(sym, keys::ignore_placement, node);
        set_symbolizer_property<symbolizer_base, color>(sym, keys::fill, node);
        set_symbolizer_property<symbolizer_base, transform_type>(sym, keys::image_transform, node);
        set_symbolizer_property<symbolizer_base, marker_placement_enum>(sym, keys::markers_placement_type, node);
        set_symbolizer_property<symbolizer_base, marker_multi_policy_enum>(sym, keys::markers_multipolicy, node);
        set_symbolizer_property<symbolizer_base, direction_enum>(sym, keys::direction, node);
        parse_stroke(sym, node);
        rule.append(std::move(sym));
    }
    catch (config_error const& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_line_pattern_symbolizer(rule& rule, xml_node const& node)
{
    try
    {
        std::string file = node.get_attr<std::string>("file");
        if (file.empty())
        {
            throw config_error("empty file attribute");
        }

        optional<std::string> base = node.get_opt_attr<std::string>("base");

        if (base)
        {
            std::map<std::string, std::string>::const_iterator itr = file_sources_.find(*base);
            if (itr != file_sources_.end())
            {
                file = itr->second + "/" + file;
            }
        }

        file = ensure_relative_to_xml(file);
        ensure_exists(file);
        line_pattern_symbolizer sym;
        parse_symbolizer_base(sym, node);
        put(sym, keys::file, parse_path(file));
        set_symbolizer_property<symbolizer_base, double>(sym, keys::opacity, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::offset, node);
        set_symbolizer_property<symbolizer_base, transform_type>(sym, keys::image_transform, node);
        set_symbolizer_property<symbolizer_base, value_double>(sym, keys::stroke_miterlimit, node);
        set_symbolizer_property<symbolizer_base, value_double>(sym, keys::stroke_width, node);
        set_symbolizer_property<symbolizer_base, line_join_enum>(sym, keys::stroke_linejoin, node);
        set_symbolizer_property<symbolizer_base, line_cap_enum>(sym, keys::stroke_linecap, node);
        set_symbolizer_property<symbolizer_base, dash_array>(sym, keys::stroke_dasharray, node);
        set_symbolizer_property<symbolizer_base, line_pattern_enum>(sym, keys::line_pattern, node);
        set_symbolizer_property<symbolizer_base, pattern_alignment_enum>(sym, keys::alignment, node);
        rule.append(std::move(sym));
    }
    catch (config_error const& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_polygon_pattern_symbolizer(rule& rule, xml_node const& node)
{
    try
    {
        std::string file = node.get_attr<std::string>("file");

        if (file.empty())
        {
            throw config_error("empty file attribute");
        }

        optional<std::string> base = node.get_opt_attr<std::string>("base");

        if (base)
        {
            std::map<std::string, std::string>::const_iterator itr = file_sources_.find(*base);
            if (itr != file_sources_.end())
            {
                file = itr->second + "/" + file;
            }
        }

        file = ensure_relative_to_xml(file);
        ensure_exists(file);
        polygon_pattern_symbolizer sym;
        parse_symbolizer_base(sym, node);
        put(sym, keys::file, parse_path(file));
        set_symbolizer_property<symbolizer_base, double>(sym, keys::opacity, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::gamma, node);
        set_symbolizer_property<symbolizer_base, transform_type>(sym, keys::image_transform, node);
        set_symbolizer_property<symbolizer_base, pattern_alignment_enum>(sym, keys::alignment, node);
        set_symbolizer_property<symbolizer_base, gamma_method_enum>(sym, keys::gamma_method, node);
        rule.append(std::move(sym));
    }
    catch (config_error const& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_text_symbolizer(rule& rule, xml_node const& node)
{
    try
    {
        text_placements_ptr placements;
        optional<std::string> placement_type = node.get_opt_attr<std::string>("placement-type");
        if (placement_type)
        {
            placements = placements::registry::instance().from_xml(*placement_type, node, fontsets_, false);
        }
        else
        {
            placements = std::make_shared<text_placements_dummy>();
            placements->defaults.from_xml(node, fontsets_, false);
        }
        if (placements)
        {
            if (strict_ && !placements->defaults.format_defaults.fontset)
            {
                ensure_font_face(placements->defaults.format_defaults.face_name);
            }
            text_symbolizer sym;
            parse_symbolizer_base(sym, node);
            put<text_placements_ptr>(sym, keys::text_placements_, placements);
            set_symbolizer_property<symbolizer_base, composite_mode_e>(sym, keys::halo_comp_op, node);
            set_symbolizer_property<symbolizer_base, halo_rasterizer_enum>(sym, keys::halo_rasterizer, node);
            set_symbolizer_property<symbolizer_base, transform_type>(sym, keys::halo_transform, node);
            set_symbolizer_property<symbolizer_base, value_double>(sym, keys::offset, node);
            rule.append(std::move(sym));
        }
    }
    catch (config_error const& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_shield_symbolizer(rule& rule, xml_node const& node)
{
    try
    {
        text_placements_ptr placements;
        optional<std::string> placement_type = node.get_opt_attr<std::string>("placement-type");
        if (placement_type)
        {
            placements = placements::registry::instance().from_xml(*placement_type, node, fontsets_, true);
            if (!placements)
                return;
        }
        else
        {
            placements = std::make_shared<text_placements_dummy>();
            placements->defaults.from_xml(node, fontsets_, true);
        }
        if (strict_ && !placements->defaults.format_defaults.fontset)
        {
            ensure_font_face(placements->defaults.format_defaults.face_name);
        }

        shield_symbolizer sym;
        parse_symbolizer_base(sym, node);
        put<text_placements_ptr>(sym, keys::text_placements_, placements);
        set_symbolizer_property<symbolizer_base, transform_type>(sym, keys::image_transform, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::shield_dx, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::shield_dy, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::opacity, node);
        set_symbolizer_property<symbolizer_base, value_bool>(sym, keys::unlock_image, node);
        set_symbolizer_property<symbolizer_base, value_double>(sym, keys::offset, node);

        std::string file = node.get_attr<std::string>("file");
        if (file.empty())
        {
            throw config_error("empty file attribute");
        }

        optional<std::string> base = node.get_opt_attr<std::string>("base");
        if (base)
        {
            std::map<std::string, std::string>::const_iterator itr = file_sources_.find(*base);
            if (itr != file_sources_.end())
            {
                file = itr->second + "/" + file;
            }
        }

        // no_text - removed property in 2.1.x that used to have a purpose
        // before you could provide an expression with an empty string
        optional<mapnik::boolean_type> no_text = node.get_opt_attr<mapnik::boolean_type>("no-text");
        if (no_text)
        {
            MAPNIK_LOG_ERROR(shield_symbolizer)
              << "'no-text' is deprecated and will be removed in Mapnik 3.x, to create a ShieldSymbolizer without text "
                 "just provide an element like: \"<ShieldSymbolizer ... />' '</>\"";
            // FIXME
            //            if (*no_text)
            //              put(shield_symbol, "no-text", set_name(parse_expression("' '"));
        }

        file = ensure_relative_to_xml(file);
        ensure_exists(file);
        put(sym, keys::file, parse_path(file));
        optional<halo_rasterizer_e> halo_rasterizer_ = node.get_opt_attr<halo_rasterizer_e>("halo-rasterizer");
        if (halo_rasterizer_)
            put(sym, keys::halo_rasterizer, halo_rasterizer_enum(*halo_rasterizer_));
        rule.append(std::move(sym));
    }
    catch (config_error const& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_stroke(symbolizer_base& sym, xml_node const& node)
{
    set_symbolizer_property<symbolizer_base, double>(sym, keys::stroke_gamma, node);
    set_symbolizer_property<symbolizer_base, double>(sym, keys::stroke_dashoffset, node);
    set_symbolizer_property<symbolizer_base, double>(sym, keys::stroke_miterlimit, node);
    set_symbolizer_property<symbolizer_base, double>(sym, keys::stroke_width, node);
    set_symbolizer_property<symbolizer_base, double>(sym, keys::stroke_opacity, node);
    set_symbolizer_property<symbolizer_base, color>(sym, keys::stroke, node);
    set_symbolizer_property<symbolizer_base, line_join_enum>(sym, keys::stroke_linejoin, node);
    set_symbolizer_property<symbolizer_base, line_cap_enum>(sym, keys::stroke_linecap, node);
    set_symbolizer_property<symbolizer_base, gamma_method_enum>(sym, keys::stroke_gamma_method, node);
    set_symbolizer_property<symbolizer_base, dash_array>(sym, keys::stroke_dasharray, node);
}

void map_parser::parse_line_symbolizer(rule& rule, xml_node const& node)
{
    try
    {
        line_symbolizer sym;
        parse_symbolizer_base(sym, node);
        parse_stroke(sym, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::offset, node);
        set_symbolizer_property<symbolizer_base, line_rasterizer_enum>(sym, keys::line_rasterizer, node);
        rule.append(std::move(sym));
    }
    catch (config_error const& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_polygon_symbolizer(rule& rule, xml_node const& node)
{
    try
    {
        polygon_symbolizer sym;
        parse_symbolizer_base(sym, node);
        set_symbolizer_property<symbolizer_base, color>(sym, keys::fill, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::fill_opacity, node);
        set_symbolizer_property<symbolizer_base, double>(sym, keys::gamma, node);
        set_symbolizer_property<symbolizer_base, gamma_method_enum>(sym, keys::gamma_method, node);
        rule.append(std::move(sym));
    }
    catch (config_error const& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_building_symbolizer(rule& rule, xml_node const& node)
{
    try
    {
        building_symbolizer building_sym;
        parse_symbolizer_base(building_sym, node);
        set_symbolizer_property<building_symbolizer, color>(building_sym, keys::fill, node);
        set_symbolizer_property<building_symbolizer, double>(building_sym, keys::fill_opacity, node);
        // TODO
        optional<expression_ptr> height = node.get_opt_attr<expression_ptr>("height");
        if (height)
            put(building_sym, keys::height, *height);
        rule.append(std::move(building_sym));
    }
    catch (config_error const& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_raster_symbolizer(rule& rule, xml_node const& node)
{
    try
    {
        raster_symbolizer raster_sym;
        parse_symbolizer_base(raster_sym, node);
        // mode
        optional<std::string> mode = node.get_opt_attr<std::string>("mode");
        if (mode)
        {
            std::string mode_string = *mode;
            if (mode_string.find('_') != std::string::npos)
            {
                MAPNIK_LOG_ERROR(raster_symbolizer)
                  << "'mode' values using \"_\" are deprecated and will be removed in Mapnik 3.x, use \"-\"instead";
                std::replace(mode_string.begin(), mode_string.end(), '_', '-');
            }
            put(raster_sym, keys::mode, mode_string);
        }

        // scaling
        optional<std::string> scaling = node.get_opt_attr<std::string>("scaling");
        if (scaling)
        {
            std::string scaling_method = *scaling;
            if (scaling_method == "fast")
            {
                MAPNIK_LOG_ERROR(raster_symbolizer) << "'scaling' value of 'fast' is deprecated and will be removed in "
                                                       "Mapnik 3.x, use 'near' with Mapnik >= 2.1.x";
                put(raster_sym, keys::scaling, SCALING_NEAR);
            }
            else
            {
                const auto method = scaling_method_from_string(scaling_method);
                if (method.has_value())
                {
                    put(raster_sym, keys::scaling, *method);
                }
                else
                {
                    throw config_error("failed to parse 'scaling': '" + *scaling + "'");
                }
            }
        }

        // opacity
        optional<double> opacity = node.get_opt_attr<double>("opacity");
        if (opacity)
            put(raster_sym, keys::opacity, *opacity);

        // filter factor
        optional<double> filter_factor = node.get_opt_attr<double>("filter-factor");
        if (filter_factor)
            put(raster_sym, keys::filter_factor, *filter_factor);

        // mesh-size
        optional<unsigned> mesh_size = node.get_opt_attr<unsigned>("mesh-size");
        if (mesh_size)
            put<value_integer>(raster_sym, keys::mesh_size, *mesh_size);

        // premultiplied status of image
        optional<mapnik::boolean_type> premultiplied = node.get_opt_attr<mapnik::boolean_type>("premultiplied");
        if (premultiplied)
            put(raster_sym, keys::premultiplied, bool(*premultiplied));

        bool found_colorizer = false;
        for (auto const& css : node)
        {
            if (css.is("RasterColorizer"))
            {
                found_colorizer = true;
                raster_colorizer_ptr colorizer = std::make_shared<raster_colorizer>();
                if (parse_raster_colorizer(colorizer, css))
                    put(raster_sym, keys::colorizer, colorizer);
            }
        }
        // look for properties one level up
        if (!found_colorizer)
        {
            raster_colorizer_ptr colorizer = std::make_shared<raster_colorizer>();
            if (parse_raster_colorizer(colorizer, node))
                put(raster_sym, keys::colorizer, colorizer);
        }
        rule.append(std::move(raster_sym));
    }
    catch (config_error const& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_group_symbolizer(rule& rule, xml_node const& node)
{
    try
    {
        group_symbolizer symbol;
        parse_symbolizer_base(symbol, node);
        group_symbolizer_properties_ptr prop = std::make_shared<group_symbolizer_properties>();
        set_symbolizer_property<symbolizer_base, value_integer>(symbol, keys::num_columns, node);
        set_symbolizer_property<symbolizer_base, value_integer>(symbol, keys::start_column, node);
        set_symbolizer_property<symbolizer_base, expression_ptr>(symbol, keys::repeat_key, node);
        text_placements_ptr placements = std::make_shared<text_placements_dummy>();
        placements->defaults.text_properties_from_xml(node);
        put<text_placements_ptr>(symbol, keys::text_placements_, placements);

        size_t layout_count = 0;
        for (auto const& child_node : node)
        {
            if (child_node.is("GroupRule"))
            {
                parse_group_rule(*prop, child_node);
                child_node.set_processed(true);
            }
            else if (child_node.is("SimpleLayout"))
            {
                parse_simple_layout(*prop, child_node);
                child_node.set_processed(true);
                ++layout_count;
            }
            else if (child_node.is("PairLayout"))
            {
                parse_pair_layout(*prop, child_node);
                child_node.set_processed(true);
                ++layout_count;
            }
            if (layout_count > 1)
            {
                throw config_error("Provide only one layout for a GroupSymbolizer.");
            }
        }
        put(symbol, keys::group_properties, prop);
        rule.append(std::move(symbol));
    }
    catch (const config_error& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_debug_symbolizer(rule& rule, xml_node const& node)
{
    debug_symbolizer symbol;
    parse_symbolizer_base(symbol, node);
    // TODO
    optional<debug_symbolizer_mode_e> mode = node.get_opt_attr<debug_symbolizer_mode_e>("mode");
    if (mode)
        put(symbol, keys::mode, debug_symbolizer_mode_enum(*mode));
    rule.append(std::move(symbol));
}

bool map_parser::parse_raster_colorizer(raster_colorizer_ptr const& rc, xml_node const& node)
{
    bool found_stops = false;
    try
    {
        // mode
        colorizer_mode default_mode =
          node.get_attr<colorizer_mode>("default-mode", colorizer_mode_enum::COLORIZER_LINEAR);

        if (default_mode == colorizer_mode_enum::COLORIZER_INHERIT)
        {
            throw config_error("RasterColorizer mode must not be INHERIT. ");
        }
        rc->set_default_mode(default_mode);

        // default colour
        optional<color> default_color = node.get_opt_attr<color>("default-color");
        if (default_color)
        {
            rc->set_default_color(*default_color);
        }

        // epsilon
        optional<float> eps = node.get_opt_attr<float>("epsilon");
        if (eps)
        {
            if (*eps < 0)
            {
                throw config_error("RasterColorizer epsilon must be > 0. ");
            }
            rc->set_epsilon(*eps);
        }

        float maximumValue = -std::numeric_limits<float>::max();
        for (auto const& n : node)
        {
            if (n.is("stop"))
            {
                found_stops = true;
                // colour is optional.
                optional<color> stopcolor = n.get_opt_attr<color>("color");
                if (!stopcolor)
                {
                    *stopcolor = *default_color;
                }

                // mode default to INHERIT
                colorizer_mode mode = n.get_attr<colorizer_mode>("mode", colorizer_mode_enum::COLORIZER_INHERIT);

                // value is required, and it must be bigger than the previous
                optional<float> val = n.get_opt_attr<float>("value");

                if (!val)
                {
                    throw config_error("stop tag missing value");
                }

                if (val < maximumValue)
                {
                    throw config_error("stop tag values must be in ascending order");
                }
                maximumValue = *val;

                optional<std::string> label = n.get_opt_attr<std::string>("label");

                // append the stop
                colorizer_stop stop;
                stop.set_color(*stopcolor);
                stop.set_mode(mode);
                stop.set_value(*val);
                if (label)
                {
                    stop.set_label(*label);
                }

                rc->add_stop(stop);
            }
        }
    }
    catch (config_error const& ex)
    {
        ex.append_context(node);
        throw;
    }
    return found_stops;
}

void map_parser::parse_group_rule(group_symbolizer_properties& prop, xml_node const& node)
{
    try
    {
        rule fake_rule;
        expression_ptr filter, repeat_key;

        xml_node const *filter_child = node.get_opt_child("Filter"), *rptkey_child = node.get_opt_child("RepeatKey");

        if (filter_child)
        {
            filter = filter_child->get_value<expression_ptr>();
        }
        else
        {
            filter = std::make_shared<mapnik::expr_node>(true);
        }

        if (rptkey_child)
        {
            repeat_key = rptkey_child->get_value<expression_ptr>();
        }

        group_rule_ptr rule = std::make_shared<group_rule>(filter, repeat_key);

        parse_symbolizers(fake_rule, node);

        for (auto const& sym : fake_rule)
        {
            rule->append(std::move(sym));
        }

        prop.add_rule(std::move(rule));
    }
    catch (const config_error& ex)
    {
        ex.append_context(node);
        throw;
    }
}

void map_parser::parse_simple_layout(group_symbolizer_properties& prop, xml_node const& node)
{
    simple_row_layout layout;

    optional<double> item_margin = node.get_opt_attr<double>("item-margin");
    if (item_margin)
        layout.set_item_margin(*item_margin);

    prop.set_layout(std::move(layout));
}

void map_parser::parse_pair_layout(group_symbolizer_properties& prop, xml_node const& node)
{
    pair_layout layout;

    optional<double> item_margin = node.get_opt_attr<double>("item-margin");
    if (item_margin)
        layout.set_item_margin(*item_margin);

    optional<double> max_difference = node.get_opt_attr<double>("max-difference");
    if (max_difference)
        layout.set_max_difference(*max_difference);

    prop.set_layout(std::move(layout));
}

void map_parser::ensure_font_face(std::string const& face_name)
{
    bool found = false;
    auto itr = font_name_cache_.find(face_name);
    if (itr != font_name_cache_.end())
    {
        found = itr->second;
    }
    else
    {
        found = freetype_engine::can_open(face_name, font_library_, font_file_mapping_, freetype_engine::get_mapping());
        font_name_cache_.emplace(face_name, found);
    }
    if (!found)
    {
        throw config_error("Failed to find font face '" + face_name + "'");
    }
}

std::string map_parser::ensure_relative_to_xml(std::optional<std::string> const& opt_path)
{
    if (marker_cache::instance().is_uri(*opt_path))
        return *opt_path;

    if (!xml_base_path_.empty())
    {
        std::string starting_path = *opt_path;
        if (mapnik::util::is_relative(starting_path))
        {
            return mapnik::util::make_absolute(starting_path, xml_base_path_);
        }
    }
    return *opt_path;
}

void map_parser::ensure_exists(std::string const& file_path)
{
    if (marker_cache::instance().is_uri(file_path))
        return;
    // validate that the filename exists if it is not a dynamic PathExpression
    if (file_path.find('[') == std::string::npos && file_path.find(']') == std::string::npos)
    {
        if (!mapnik::util::exists(file_path))
        {
            throw mapnik::config_error("file could not be found: '" + file_path + "'");
        }
    }
}

void map_parser::find_unused_nodes(xml_node const& root)
{
    std::string error_message;
    find_unused_nodes_recursive(root, error_message);
    if (!error_message.empty())
    {
        std::string msg("Unable to process some data while parsing '" + filename_ + "':" + error_message);
        if (strict_)
        {
            throw config_error(msg);
        }
        else
        {
            MAPNIK_LOG_ERROR(load_map) << msg;
        }
    }
}

void map_parser::find_unused_nodes_recursive(xml_node const& node, std::string& error_message)
{
    if (!node.ignore())
    {
        if (!node.processed())
        {
            if (node.is_text())
            {
                error_message += "\n* text '" + node.text();
            }
            else
            {
                error_message += "\n* node '" + node.name();
            }
            error_message += "' at line " + node.line_to_string();

            return; // All attributes and children are automatically unprocessed, too.
        }
        xml_node::attribute_map const& attrs = node.get_attributes();
        for (auto const& attr : attrs)
        {
            if (!attr.second.processed)
            {
                error_message += "\n* attribute '" + attr.first + "' with value '" + attr.second.value + "' at line " +
                                 node.line_to_string();
            }
        }
        for (auto const& child_node : node)
        {
            find_unused_nodes_recursive(child_node, error_message);
        }
    }
}

void map_parser::check_styles(Map const& map)
{
    for (auto const& layer : map.layers())
    {
        for (auto const& style : layer.styles())
        {
            if (!map.find_style(style))
            {
                throw config_error("Unable to process some data while parsing '" + filename_ + "': Style '" + style +
                                   "' required for layer '" + layer.name() + "'.");
            }
        }
    }
}

} // end of namespace mapnik
