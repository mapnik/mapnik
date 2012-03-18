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
#include <mapnik/load_map.hpp>

#include <mapnik/xml_tree.hpp>
#include <mapnik/version.hpp>
#include <mapnik/image_reader.hpp>
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
#include <mapnik/raster_colorizer.hpp>

#include <mapnik/svg/svg_path_parser.hpp>

#include <mapnik/metawriter_factory.hpp>

#include <mapnik/text_placements/registry.hpp>
#include <mapnik/text_placements/dummy.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/config_error.hpp>

// boost
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/static_assert.hpp>
#include <boost/filesystem/operations.hpp>

// agg
#include "agg_trans_affine.h"

// stl
#include <iostream>
#include <sstream>

using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::tokenizer;

using std::endl;

namespace mapnik
{
using boost::optional;

class map_parser : boost::noncopyable {
public:
    map_parser(bool strict, std::string const& filename = "") :
        strict_(strict),
        filename_(filename),
        relative_to_xml_(true),
        font_manager_(font_engine_)
    {}

    void parse_map(Map & map, xml_node const& sty, std::string const& base_path);
private:
    void parse_map_include(Map & map, xml_node const& include);
    void parse_style(Map & map, xml_node const& sty);
    void parse_layer(Map & map, xml_node const& lay);
    void parse_metawriter(Map & map, xml_node const& lay);
    void parse_metawriter_in_symbolizer(symbolizer_base &sym, xml_node const& pt);

    void parse_fontset(Map & map, xml_node const & fset);
    void parse_font(font_set & fset, xml_node const& f);

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

    void parse_raster_colorizer(raster_colorizer_ptr const& rc, xml_node const& node);
    void parse_stroke(stroke & strk, xml_node const & sym);

    void ensure_font_face(std::string const& face_name);
    void find_unused_nodes(xml_node const& root);
    void find_unused_nodes_recursive(xml_node const& node, std::stringstream &error_text);


    std::string ensure_relative_to_xml(boost::optional<std::string> opt_path);
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
};

#include <mapnik/internal/dump_xml.hpp>
void load_map(Map & map, std::string const& filename, bool strict)
{
    // TODO - use xml encoding?
    xml_tree tree("utf8");
    tree.set_filename(filename);
    read_xml(filename, tree.root());
    map_parser parser(strict, filename);
    parser.parse_map(map, tree.root(), "");
#ifdef MAPNIK_DEBUG
    dump_xml(tree.root());
#endif
}

void load_map_string(Map & map, std::string const& str, bool strict, std::string base_path)
{
    // TODO - use xml encoding?
    xml_tree tree("utf8");
    if (!base_path.empty())
        read_xml_string(str, tree.root(), base_path); // accept base_path passed into function
    else
        read_xml_string(str, tree.root(), map.base_path()); // default to map base_path
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
            parameters extra_attr;

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
            else
            {
                boost::filesystem::path xml_path(filename_);
                // TODO - should we make this absolute?
#if (BOOST_FILESYSTEM_VERSION == 3)
                std::string base = xml_path.parent_path().string();
#else // v2
                std::string base = xml_path.branch_path().string();
#endif

                map.set_base_path(base);
            }

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

            map.set_srs(map_node.get_attr("srs", map.srs()));

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
                    std::ostringstream s_err;
                    s_err << "failed to parse 'maximum-extent'";
                    if (strict_)
                        throw config_error(s_err.str());
                    else
                        std::clog << "### WARNING: " << s_err.str() << std::endl;
                }
            }

            optional<std::string> font_directory = map_node.get_opt_attr<std::string>("font-directory");
            if (font_directory)
            {
                extra_attr["font-directory"] = *font_directory;
                freetype_engine::register_fonts(ensure_relative_to_xml(font_directory), false);
            }

            optional<std::string> min_version_string = map_node.get_opt_attr<std::string>("minimum-version");

            if (min_version_string)
            {
                extra_attr["minimum-version"] = *min_version_string;
                boost::char_separator<char> sep(".");
                boost::tokenizer<boost::char_separator<char> > tokens(*min_version_string, sep);
                unsigned i = 0;
                bool success = false;
                int n[3];
                for (boost::tokenizer<boost::char_separator<char> >::iterator beg = tokens.begin();
                     beg != tokens.end(); ++beg)
                {
                    try
                    {
                        n[i] = boost::lexical_cast<int>(boost::trim_copy(*beg));
                    }
                    catch (boost::bad_lexical_cast & ex)
                    {
                        std::clog << *beg << " : " << ex.what() << "\n";
                        break;
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

            map.set_extra_attributes(extra_attr);
        }
        catch (const config_error & ex)
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
            else if (itr->is("MetaWriter"))
            {
                parse_metawriter(map, *itr);
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
                        std::string name = paramIter->get_attr<std::string>("name");
                        std::string value = paramIter->get_text();
                        params[name] = value;
                    }
                }
                datasource_templates_[name] = params;
            }
            else if (itr->is("Parameters"))
            {
                std::string name = itr->get_attr("name", std::string("Unnamed"));
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
                                int value = paramIter->get_value<int>();
                                params[name] = value;
                            }
                            else if (*type == "float")
                            {
                                is_string = false;
                                double value = paramIter->get_value<double>();
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
    } catch (const config_error & ex) {
        ex.append_context(include);
        throw;
    }

    map.init_metawriters();
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
    } catch (const config_error & ex) {
        ex.append_context(std::string("in style '") + name + "'", sty);
        throw;
    }
}

