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
#include <mapnik/debug.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/xml_tree.hpp>
#include <mapnik/version.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/color.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/xml_loader.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/parse_transform.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/svg/svg_path_parser.hpp>
#include <mapnik/text/placements/registry.hpp>
#include <mapnik/text/placements/dummy.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/util/dasharray_parser.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/noncopyable.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/image_filter_types.hpp>
#include <mapnik/projection.hpp>

// boost
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/static_assert.hpp>

// agg
#include "agg_trans_affine.h"

using boost::tokenizer;

namespace mapnik
{
using boost::optional;

class map_parser : mapnik::noncopyable {
public:
    map_parser(bool strict, std::string const& filename = "") :
        strict_(strict),
        filename_(filename),
        relative_to_xml_(true),
        font_manager_(font_engine_),
        xml_base_path_()
    {}

    void parse_map(Map & map, xml_node const& sty, std::string const& base_path);
private:
    void parse_map_include(Map & map, xml_node const& include);
    void parse_style(Map & map, xml_node const& sty);
    void parse_layer(Map & map, xml_node const& lay);
    void parse_symbolizer_base(symbolizer_base &sym, xml_node const& pt);

    void parse_fontset(Map & map, xml_node const & fset);
    bool parse_font(font_set & fset, xml_node const& f);

    void parse_rule(feature_type_style & style, xml_node const & r);

    void parse_point_symbolizer(rule & rule, xml_node const& sym);
    void parse_line_pattern_symbolizer(rule & rule, xml_node const& sym);
    void parse_polygon_pattern_symbolizer(rule & rule, xml_node const& sym);
    void parse_text_symbolizer(rule & rule, xml_node const& sym);
    void parse_shield_symbolizer(rule & rule, xml_node const& sym);
    void parse_line_symbolizer(rule & rule, xml_node const& sym);
    void parse_polygon_symbolizer(rule & rule, xml_node const& sym);
    void parse_building_symbolizer(rule & rule, xml_node const& sym);
    void parse_raster_symbolizer(rule & rule, xml_node const& sym);
    void parse_markers_symbolizer(rule & rule, xml_node const& sym);
    void parse_debug_symbolizer(rule & rule, xml_node const& sym);

    bool parse_raster_colorizer(raster_colorizer_ptr const& rc, xml_node const& node);
    bool parse_stroke(stroke & strk, xml_node const & sym);

    void ensure_font_face(std::string const& face_name);
    void find_unused_nodes(xml_node const& root);
    void find_unused_nodes_recursive(xml_node const& node, std::string & error_text);


    std::string ensure_relative_to_xml(boost::optional<std::string> const& opt_path);
    void ensure_exists(std::string const& file_path);
    boost::optional<color> get_opt_color_attr(boost::property_tree::ptree const& node,
                                              std::string const& name);