void map_parser::parse_metawriter(Map & map, xml_node const& pt)
{
    std::string name("<missing name>");
    metawriter_ptr writer;
    try
    {
        name = pt.get_attr<std::string>("name");
        writer = metawriter_create(pt);
        map.insert_metawriter(name, writer);
    } catch (const config_error & ex) {
        ex.append_context(std::string("in meta writer '") + name + "'", pt);
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

        for (; itr != end; ++itr)
        {
            if (itr->is("Font"))
            {
                parse_font(fontset, *itr);
            }
        }

        map.insert_fontset(name, fontset);

        // XXX Hack because map object isn't accessible by text_symbolizer
        // when it's parsed
        fontsets_.insert(pair<std::string, font_set>(name, fontset));
    } catch (const config_error & ex) {
        ex.append_context(std::string("in FontSet '") + name + "'", fset);
        throw;
    }
}

void map_parser::parse_font(font_set &fset, xml_node const& f)
{
    optional<std::string> face_name = f.get_opt_attr<std::string>("face-name");
    if (face_name)
    {
        if (strict_)
        {
            ensure_font_face(*face_name);
        }
        fset.add_face_name(*face_name);
    }
    else
    {
        throw config_error("Must have 'face-name' set", f);
    }
}

void map_parser::parse_layer(Map & map, xml_node const& lay)
{
    std::string name;
    try
    {
        name = lay.get_attr("name", std::string("Unnamed"));

        // XXX if no projection is given inherit from map? [DS]
        std::string srs = lay.get_attr("srs", map.srs());

        layer lyr(name, srs);

        optional<boolean> status = lay.get_opt_attr<boolean>("status");
        if (status)
        {
            lyr.set_active(* status);
        }

        optional<double> min_zoom = lay.get_opt_attr<double>("minzoom");
        if (min_zoom)
        {
            lyr.set_min_zoom(* min_zoom);
        }


        optional<double> max_zoom = lay.get_opt_attr<double>("maxzoom");
        if (max_zoom)
        {
            lyr.set_max_zoom(* max_zoom);
        }

        optional<boolean> queryable = lay.get_opt_attr<boolean>("queryable");
        if (queryable)
        {
            lyr.set_queryable(* queryable);
        }

        optional<boolean> clear_cache =
            lay.get_opt_attr<boolean>("clear-label-cache");
        if (clear_cache)
        {
            lyr.set_clear_label_cache(* clear_cache);
        }

        optional<boolean> cache_features =
            lay.get_opt_attr<boolean>("cache-features");
        if (cache_features)
        {
            lyr.set_cache_features(* cache_features);
        }

        optional<std::string> group_by =
            lay.get_opt_attr<std::string>("group-by");
        if (group_by)
        {
            lyr.set_group_by(* group_by);
        }

        xml_node::const_iterator child = lay.begin();
        xml_node::const_iterator end = lay.end();

        for(; child != end; ++child)
        {

            if (child->is("StyleName"))
            {
                std::string style_name = child->get_text();
                if (style_name.empty())
                {
                    std::ostringstream ss;
                    ss << "StyleName is empty in Layer: '" << lyr.name() << "'";
                    if (strict_)
                        throw config_error(ss.str());
                    else
                        std::clog << "### WARNING: " << ss.str() << std::endl;
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
                        params = base_itr->second;
                }

                xml_node::const_iterator paramIter = child->begin();
                xml_node::const_iterator endParam = child->end();
                for (; paramIter != endParam; ++paramIter)
                {
                    if (paramIter->is("Parameter"))
                    {
                        std::string name = paramIter->get_attr<std::string>("name");
                        std::string value = paramIter->get_text();
                        params[name] = value;
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
                    boost::shared_ptr<datasource> ds =
                        datasource_cache::instance()->create(params);
                    lyr.set_datasource(ds);
                }

                catch (const std::exception & ex)
                {
                    throw config_error(ex.what());
                }

                catch (...)
                {
                    throw config_error("Unknown exception occured attempting to create datasoure for layer '" + lyr.name() + "'");
                }
            }
        }
        map.addLayer(lyr);
    }
    catch (const config_error & ex)
    {
        if (!name.empty())
        {
            ex.append_context(std::string(" encountered during parsing of layer '") + name + "'", lay);
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
        }
        style.add_rule(rule);

    }
    catch (const config_error & ex)
    {
        if (!name.empty())
        {
            ex.append_context(std::string("in rule '") + name + "'", r);
        }
        throw;
    }
}

void map_parser::parse_metawriter_in_symbolizer(symbolizer_base &sym, xml_node const &pt)
{
    optional<std::string> writer = pt.get_opt_attr<std::string>("meta-writer");
    if (!writer) return;
    optional<std::string> output = pt.get_opt_attr<std::string>("meta-output");
    sym.add_metawriter(*writer, output);
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
        optional<std::string> transform_wkt = sym.get_opt_attr<std::string>("transform");

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
            sym.get_attr<point_placement_e>("placement", CENTROID_POINT_PLACEMENT);
        symbol.set_point_placement(placement);

        if (file)
        {
            try
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

                symbol.set_filename(parse_path(*file));

                if (transform_wkt)
                {
                    agg::trans_affine tr;
                    if (!mapnik::svg::parse_transform((*transform_wkt).c_str(),tr))
                    {
                        std::stringstream ss;
                        ss << "Could not parse transform from '" << transform_wkt
                           << "', expected string like: 'matrix(1, 0, 0, 1, 0, 0)'";
                        if (strict_)
                            throw config_error(ss.str()); // value_error here?
                        else
                            std::clog << "### WARNING: " << ss << endl;
                    }
                    boost::array<double,6> matrix;
                    tr.store_to(&matrix[0]);
                    symbol.set_transform(matrix);
                }
            }
            catch (image_reader_exception const & ex)
            {
                std::string msg("Failed to load image file '" + * file +
                                "': " + ex.what());
                if (strict_)
                {
                    throw config_error(msg);
                }
                else
                {
                    std::clog << "### WARNING: " << msg << endl;
                }
            }
        }
        parse_metawriter_in_symbolizer(symbol, sym);
        rule.append(symbol);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in PointSymbolizer", sym);
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
        optional<std::string> transform_wkt = sym.get_opt_attr<std::string>("transform");

        if (file)
        {
            try
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
            catch (...)
            {
                std::string msg("Failed to load marker file '" + *file + "'!");
                if (strict_)
                {
                    throw config_error(msg);
                }
                else
                {
                    std::clog << "### WARNING: " << msg << endl;
                }
            }
        }

        markers_symbolizer symbol(parse_path(filename));
        optional<float> opacity = sym.get_opt_attr<float>("opacity");
        if (opacity) symbol.set_opacity(*opacity);

        if (transform_wkt)
        {
            agg::trans_affine tr;
            if (!mapnik::svg::parse_transform((*transform_wkt).c_str(),tr))
            {
                std::stringstream ss;
                ss << "Could not parse transform from '" << transform_wkt
                   << "', expected string like: 'matrix(1, 0, 0, 1, 0, 0)'";
                if (strict_)
                    throw config_error(ss.str()); // value_error here?
                else
                    std::clog << "### WARNING: " << ss << endl;
            }
            boost::array<double,6> matrix;
            tr.store_to(&matrix[0]);
            symbol.set_transform(matrix);
        }

        optional<color> c = sym.get_opt_attr<color>("fill");
        if (c) symbol.set_fill(*c);
        optional<double> spacing = sym.get_opt_attr<double>("spacing");
        if (spacing) symbol.set_spacing(*spacing);
        optional<double> max_error = sym.get_opt_attr<double>("max-error");
        if (max_error) symbol.set_max_error(*max_error);
        optional<boolean> allow_overlap = sym.get_opt_attr<boolean>("allow-overlap");
        optional<boolean> ignore_placement = sym.get_opt_attr<boolean>("ignore-placement");
        if (allow_overlap) symbol.set_allow_overlap(*allow_overlap);
        if (ignore_placement) symbol.set_ignore_placement(*ignore_placement);

        optional<double> w = sym.get_opt_attr<double>("width");
        optional<double> h = sym.get_opt_attr<double>("height");
        if (w && h)
        {
            symbol.set_width(*w);
            symbol.set_height(*h);
        }
        else if (w)
        {
            symbol.set_width(*w);
            symbol.set_height(*w);

        }
        else if (h)
        {
            symbol.set_width(*h);
            symbol.set_height(*h);
        }

        stroke strk;
        parse_stroke(strk,sym);
        symbol.set_stroke(strk);

        marker_placement_e placement = sym.get_attr<marker_placement_e>("placement", MARKER_LINE_PLACEMENT);
        symbol.set_marker_placement(placement);

        marker_type_e dfl_marker_type = ARROW;

        if (placement == MARKER_POINT_PLACEMENT)
            dfl_marker_type = ELLIPSE;

        marker_type_e marker_type = sym.get_attr<marker_type_e>("marker-type", dfl_marker_type);
        symbol.set_marker_type(marker_type);

        parse_metawriter_in_symbolizer(symbol, sym);
        rule.append(symbol);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in MarkersSymbolizer", sym);
        throw;
    }
}

void map_parser::parse_line_pattern_symbolizer(rule & rule, xml_node const & sym)
{
    try
    {
        std::string file = sym.get_attr<std::string>("file");
        optional<std::string> base = sym.get_opt_attr<std::string>("base");

        try
        {
            if(base)
            {
                std::map<std::string,std::string>::const_iterator itr = file_sources_.find(*base);
                if (itr!=file_sources_.end())
                {
                    file = itr->second + "/" + file;
                }
            }

            file = ensure_relative_to_xml(file);

            line_pattern_symbolizer symbol(parse_path(file));

            parse_metawriter_in_symbolizer(symbol, sym);
            rule.append(symbol);
        }
        catch (image_reader_exception const & ex)
        {
            std::string msg("Failed to load image file '" + file +
                            "': " + ex.what());
            if (strict_)
            {
                throw config_error(msg);
            }
            else
            {
                std::clog << "### WARNING: " << msg << endl;
            }
        }
    }
    catch (const config_error & ex)
    {
        ex.append_context("in LinePatternSymbolizer", sym);
        throw;
    }
}

void map_parser::parse_polygon_pattern_symbolizer(rule & rule,
                                                   xml_node const & sym)
{
    try
    {
        std::string file = sym.get_attr<std::string>("file");
        optional<std::string> base = sym.get_opt_attr<std::string>("base");

        try
        {
            if(base)
            {
                std::map<std::string,std::string>::iterator itr = file_sources_.find(*base);
                if (itr!=file_sources_.end())
                {
                    file = itr->second + "/" + file;
                }
            }

            file = ensure_relative_to_xml(file);

            polygon_pattern_symbolizer symbol(parse_path(file));

            // pattern alignment
            pattern_alignment_e p_alignment = sym.get_attr<pattern_alignment_e>("alignment",LOCAL_ALIGNMENT);
            symbol.set_alignment(p_alignment);

            // gamma
            optional<double> gamma = sym.get_opt_attr<double>("gamma");
            if (gamma)  symbol.set_gamma(*gamma);

            // gamma method
            optional<gamma_method_e> gamma_method = sym.get_opt_attr<gamma_method_e>("gamma-method");
            if (gamma_method) symbol.set_gamma_method(*gamma_method);

            parse_metawriter_in_symbolizer(symbol, sym);
            rule.append(symbol);
        }
        catch (image_reader_exception const & ex)
        {
            std::string msg("Failed to load image file '" + file +
                            "': " + ex.what());
            if (strict_)
            {
                throw config_error(msg);
            }
            else
            {
                std::clog << "### WARNING: " << msg << endl;
            }
        }
    }
    catch (const config_error & ex)
    {
        ex.append_context("in PolygonPatternSymbolizer", sym);
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
            placement_finder = placements::registry::instance()->from_xml(*placement_type, sym, fontsets_);
        } else {
            placement_finder = boost::make_shared<text_placements_dummy>();
            placement_finder->defaults.from_xml(sym, fontsets_);
        }
        if (strict_ &&
            !placement_finder->defaults.format.fontset.size())
            ensure_font_face(placement_finder->defaults.format.face_name);

        text_symbolizer text_symbol = text_symbolizer(placement_finder);
        parse_metawriter_in_symbolizer(text_symbol, sym);
        rule.append(text_symbol);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in TextSymbolizer", sym);
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
            placement_finder = placements::registry::instance()->from_xml(*placement_type, sym, fontsets_);
        } else {
            placement_finder = boost::make_shared<text_placements_dummy>();
        }
        placement_finder->defaults.from_xml(sym, fontsets_);
        if (strict_ &&
            !placement_finder->defaults.format.fontset.size())
            ensure_font_face(placement_finder->defaults.format.face_name);

        shield_symbolizer shield_symbol = shield_symbolizer(placement_finder);
        /* Symbolizer specific attributes. */
        optional<std::string> transform_wkt = sym.get_opt_attr<std::string>("transform");
        if (transform_wkt)
        {
            agg::trans_affine tr;
            if (!mapnik::svg::parse_transform((*transform_wkt).c_str(),tr))
            {
                std::stringstream ss;
                ss << "Could not parse transform from '" << transform_wkt << "', expected string like: 'matrix(1, 0, 0, 1, 0, 0)'";
                if (strict_)
                    throw config_error(ss.str()); // value_error here?
                else
                    std::clog << "### WARNING: " << ss << endl;
            }
            boost::array<double,6> matrix;
            tr.store_to(&matrix[0]);
            shield_symbol.set_transform(matrix);
        }
        // shield displacement
        double shield_dx = sym.get_attr("shield-dx", 0.0);
        double shield_dy = sym.get_attr("shield-dy", 0.0);
        shield_symbol.set_shield_displacement(shield_dx,shield_dy);

        // opacity
        optional<double> opacity = sym.get_opt_attr<double>("opacity");
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

        parse_metawriter_in_symbolizer(shield_symbol, sym);

        std::string image_file = sym.get_attr<std::string>("file");
        optional<std::string> base = sym.get_opt_attr<std::string>("base");

        try
        {
            if(base)
            {
                std::map<std::string,std::string>::const_iterator itr = file_sources_.find(*base);
                if (itr!=file_sources_.end())
                {
                    image_file = itr->second + "/" + image_file;
                }
            }

            image_file = ensure_relative_to_xml(image_file);
            shield_symbol.set_filename(parse_path(image_file));
        }
        catch (image_reader_exception const & ex)
        {
            std::string msg("Failed to load image file '" + image_file +
                            "': " + ex.what());
            if (strict_)
            {
                throw config_error(msg);
            }
            else
            {
                std::clog << "### WARNING: " << msg << endl;
            }
        }
        rule.append(shield_symbol);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in ShieldSymbolizer", sym);
        throw;
    }
}