    bool strict_;
    std::string filename_;
    bool relative_to_xml_;
    std::map<std::string,parameters> datasource_templates_;
    freetype_engine font_engine_;
    face_manager<freetype_engine> font_manager_;
    std::map<std::string,std::string> file_sources_;
    std::map<std::string,font_set> fontsets_;
    std::string xml_base_path_;
};

//#include <mapnik/internal/dump_xml.hpp>
void load_map(Map & map, std::string const& filename, bool strict, std::string base_path)
{
    // TODO - use xml encoding?
    xml_tree tree("utf8");
    tree.set_filename(filename);
    read_xml(filename, tree.root());
    map_parser parser(strict, filename);
    parser.parse_map(map, tree.root(), base_path);
    //dump_xml(tree.root());
}

void load_map_string(Map & map, std::string const& str, bool strict, std::string base_path)
{
    // TODO - use xml encoding?
    xml_tree tree("utf8");
    if (!base_path.empty())
    {
        read_xml_string(str, tree.root(), base_path); // accept base_path passed into function
    }
    else
    {
        read_xml_string(str, tree.root(), map.base_path()); // FIXME - this value is not fully known yet
    }
    map_parser parser(strict, base_path);
    parser.parse_map(map, tree.root(), base_path);
}

void map_parser::parse_map(Map & map, xml_node const& pt, std::string const& base_path)
{
    try
    {
        xml_node const& map_node = pt.get_child("Map");
        try
        {
            // Check if relative paths should be interpreted as relative to/from XML location
            // Default is true, and map_parser::ensure_relative_to_xml will be called to modify path
            optional<boolean> paths_from_xml = map_node.get_opt_attr<boolean>("paths-from-xml");
            if (paths_from_xml)
            {
                relative_to_xml_ = *paths_from_xml;
            }

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

            optional<float> opacity = map_node.get_opt_attr<float>("background-image-opacity");
            if (opacity)
            {
                map.set_background_image_opacity(*opacity);
            }

            std::string srs = map_node.get_attr("srs", map.srs());
            try
            {
                // create throwaway projection object here to ensure it is valid
                projection proj(srs);
            }
            catch (std::exception const& ex)
            {
                throw mapnik::config_error(ex.what());
            }
            map.set_srs(srs);

            optional<unsigned> buffer_size = map_node.get_opt_attr<unsigned>("buffer-size");
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
                if (!freetype_engine::register_fonts(ensure_relative_to_xml(font_directory), false))
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
                boost::tokenizer<boost::char_separator<char> > tokens(*min_version_string, sep);
                unsigned i = 0;
                bool success = false;
                int n[3];
                for (boost::tokenizer<boost::char_separator<char> >::iterator beg = tokens.begin();
                     beg != tokens.end(); ++beg)
                {
                    std::string item = mapnik::util::trim_copy(*beg);
                    if (!mapnik::util::string2int(item,n[i]))
                    {
                        throw config_error(std::string("Invalid version string encountered: '")
                            + *beg + "' in '" + *min_version_string + "'");
                    }
                    if (i==2)
                    {
                        success = true;
                        break;
                    }
                    ++i;
                }
                if (success)
                {
                    int min_version = (n[0] * 100000) + (n[1] * 100) + (n[2]);
                    if (min_version > MAPNIK_VERSION)
                    {
                        throw config_error(std::string("This map uses features only present in Mapnik version ") + *min_version_string + " and newer");
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
    find_unused_nodes(pt);
}

void map_parser::parse_map_include(Map & map, xml_node const& include)
{
    try
    {
        xml_node::const_iterator itr = include.begin();
        xml_node::const_iterator end = include.end();

        for (; itr != end; ++itr)
        {
            if (itr->is_text()) continue;
            if (itr->is("Include"))
            {
                parse_map_include(map, *itr);
            }
            else if (itr->is("Style"))
            {
                parse_style(map, *itr);
            }
            else if (itr->is("Layer"))
            {
                parse_layer(map, *itr);
            }
            else if (itr->is("FontSet"))
            {
                parse_fontset(map, *itr);
            }
            else if (itr->is("FileSource"))
            {
                std::string name = itr->get_attr<std::string>("name");
                std::string value = itr->get_text();
                file_sources_[name] = value;
            }
            else if (itr->is("Datasource"))
            {
                std::string name = itr->get_attr("name", std::string("Unnamed"));
                parameters params;
                xml_node::const_iterator paramIter = itr->begin();
                xml_node::const_iterator endParam = itr->end();
                for (; paramIter != endParam; ++paramIter)
                {
                    if (paramIter->is("Parameter"))
                    {
                        std::string param_name = paramIter->get_attr<std::string>("name");
                        std::string value = paramIter->get_text();
                        params[param_name] = value;
                    }
                }
                datasource_templates_[name] = params;
            }
            else if (itr->is("Parameters"))
            {
                parameters & params = map.get_extra_parameters();
                xml_node::const_iterator paramIter = itr->begin();
                xml_node::const_iterator endParam = itr->end();
                for (; paramIter != endParam; ++paramIter)
                {
                    if (paramIter->is("Parameter"))
                    {
                        std::string name = paramIter->get_attr<std::string>("name");
                        bool is_string = true;
                        boost::optional<std::string> type = paramIter->get_opt_attr<std::string>("type");
                        if (type)
                        {
                            if (*type == "int")
                            {
                                is_string = false;
                                mapnik::value_integer value = paramIter->get_value<mapnik::value_integer>();
                                params[name] = value;
                            }
                            else if (*type == "float")
                            {
                                is_string = false;
                                double value = paramIter->get_value<mapnik::value_double>();
                                params[name] = value;
                            }
                        }

                        if (is_string)
                        {
                            std::string value = paramIter->get_text();
                            params[name] = value;
                        }
                    }
                }
            }
        }
    } catch (config_error const& ex) {
        ex.append_context(include);
        throw;
    }
}

void map_parser::parse_style(Map & map, xml_node const& sty)
{
    std::string name("<missing name>");
    try
    {
        name = sty.get_attr<std::string>("name");
        feature_type_style style;

        filter_mode_e filter_mode = sty.get_attr<filter_mode_e>("filter-mode", FILTER_ALL);
        style.set_filter_mode(filter_mode);

        // compositing
        optional<std::string> comp_op_name = sty.get_opt_attr<std::string>("comp-op");
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

        optional<float> opacity = sty.get_opt_attr<float>("opacity");
        if (opacity)
        {
            style.set_opacity(*opacity);
        }

        // image filters
        optional<std::string> filters = sty.get_opt_attr<std::string>("image-filters");
        if (filters)
        {
            std::string filter_str = *filters;
            bool result = filter::parse_image_filters(filter_str, style.image_filters());
            if (!result)
            {
                throw config_error("failed to parse image-filters: '" + filter_str + "'");
            }
        }

        // direct image filters (applied directly on main image buffer
        // TODO : consider creating a separate XML node e.g
        // <ImageFilter name="myfilter" op="blur emboss"/>
        //
        optional<std::string> direct_filters = sty.get_opt_attr<std::string>("direct-image-filters");
        if (direct_filters)
        {
            std::string filter_str = *direct_filters;
            std::string::const_iterator itr = filter_str.begin();
            std::string::const_iterator end = filter_str.end();
            boost::spirit::qi::ascii::space_type space;
            bool result = boost::spirit::qi::phrase_parse(itr,end,
                                                          sty.get_tree().image_filters_grammar,
                                                          space,
                                                          style.direct_image_filters());
            if (!result || itr!=end)
            {
                throw config_error("failed to parse direct-image-filters: '" + std::string(itr,end) + "'");
            }
        }

        // rules
        xml_node::const_iterator ruleIter = sty.begin();
        xml_node::const_iterator endRule = sty.end();

        for (; ruleIter!=endRule; ++ruleIter)
        {
            if (ruleIter->is("Rule"))
            {
                parse_rule(style, *ruleIter);
            }
        }

        map.insert_style(name, style);
    } catch (config_error const& ex) {
        ex.append_context(std::string("in style '") + name + "'", sty);
        throw;
    }
}

void map_parser::parse_fontset(Map & map, xml_node const& fset)
{
    std::string name("<missing name>");
    try
    {
        name = fset.get_attr<std::string>("name");
        font_set fontset(name);
        xml_node::const_iterator itr = fset.begin();
        xml_node::const_iterator end = fset.end();

        bool success = false;
        for (; itr != end; ++itr)
        {
            if (itr->is("Font"))
            {
                if (parse_font(fontset, *itr))
                {
                    success = true;
                }
            }
        }

        // if not at least one face-name is valid
        if (!success)
        {
            throw mapnik::config_error("no valid fonts could be loaded");
        }

        map.insert_fontset(name, fontset);

        // XXX Hack because map object isn't accessible by text_symbolizer
        // when it's parsed
        fontsets_.insert(std::pair<std::string, font_set>(name, fontset));
    }
    catch (config_error const& ex)
    {
        ex.append_context(std::string("in FontSet '") + name + "'", fset);
        throw;
    }
}

bool map_parser::parse_font(font_set &fset, xml_node const& f)
{
    optional<std::string> face_name = f.get_opt_attr<std::string>("face-name");
    if (face_name)
    {
        face_ptr face = font_manager_.get_face(*face_name);
        if (face)
        {
            fset.add_face_name(*face_name);
            return true;
        }
        else if (strict_)
        {
            throw config_error("Failed to find font face '" +
                               *face_name + "'");
        }
    }
    else
    {
        throw config_error("Must have 'face-name' set", f);
    }
    return false;
}

void map_parser::parse_layer(Map & map, xml_node const& node)
{
    std::string name;
    try
    {
        name = node.get_attr("name", std::string("Unnamed"));

        // If no projection is given inherit from map
        std::string srs = node.get_attr("srs", map.srs());
        try
        {
            // create throwaway projection object here to ensure it is valid
            projection proj(srs);
        }
        catch (std::exception const& ex)
        {
            throw mapnik::config_error(ex.what());
        }
        layer lyr(name, srs);

        optional<boolean> status = node.get_opt_attr<boolean>("status");
        if (status)
        {
            lyr.set_active(* status);
        }

        optional<double> min_zoom = node.get_opt_attr<double>("minzoom");
        if (min_zoom)
        {
            lyr.set_min_zoom(* min_zoom);
        }


        optional<double> max_zoom = node.get_opt_attr<double>("maxzoom");
        if (max_zoom)
        {
            lyr.set_max_zoom(* max_zoom);
        }

        optional<boolean> queryable = node.get_opt_attr<boolean>("queryable");
        if (queryable)
        {
            lyr.set_queryable(* queryable);
        }

        optional<boolean> clear_cache =
            node.get_opt_attr<boolean>("clear-label-cache");
        if (clear_cache)
        {
            lyr.set_clear_label_cache(* clear_cache);
        }

        optional<boolean> cache_features =
            node.get_opt_attr<boolean>("cache-features");
        if (cache_features)
        {
            lyr.set_cache_features(* cache_features);
        }

        optional<std::string> group_by =
            node.get_opt_attr<std::string>("group-by");
        if (group_by)
        {
            lyr.set_group_by(* group_by);
        }

        optional<unsigned> buffer_size = node.get_opt_attr<unsigned>("buffer-size");
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

        xml_node::const_iterator child = node.begin();
        xml_node::const_iterator end = node.end();

        for(; child != end; ++child)
        {

            if (child->is("StyleName"))
            {
                std::string style_name = child->get_text();
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
            else if (child->is("Datasource"))
            {
                parameters params;
                optional<std::string> base = child->get_opt_attr<std::string>("base");
                if(base)
                {
                    std::map<std::string,parameters>::const_iterator base_itr = datasource_templates_.find(*base);
                    if (base_itr!=datasource_templates_.end())
                    {
                        params = base_itr->second;
                    }
                    else
                    {
                        MAPNIK_LOG_ERROR(datasource) << "Datasource template '" << *base
                            << "' not found for layer '" << name << "'";
                    }
                }

                xml_node::const_iterator paramIter = child->begin();
                xml_node::const_iterator endParam = child->end();
                for (; paramIter != endParam; ++paramIter)
                {
                    if (paramIter->is("Parameter"))
                    {
                        std::string param_name = paramIter->get_attr<std::string>("name");
                        std::string value = paramIter->get_text();
                        params[param_name] = value;
                    }
                }

                boost::optional<std::string> base_param = params.get<std::string>("base");
                boost::optional<std::string> file_param = params.get<std::string>("file");

                if (base_param){
                    params["base"] = ensure_relative_to_xml(base_param);
                }

                else if (file_param){
                    params["file"] = ensure_relative_to_xml(file_param);
                }

                //now we are ready to create datasource
                try
                {
                    std::shared_ptr<datasource> ds =
                        datasource_cache::instance().create(params);
                    lyr.set_datasource(ds);
                }
                catch (std::exception const& ex)
                {
                    throw config_error(ex.what());
                }
                catch (...)
                {
                    throw config_error("Unknown exception occured attempting to create datasoure for layer '" + lyr.name() + "'");
                }
            }
        }
        map.add_layer(lyr);
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

void map_parser::parse_rule(feature_type_style & style, xml_node const& r)
{
    std::string name;
    try
    {
        name = r.get_attr("name", std::string());
        rule rule(name);

        xml_node const* child = r.get_opt_child("Filter");
        if (child)
        {
            rule.set_filter(child->get_value<expression_ptr>());
        }

        if (r.has_child("ElseFilter"))
        {
            rule.set_else(true);
        }

        if (r.has_child("AlsoFilter"))
        {
            rule.set_also(true);
        }

        child = r.get_opt_child("MinScaleDenominator");
        if (child)
        {
            rule.set_min_scale(child->get_value<double>());
        }

        child = r.get_opt_child("MaxScaleDenominator");
        if (child)
        {
            rule.set_max_scale(child->get_value<double>());
        }

        xml_node::const_iterator symIter = r.begin();
        xml_node::const_iterator endSym = r.end();

        for(;symIter != endSym; ++symIter)
        {

            if (symIter->is("PointSymbolizer"))
            {
                parse_point_symbolizer(rule, *symIter);
            }
            else if (symIter->is("LinePatternSymbolizer"))
            {
                parse_line_pattern_symbolizer(rule, *symIter);
            }
            else if (symIter->is("PolygonPatternSymbolizer"))
            {
                parse_polygon_pattern_symbolizer(rule, *symIter);
            }
            else if (symIter->is("TextSymbolizer"))
            {
                parse_text_symbolizer(rule, *symIter);
            }
            else if (symIter->is("ShieldSymbolizer"))
            {
                parse_shield_symbolizer(rule, *symIter);
            }
            else if (symIter->is("LineSymbolizer"))
            {
                parse_line_symbolizer(rule, *symIter);
            }
            else if (symIter->is("PolygonSymbolizer"))
            {
                parse_polygon_symbolizer(rule, *symIter);
            }
            else if (symIter->is("BuildingSymbolizer"))
            {
                parse_building_symbolizer(rule, *symIter);
            }
            else if (symIter->is("RasterSymbolizer"))
            {
                parse_raster_symbolizer(rule, *symIter);
            }
            else if (symIter->is("MarkersSymbolizer"))
            {
                parse_markers_symbolizer(rule, *symIter);
            }
            else if (symIter->is("DebugSymbolizer"))
            {
                parse_debug_symbolizer(rule, *symIter);
            }
        }
        style.add_rule(rule);

    }
    catch (config_error const& ex)
    {
        if (!name.empty())
        {
            ex.append_context(std::string("in rule '") + name + "'", r);
        }
        throw;
    }
}

void map_parser::parse_symbolizer_base(symbolizer_base &sym, xml_node const &pt)
{
    optional<std::string> comp_op_name = pt.get_opt_attr<std::string>("comp-op");
    if (comp_op_name)
    {
        optional<composite_mode_e> comp_op = comp_op_from_string(*comp_op_name);
        if (comp_op)
        {
            sym.set_comp_op(*comp_op);
        }
        else
        {
            throw config_error("failed to parse comp-op: '" + *comp_op_name + "'");
        }
    }

    optional<std::string> geometry_transform_wkt = pt.get_opt_attr<std::string>("geometry-transform");
    if (geometry_transform_wkt)
    {
        mapnik::transform_list_ptr tl = std::make_shared<mapnik::transform_list>();
        if (!mapnik::parse_transform(*tl, *geometry_transform_wkt, pt.get_tree().transform_expr_grammar))
        {
            std::string ss("Could not parse transform from '");
            ss += *geometry_transform_wkt + "', expected transform attribute";
            throw config_error(ss);
        }
        sym.set_transform(tl);
    }

    optional<boolean> clip = pt.get_opt_attr<boolean>("clip");
    if (clip) sym.set_clip(*clip);

    // simplify algorithm
    optional<std::string> simplify_algorithm_name = pt.get_opt_attr<std::string>("simplify-algorithm");
    if (simplify_algorithm_name)
    {
        optional<simplify_algorithm_e> simplify_algorithm = simplify_algorithm_from_string(*simplify_algorithm_name);
        if (simplify_algorithm)
        {
            sym.set_simplify_algorithm(*simplify_algorithm);
        }
        else
        {
            throw config_error("failed to parse simplify-algorithm: '" + *simplify_algorithm_name + "'");
        }
    }

    // simplify value
    optional<double> simplify_tolerance = pt.get_opt_attr<double>("simplify");
    if (simplify_tolerance) sym.set_simplify_tolerance(*simplify_tolerance);

    // smooth value
    optional<double> smooth = pt.get_opt_attr<double>("smooth");
    if (smooth) sym.set_smooth(*smooth);
}

void map_parser::parse_point_symbolizer(rule & rule, xml_node const & sym)
{
    try
    {
        optional<std::string> file = sym.get_opt_attr<std::string>("file");
        optional<std::string> base = sym.get_opt_attr<std::string>("base");
        optional<boolean> allow_overlap = sym.get_opt_attr<boolean>("allow-overlap");
        optional<boolean> ignore_placement = sym.get_opt_attr<boolean>("ignore-placement");
        optional<float> opacity = sym.get_opt_attr<float>("opacity");
        optional<std::string> image_transform_wkt = sym.get_opt_attr<std::string>("transform");

        point_symbolizer symbol;
        if (allow_overlap)
        {
            symbol.set_allow_overlap(* allow_overlap);
        }
        if (opacity)
        {
            symbol.set_opacity(* opacity);
        }
        if (ignore_placement)
        {
            symbol.set_ignore_placement(* ignore_placement);
        }
        point_placement_e placement =
            sym.get_attr<point_placement_e>("placement", symbol.get_point_placement());
        symbol.set_point_placement(placement);

        if (file && !file->empty())
        {
            if(base)
            {
                std::map<std::string,std::string>::const_iterator itr = file_sources_.find(*base);
                if (itr!=file_sources_.end())
                {
                    *file = itr->second + "/" + *file;
                }
            }

            *file = ensure_relative_to_xml(file);
            std::string filename = *file;
            ensure_exists(filename);
            symbol.set_filename( parse_path(filename, sym.get_tree().path_expr_grammar) );

            if (image_transform_wkt)
            {
                mapnik::transform_list_ptr tl = std::make_shared<mapnik::transform_list>();
                if (!mapnik::parse_transform(*tl, *image_transform_wkt, sym.get_tree().transform_expr_grammar))
                {
                    throw mapnik::config_error("Failed to parse transform: '" + *image_transform_wkt + "'");
                }
                symbol.set_image_transform(tl);
            }
        }
        parse_symbolizer_base(symbol, sym);
        rule.append(symbol);
    }
    catch (config_error const& ex)
    {
        ex.append_context(sym);
        throw;
    }
}

void map_parser::parse_markers_symbolizer(rule & rule, xml_node const& sym)
{
    try
    {
        std::string filename("");
        optional<std::string> file = sym.get_opt_attr<std::string>("file");
        optional<std::string> base = sym.get_opt_attr<std::string>("base");

        if (file && !file->empty())
        {
            if (base)
            {
                std::map<std::string,std::string>::const_iterator itr = file_sources_.find(*base);
                if (itr!=file_sources_.end())
                {
                    *file = itr->second + "/" + *file;
                }
            }

            filename = ensure_relative_to_xml(file);
        }

        optional<std::string> marker_type = sym.get_opt_attr<std::string>("marker-type");
        if (marker_type)
        {
            // TODO - revisit whether to officially deprecate marker-type
            // https://github.com/mapnik/mapnik/issues/1427
            //MAPNIK_LOG_WARN(markers_symbolizer) << "'marker-type' is deprecated and will be removed in Mapnik 3.x, use file='shape://<type>' to specify known svg shapes";
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

        markers_symbolizer symbol;

        if (!filename.empty())
        {
            ensure_exists(filename);
            symbol.set_filename( parse_path(filename, sym.get_tree().path_expr_grammar) );
        }

        // overall opacity to be applied to all paths
        optional<float> opacity = sym.get_opt_attr<float>("opacity");
        if (opacity) symbol.set_opacity(*opacity);

        optional<float> fill_opacity = sym.get_opt_attr<float>("fill-opacity");
        if (fill_opacity) symbol.set_fill_opacity(*fill_opacity);

        optional<std::string> image_transform_wkt = sym.get_opt_attr<std::string>("transform");
        if (image_transform_wkt)
        {
            mapnik::transform_list_ptr tl = std::make_shared<mapnik::transform_list>();
            if (!mapnik::parse_transform(*tl, *image_transform_wkt, sym.get_tree().transform_expr_grammar))
            {
                throw mapnik::config_error("Failed to parse transform: '" + *image_transform_wkt + "'");
            }
            symbol.set_image_transform(tl);
        }

        optional<color> c = sym.get_opt_attr<color>("fill");
        if (c) symbol.set_fill(*c);

        optional<double> spacing = sym.get_opt_attr<double>("spacing");
        if (spacing) symbol.set_spacing(*spacing);

        optional<double> max_error = sym.get_opt_attr<double>("max-error");
        if (max_error) symbol.set_max_error(*max_error);

        optional<boolean> allow_overlap = sym.get_opt_attr<boolean>("allow-overlap");
        if (allow_overlap) symbol.set_allow_overlap(*allow_overlap);

        optional<boolean> ignore_placement = sym.get_opt_attr<boolean>("ignore-placement");
        if (ignore_placement) symbol.set_ignore_placement(*ignore_placement);

        optional<expression_ptr> width = sym.get_opt_attr<expression_ptr>("width");
        if (width) symbol.set_width(*width);

        optional<expression_ptr> height = sym.get_opt_attr<expression_ptr>("height");
        if (height) symbol.set_height(*height);

        stroke strk;
        if (parse_stroke(strk,sym))
        {
            symbol.set_stroke(strk);
        }

        marker_placement_e placement = sym.get_attr<marker_placement_e>("placement",symbol.get_marker_placement());
        symbol.set_marker_placement(placement);

        marker_multi_policy_e mpolicy = sym.get_attr<marker_multi_policy_e>("multi-policy",symbol.get_marker_multi_policy());
        symbol.set_marker_multi_policy(mpolicy);

        parse_symbolizer_base(symbol, sym);
        rule.append(symbol);
    }
    catch (config_error const& ex)
    {
        ex.append_context(sym);
        throw;
    }
}

void map_parser::parse_line_pattern_symbolizer(rule & rule, xml_node const & sym)
{
    try
    {
        std::string file = sym.get_attr<std::string>("file");
        if (file.empty())
        {
            throw config_error("empty file attribute");
        }

        optional<std::string> base = sym.get_opt_attr<std::string>("base");

        if(base)
        {
            std::map<std::string,std::string>::const_iterator itr = file_sources_.find(*base);
            if (itr!=file_sources_.end())
            {
                file = itr->second + "/" + file;
            }
        }

        file = ensure_relative_to_xml(file);
        ensure_exists(file);
        line_pattern_symbolizer symbol( parse_path(file, sym.get_tree().path_expr_grammar) );

        // offset value
        optional<double> offset = sym.get_opt_attr<double>("offset");
        if (offset) symbol.set_offset(*offset);

        parse_symbolizer_base(symbol, sym);
        rule.append(symbol);
    }
    catch (config_error const& ex)
    {
        ex.append_context(sym);
        throw;
    }
}

void map_parser::parse_polygon_pattern_symbolizer(rule & rule,
                                                  xml_node const & sym)
{
    try
    {
        std::string file = sym.get_attr<std::string>("file");

        if (file.empty())
        {
            throw config_error("empty file attribute");
        }

        optional<std::string> base = sym.get_opt_attr<std::string>("base");

        if(base)
        {
            std::map<std::string,std::string>::iterator itr = file_sources_.find(*base);
            if (itr!=file_sources_.end())
            {
                file = itr->second + "/" + file;
            }
        }

        file = ensure_relative_to_xml(file);
        ensure_exists(file);
        polygon_pattern_symbolizer symbol( parse_path(file, sym.get_tree().path_expr_grammar) );

        // pattern alignment
        pattern_alignment_e p_alignment = sym.get_attr<pattern_alignment_e>("alignment",LOCAL_ALIGNMENT);
        symbol.set_alignment(p_alignment);

        // opacity
        optional<float> opacity = sym.get_opt_attr<float>("opacity");
        if (opacity) symbol.set_opacity(*opacity);

        // gamma
        optional<double> gamma = sym.get_opt_attr<double>("gamma");
        if (gamma)  symbol.set_gamma(*gamma);

        // gamma method
        optional<gamma_method_e> gamma_method = sym.get_opt_attr<gamma_method_e>("gamma-method");
        if (gamma_method) symbol.set_gamma_method(*gamma_method);

        parse_symbolizer_base(symbol, sym);
        rule.append(symbol);
    }
    catch (config_error const& ex)
    {
        ex.append_context(sym);
        throw;
    }
}

void map_parser::parse_text_symbolizer(rule & rule, xml_node const& sym)
{
    try
    {
        text_placements_ptr placement_finder;
        optional<std::string> placement_type = sym.get_opt_attr<std::string>("placement-type");
        if (placement_type) {
            placement_finder = placements::registry::instance().from_xml(*placement_type, sym, fontsets_);
        } else {
            placement_finder = std::make_shared<text_placements_dummy>();
            placement_finder->defaults.from_xml(sym, fontsets_);
        }
        if (strict_ && (!placement_finder->defaults.format->fontset ||
            !placement_finder->defaults.format->fontset->size()))
        {
            ensure_font_face(placement_finder->defaults.format->face_name);
        }

        text_symbolizer text_symbol = text_symbolizer(placement_finder);
        parse_symbolizer_base(text_symbol, sym);
        optional<halo_rasterizer_e> halo_rasterizer = sym.get_opt_attr<halo_rasterizer_e>("halo-rasterizer");
        if (halo_rasterizer) text_symbol.set_halo_rasterizer(*halo_rasterizer);

        rule.append(text_symbol);
    }
    catch (config_error const& ex)
    {
        ex.append_context(sym);
        throw;
    }
}

void map_parser::parse_shield_symbolizer(rule & rule, xml_node const& sym)
{
    try
    {
        text_placements_ptr placement_finder;
        optional<std::string> placement_type = sym.get_opt_attr<std::string>("placement-type");
        if (placement_type) {
            placement_finder = placements::registry::instance().from_xml(*placement_type, sym, fontsets_);
        } else {
            placement_finder = std::make_shared<text_placements_dummy>();
        }
        placement_finder->defaults.from_xml(sym, fontsets_);
        if (strict_ &&
            (!placement_finder->defaults.format->fontset ||
            !placement_finder->defaults.format->fontset->size()))
        {
            ensure_font_face(placement_finder->defaults.format->face_name);
        }

        shield_symbolizer shield_symbol = shield_symbolizer(placement_finder);

        optional<std::string> image_transform_wkt = sym.get_opt_attr<std::string>("transform");
        if (image_transform_wkt)
        {
            mapnik::transform_list_ptr tl = std::make_shared<mapnik::transform_list>();
            if (!mapnik::parse_transform(*tl, *image_transform_wkt, sym.get_tree().transform_expr_grammar))
            {
                throw mapnik::config_error("Failed to parse transform: '" + *image_transform_wkt + "'");
            }
            shield_symbol.set_image_transform(tl);
        }

        // shield displacement
        double shield_dx = sym.get_attr("shield-dx", 0.0);
        double shield_dy = sym.get_attr("shield-dy", 0.0);
        shield_symbol.set_shield_displacement(shield_dx,shield_dy);

        // opacity
        optional<float> opacity = sym.get_opt_attr<float>("opacity");
        if (opacity)
        {
            shield_symbol.set_opacity(*opacity);
        }

        // text-opacity
        // TODO: Could be problematic because it is named opacity in TextSymbolizer but opacity has a diffrent meaning here.
        optional<double> text_opacity =
            sym.get_opt_attr<double>("text-opacity");
        if (text_opacity)
        {
            shield_symbol.set_text_opacity(* text_opacity);
        }

        // unlock_image
        optional<boolean> unlock_image =
            sym.get_opt_attr<boolean>("unlock-image");
        if (unlock_image)
        {
            shield_symbol.set_unlock_image(* unlock_image);
        }

        std::string file = sym.get_attr<std::string>("file");
        if (file.empty())
        {
            throw config_error("empty file attribute");
        }

        optional<std::string> base = sym.get_opt_attr<std::string>("base");

        if(base)
        {
            std::map<std::string,std::string>::const_iterator itr = file_sources_.find(*base);
            if (itr!=file_sources_.end())
            {
                file = itr->second + "/" + file;
            }
        }

        // no_text - removed property in 2.1.x that used to have a purpose
        // before you could provide an expression with an empty string
        optional<boolean> no_text =
            sym.get_opt_attr<boolean>("no-text");
        if (no_text)
        {
            MAPNIK_LOG_ERROR(shield_symbolizer) << "'no-text' is deprecated and will be removed in Mapnik 3.x, to create a ShieldSymbolizer without text just provide an element like: \"<ShieldSymbolizer ... />' '</>\"";
            if (*no_text)
                shield_symbol.set_name(parse_expression("' '"));
        }

        file = ensure_relative_to_xml(file);
        ensure_exists(file);
        shield_symbol.set_filename( parse_path(file, sym.get_tree().path_expr_grammar) );
        parse_symbolizer_base(shield_symbol, sym);
        rule.append(shield_symbol);
    }
    catch (config_error const& ex)
    {
        ex.append_context(sym);
        throw;
    }
}

bool map_parser::parse_stroke(stroke & strk, xml_node const & sym)
{
    bool result = false;

    // stroke color
    optional<color> c = sym.get_opt_attr<color>("stroke");
    if (c)
    {
        result = true;
        strk.set_color(*c);
    }

    // stroke-width
    optional<double> width = sym.get_opt_attr<double>("stroke-width");
    if (width)
    {
        result = true;
        strk.set_width(*width);
    }

    // stroke-opacity
    optional<double> opacity = sym.get_opt_attr<double>("stroke-opacity");
    if (opacity)
    {
        result = true;
        strk.set_opacity(*opacity);
    }

    // stroke-linejoin
    optional<line_join_e> line_join = sym.get_opt_attr<line_join_e>("stroke-linejoin");
    if (line_join) strk.set_line_join(*line_join);

    // stroke-linecap
    optional<line_cap_e> line_cap = sym.get_opt_attr<line_cap_e>("stroke-linecap");
    if (line_cap) strk.set_line_cap(*line_cap);

    // stroke-gamma
    optional<double> gamma = sym.get_opt_attr<double>("stroke-gamma");
    if (gamma) strk.set_gamma(*gamma);

    // stroke-gamma-method
    optional<gamma_method_e> gamma_method = sym.get_opt_attr<gamma_method_e>("stroke-gamma-method");
    if (gamma_method) strk.set_gamma_method(*gamma_method);

    // stroke-dashoffset
    optional<double> dash_offset = sym.get_opt_attr<double>("stroke-dashoffset");
    if (dash_offset) strk.set_dash_offset(*dash_offset);

    // stroke-dasharray
    optional<std::string> str = sym.get_opt_attr<std::string>("stroke-dasharray");
    if (str)
    {
        std::vector<double> dash_array;
        if (util::parse_dasharray((*str).begin(),(*str).end(),dash_array))
        {
            if (!dash_array.empty())
            {
                size_t size = dash_array.size();
                if (size % 2 == 1)
                    dash_array.insert(dash_array.end(),dash_array.begin(),dash_array.end());

                std::vector<double>::const_iterator pos = dash_array.begin();
                while (pos != dash_array.end())
                {
                    if (*pos > 0.0 || *(pos+1) > 0.0) // avoid both dash and gap eq 0.0
                        strk.add_dash(*pos,*(pos + 1));
                    pos +=2;
                }
            }
        }
        else
        {
            throw config_error(std::string("Failed to parse dasharray ") +
                               "'. Expected a " +
                               "list of floats or 'none' but got '" + (*str) + "'");
        }
    }

    // stroke-miterlimit
    optional<double> miterlimit = sym.get_opt_attr<double>("stroke-miterlimit");
    if (miterlimit) strk.set_miterlimit(*miterlimit);
    return result;
}

void map_parser::parse_line_symbolizer(rule & rule, xml_node const & sym)
{
    try
    {
        stroke strk;
        parse_stroke(strk,sym);
        line_symbolizer symbol = line_symbolizer(strk);

        // offset value
        optional<double> offset = sym.get_opt_attr<double>("offset");
        if (offset) symbol.set_offset(*offset);

        line_rasterizer_e rasterizer = sym.get_attr<line_rasterizer_e>("rasterizer", RASTERIZER_FULL);
        symbol.set_rasterizer(rasterizer);

        parse_symbolizer_base(symbol, sym);
        rule.append(symbol);
    }
    catch (config_error const& ex)
    {
        ex.append_context(sym);
        throw;
    }
}


void map_parser::parse_polygon_symbolizer(rule & rule, xml_node const & sym)
{
    try
    {
        polygon_symbolizer poly_sym;
        // fill
        optional<color> fill = sym.get_opt_attr<color>("fill");
        if (fill) poly_sym.set_fill(*fill);
        // fill-opacity
        optional<double> opacity = sym.get_opt_attr<double>("fill-opacity");
        if (opacity) poly_sym.set_opacity(*opacity);
        // gamma
        optional<double> gamma = sym.get_opt_attr<double>("gamma");
        if (gamma)  poly_sym.set_gamma(*gamma);
        // gamma method
        optional<gamma_method_e> gamma_method = sym.get_opt_attr<gamma_method_e>("gamma-method");
        if (gamma_method) poly_sym.set_gamma_method(*gamma_method);

        parse_symbolizer_base(poly_sym, sym);
        rule.append(poly_sym);
    }
    catch (config_error const& ex)
    {
        ex.append_context(sym);
        throw;
    }
}

void map_parser::parse_building_symbolizer(rule & rule, xml_node const & sym)
{
    try
    {
        building_symbolizer building_sym;

        // fill
        optional<color> fill = sym.get_opt_attr<color>("fill");
        if (fill) building_sym.set_fill(*fill);
        // fill-opacity
        optional<double> opacity = sym.get_opt_attr<double>("fill-opacity");
        if (opacity) building_sym.set_opacity(*opacity);
        // height
        optional<expression_ptr> height = sym.get_opt_attr<expression_ptr>("height");
        if (height) building_sym.set_height(*height);

        parse_symbolizer_base(building_sym, sym);
        rule.append(building_sym);
    }
    catch (config_error const& ex)
    {
        ex.append_context(sym);
        throw;
    }
}

void map_parser::parse_raster_symbolizer(rule & rule, xml_node const & sym)
{
    try
    {
        raster_symbolizer raster_sym;

        // mode
        optional<std::string> mode = sym.get_opt_attr<std::string>("mode");
        if (mode)
        {
            std::string mode_string = *mode;
            if (boost::algorithm::find_first(mode_string,"_"))
            {
                MAPNIK_LOG_ERROR(raster_symbolizer) << "'mode' values using \"_\" are deprecated and will be removed in Mapnik 3.x, use \"-\"instead";
                boost::algorithm::replace_all(mode_string,"_","-");
            }
            raster_sym.set_mode(mode_string);
        }

        // scaling
        optional<std::string> scaling = sym.get_opt_attr<std::string>("scaling");
        if (scaling)
        {
            std::string scaling_method = *scaling;
            if (scaling_method == "fast")
            {
                MAPNIK_LOG_ERROR(raster_symbolizer) << "'scaling' value of 'fast' is deprecated and will be removed in Mapnik 3.x, use 'near' with Mapnik >= 2.1.x";
                raster_sym.set_scaling_method(SCALING_NEAR);
            }
            else
            {
                boost::optional<scaling_method_e> method = scaling_method_from_string(scaling_method);
                if (method)
                {
                    raster_sym.set_scaling_method(*method);
                }
                else
                {
                    throw config_error("failed to parse 'scaling': '" + *scaling + "'");
                }
            }
        }

        // opacity
        optional<float> opacity = sym.get_opt_attr<float>("opacity");
        if (opacity) raster_sym.set_opacity(*opacity);

        // filter factor
        optional<double> filter_factor = sym.get_opt_attr<double>("filter-factor");
        if (filter_factor) raster_sym.set_filter_factor(*filter_factor);

        // mesh-size
        optional<unsigned> mesh_size = sym.get_opt_attr<unsigned>("mesh-size");
        if (mesh_size) raster_sym.set_mesh_size(*mesh_size);

        // premultiplied status of image
        optional<boolean> premultiplied = sym.get_opt_attr<boolean>("premultiplied");
        if (premultiplied) raster_sym.set_premultiplied(*premultiplied);

        xml_node::const_iterator cssIter = sym.begin();
        xml_node::const_iterator endCss = sym.end();

        bool found_colorizer = false;
        for(; cssIter != endCss; ++cssIter)
        {
            if (cssIter->is("RasterColorizer"))
            {
                found_colorizer = true;
                raster_colorizer_ptr colorizer = std::make_shared<raster_colorizer>();
                raster_sym.set_colorizer(colorizer);
                if (parse_raster_colorizer(colorizer, *cssIter))
                    raster_sym.set_colorizer(colorizer);

            }
        }
        // look for properties one level up
        if (!found_colorizer)
        {
            raster_colorizer_ptr colorizer = std::make_shared<raster_colorizer>();
            if (parse_raster_colorizer(colorizer, sym))
                raster_sym.set_colorizer(colorizer);
        }
        parse_symbolizer_base(raster_sym, sym);
        rule.append(raster_sym);
    }
    catch (config_error const& ex)
    {
        ex.append_context(sym);
        throw;
    }
}

void map_parser::parse_debug_symbolizer(rule & rule, xml_node const & sym)
{
    debug_symbolizer symbol;
    parse_symbolizer_base(symbol, sym);
    debug_symbolizer_mode_e mode =
        sym.get_attr<debug_symbolizer_mode_e>("mode", DEBUG_SYM_MODE_COLLISION);
    symbol.set_mode(mode);
    rule.append(symbol);
}

bool map_parser::parse_raster_colorizer(raster_colorizer_ptr const& rc,
                                        xml_node const& node)
{
    bool found_stops = false;
    try
    {
        // mode
        colorizer_mode default_mode =
            node.get_attr<colorizer_mode>("default-mode", COLORIZER_LINEAR);

        if(default_mode == COLORIZER_INHERIT) {
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
            if(*eps < 0) {
                throw config_error("RasterColorizer epsilon must be > 0. ");
            }
            rc->set_epsilon(*eps);
        }


        xml_node::const_iterator stopIter = node.begin();
        xml_node::const_iterator endStop = node.end();
        float maximumValue = -std::numeric_limits<float>::max();

        for(; stopIter != endStop; ++stopIter)
        {
            if (stopIter->is("stop"))
            {
                found_stops = true;
                // colour is optional.
                optional<color> stopcolor = stopIter->get_opt_attr<color>("color");
                if (!stopcolor) {
                    *stopcolor = *default_color;
                }

                // mode default to INHERIT
                colorizer_mode mode =
                    stopIter->get_attr<colorizer_mode>("mode", COLORIZER_INHERIT);

                // value is required, and it must be bigger than the previous
                optional<float> value =
                    stopIter->get_opt_attr<float>("value");

                if(!value) {
                    throw config_error("stop tag missing value");
                }

                if(value < maximumValue) {
                    throw config_error("stop tag values must be in ascending order");
                }
                maximumValue = *value;

                optional<std::string> label =
                    stopIter->get_opt_attr<std::string>("label");

                //append the stop
                colorizer_stop tmpStop;
                tmpStop.set_color(*stopcolor);
                tmpStop.set_mode(mode);
                tmpStop.set_value(*value);
                if (label)
                    tmpStop.set_label(*label);

                rc->add_stop(tmpStop);
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

void map_parser::ensure_font_face(std::string const& face_name)
{
    if (! font_manager_.get_face(face_name))
    {
        throw config_error("Failed to find font face '" +
                           face_name + "'");
    }
}

std::string map_parser::ensure_relative_to_xml(boost::optional<std::string> const& opt_path)
{
    if (marker_cache::instance().is_uri(*opt_path))
        return *opt_path;

    if (!xml_base_path_.empty() && relative_to_xml_)
    {
        std::string starting_path = *opt_path;
        if (mapnik::util::is_relative(starting_path))
        {
            return mapnik::util::make_absolute(starting_path,xml_base_path_);
        }
    }
    return *opt_path;
}

void map_parser::ensure_exists(std::string const& file_path)
{
    if (marker_cache::instance().is_uri(file_path))
        return;
    // validate that the filename exists if it is not a dynamic PathExpression
    if (!boost::algorithm::find_first(file_path,"[") && !boost::algorithm::find_first(file_path,"]"))
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

void map_parser::find_unused_nodes_recursive(xml_node const& node, std::string & error_message)
{
    if (!node.processed())
    {
        if (node.is_text()) {
            error_message += "\n* text '" + node.text() + "'";
        } else {
            error_message += "\n* node '" + node.name() + "' at line " + node.line_to_string();
        }
        return; //All attributes and children are automatically unprocessed, too.
    }
    xml_node::attribute_map const& attr = node.get_attributes();
    xml_node::attribute_map::const_iterator aitr = attr.begin();
    xml_node::attribute_map::const_iterator aend = attr.end();
    for (;aitr!=aend; aitr++)
    {
        if (!aitr->second.processed)
        {
            error_message += "\n* attribute '" + aitr->first +
                "' with value '" + aitr->second.value +
                "' at line " + node.line_to_string();
        }
    }
    xml_node::const_iterator itr = node.begin();
    xml_node::const_iterator end = node.end();
    for (; itr!=end; itr++)
    {
        find_unused_nodes_recursive(*itr, error_message);
    }
}

} // end of namespace mapnik