void map_parser::parse_stroke(stroke & strk, xml_node const & sym)
{
    // stroke color
    optional<color> c = sym.get_opt_attr<color>("stroke");
    if (c) strk.set_color(*c);

    // stroke-width
    optional<double> width =  sym.get_opt_attr<double>("stroke-width");
    if (width) strk.set_width(*width);

    // stroke-opacity
    optional<double> opacity = sym.get_opt_attr<double>("stroke-opacity");
    if (opacity) strk.set_opacity(*opacity);

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
        tokenizer<> tok (*str);
        std::vector<double> dash_array;
        tokenizer<>::iterator itr = tok.begin();
        for (; itr != tok.end(); ++itr)
        {
            try
            {
                double f = boost::lexical_cast<double>(*itr);
                dash_array.push_back(f);
            }
            catch (boost::bad_lexical_cast &)
            {
                throw config_error(std::string("Failed to parse dasharray ") +
                                   "'. Expected a " +
                                   "list of floats but got '" + (*str) + "'");
            }
        }
        if (dash_array.size())
        {
            size_t size = dash_array.size();
            if (size % 2)
            {
                for (size_t i=0; i < size ;++i)
                {
                    dash_array.push_back(dash_array[i]);
                }
            }
            std::vector<double>::const_iterator pos = dash_array.begin();
            while (pos != dash_array.end())
            {
                strk.add_dash(*pos,*(pos + 1));
                pos +=2;
            }
        }
    }
}

void map_parser::parse_line_symbolizer(rule & rule, xml_node const & sym)
{
    try
    {
        stroke strk;
        parse_stroke(strk,sym);
        line_symbolizer symbol = line_symbolizer(strk);

        // rasterizer method
        line_rasterizer_e rasterizer = sym.get_attr<line_rasterizer_e>("rasterizer", RASTERIZER_FULL);
        //optional<line_rasterizer_e> rasterizer_method = sym.get_opt_attr<line_rasterizer_e>("full");
        symbol.set_rasterizer(rasterizer);
        // smooth value
        optional<double> smooth = sym.get_opt_attr<double>("smooth");
        if (smooth) symbol.set_smooth(*smooth);
        // meta-writer
        parse_metawriter_in_symbolizer(symbol, sym);
        rule.append(symbol);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in LineSymbolizer", sym);
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
        // smooth value
        optional<double> smooth = sym.get_opt_attr<double>("smooth");
        if (smooth) poly_sym.set_smooth(*smooth);
        
        parse_metawriter_in_symbolizer(poly_sym, sym);
        rule.append(poly_sym);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in PolygonSymbolizer", sym);
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

        parse_metawriter_in_symbolizer(building_sym, sym);
        rule.append(building_sym);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in BuildingSymbolizer", sym);
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
        if (mode) raster_sym.set_mode(*mode);

        // scaling
        optional<std::string> scaling = sym.get_opt_attr<std::string>("scaling");
        if (scaling) raster_sym.set_scaling(*scaling);

        // opacity
        optional<double> opacity = sym.get_opt_attr<double>("opacity");
        if (opacity) raster_sym.set_opacity(*opacity);

        // filter factor
        optional<double> filter_factor = sym.get_opt_attr<double>("filter-factor");
        if (filter_factor) raster_sym.set_filter_factor(*filter_factor);

        // mesh-size
        optional<unsigned> mesh_size = sym.get_opt_attr<unsigned>("mesh-size");
        if (mesh_size) raster_sym.set_mesh_size(*mesh_size);


        xml_node::const_iterator cssIter = sym.begin();
        xml_node::const_iterator endCss = sym.end();

        for(; cssIter != endCss; ++cssIter)
        {
            if (cssIter->is("RasterColorizer"))
            {
                raster_colorizer_ptr colorizer = boost::make_shared<raster_colorizer>();
                raster_sym.set_colorizer(colorizer);
                parse_raster_colorizer(colorizer, *cssIter);
            }
        }
        //Note: raster_symbolizer doesn't support metawriters
        rule.append(raster_sym);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in RasterSymbolizer", sym);
        throw;
    }
}

void map_parser::parse_raster_colorizer(raster_colorizer_ptr const& rc,
                                        xml_node const& node)
{
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
    catch (const config_error & ex)
    {
        ex.append_context("in RasterColorizer", node);
        throw;
    }
}

void map_parser::ensure_font_face(std::string const& face_name)
{
    if (! font_manager_.get_face(face_name))
    {
        throw config_error("Failed to find font face '" +
                           face_name + "'");
    }
}

std::string map_parser::ensure_relative_to_xml(boost::optional<std::string> opt_path)
{
    if (relative_to_xml_)
    {
        boost::filesystem::path xml_path = filename_;
        boost::filesystem::path rel_path = *opt_path;
        if (!rel_path.has_root_path())
        {
#if (BOOST_FILESYSTEM_VERSION == 3)
            // TODO - normalize is now deprecated, use make_preferred?
            boost::filesystem::path full = boost::filesystem::absolute(xml_path.parent_path()/rel_path);
#else // v2
            boost::filesystem::path full = boost::filesystem::complete(xml_path.branch_path()/rel_path).normalize();
#endif

#ifdef MAPNIK_DEBUG
            std::clog << "\nModifying relative paths to be relative to xml...\n";
            std::clog << "original base path: " << *opt_path << "\n";
            std::clog << "relative base path: " << full.string() << "\n";
#endif
            return full.string();
        }
    }
    return *opt_path;
}

void map_parser::find_unused_nodes(xml_node const& root)
{
    std::stringstream error_message;
    find_unused_nodes_recursive(root, error_message);
    if (!error_message.str().empty())
    {
        throw config_error("The following nodes or attributes were not processed while parsing the xml file:" + error_message.str());
    }
}

void map_parser::find_unused_nodes_recursive(xml_node const& node, std::stringstream &error_message)
{
    if (!node.processed())
    {
        if (node.is_text()) {
            error_message << "\n* text '" << node.text() << "'";
        } else {
            error_message << "\n* node '" << node.name() << "' in line " << node.line();
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
            error_message << "\n* attribute '" << aitr->first <<
                "' with value '" << aitr->second.value <<
                "' in line " << node.line();
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
