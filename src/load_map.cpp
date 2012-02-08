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

#include <mapnik/ptree_helpers.hpp>
#ifdef HAVE_LIBXML2
#include <mapnik/libxml2_loader.hpp>
#endif

#include <mapnik/filter_factory.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/raster_colorizer.hpp>

#include <mapnik/svg/svg_path_parser.hpp>

#include <mapnik/metawriter_factory.hpp>

#include <mapnik/text_placements_simple.hpp>
#include <mapnik/text_placements_list.hpp>
#include <mapnik/text_processing.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/rule.hpp>

// boost
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
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

#ifdef _WINDOWS
#include <Windows.h>
#endif

using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::tokenizer;
using boost::property_tree::ptree;

using std::cerr;
using std::endl;

namespace mapnik
{
using boost::optional;

class map_parser : boost::noncopyable {
public:
    map_parser( bool strict, std::string const& filename = "" ) :
        strict_( strict ),
        filename_( filename ),
        relative_to_xml_(true),
        font_manager_(font_engine_) {}

    void parse_map(Map & map, ptree const & sty, std::string const& base_path="");
private:
    void parse_map_include( Map & map, ptree const & include);
    void parse_style(Map & map, ptree const & sty);
    void parse_layer(Map & map, ptree const & lay);
    void parse_metawriter(Map & map, ptree const & lay);
    void parse_metawriter_in_symbolizer(symbolizer_base &sym, ptree const &pt);

    void parse_fontset(Map & map, ptree const & fset);
    void parse_font(font_set & fset, ptree const & f);

    void parse_rule(feature_type_style & style, ptree const & r);

    void parse_point_symbolizer(rule & rule, ptree const & sym);
    void parse_line_pattern_symbolizer(rule & rule, ptree const & sym);
    void parse_polygon_pattern_symbolizer(rule & rule, ptree const & sym);
    void parse_text_symbolizer(rule & rule, ptree const & sym);
    void parse_shield_symbolizer(rule & rule, ptree const & sym);
    void parse_line_symbolizer(rule & rule, ptree const & sym);
    void parse_polygon_symbolizer(rule & rule, ptree const & sym);
    void parse_building_symbolizer(rule & rule, ptree const & sym );
    void parse_raster_symbolizer(rule & rule, ptree const & sym );
    void parse_markers_symbolizer(rule & rule, ptree const & sym );

    void parse_raster_colorizer(raster_colorizer_ptr const& rc, ptree const& node );
    void parse_stroke(stroke & strk, ptree const & sym);

    void ensure_font_face( const std::string & face_name );

    std::string ensure_relative_to_xml( boost::optional<std::string> opt_path );
    void ensure_attrs( ptree const& sym, std::string name, std::string attrs);

    bool strict_;
    std::string filename_;
    bool relative_to_xml_;
    std::map<std::string,parameters> datasource_templates_;
    freetype_engine font_engine_;
    face_manager<freetype_engine> font_manager_;
    std::map<std::string,std::string> file_sources_;
    std::map<std::string,font_set> fontsets_;

};

#ifdef _WINDOWS
std::string wstring2string(const std::wstring& s)
{
    int slength = (int)s.length() + 1;
    int len = ::WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
    boost::scoped_array<char> buf_ptr(new char [len+1]);
    ::WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf_ptr.get(), len, 0, 0);
    std::string r(buf_ptr.get());
    return r;
}

std::wstring utf8ToWide( const std::string& str )
{
    int len = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, 0, 0);
    boost::scoped_array<wchar_t> buf_ptr(new wchar_t [len+1]);
    ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buf_ptr.get(), len);
    std::wstring rt(buf_ptr.get());
    return rt;
}
#endif

void load_map(Map & map, std::string const& filename, bool strict)
{
    ptree pt;
#ifdef HAVE_LIBXML2
    read_xml2(filename, pt);
#else
    try
    {
        read_xml(filename, pt);
    }
    catch (const boost::property_tree::xml_parser_error & ex)
    {
        throw config_error( ex.what() );
    }
#endif
    map_parser parser( strict, filename);
    parser.parse_map(map, pt);
}

void load_map_string(Map & map, std::string const& str, bool strict, std::string const& base_path)
{
    ptree pt;
#ifdef HAVE_LIBXML2
    if (!base_path.empty())
        read_xml2_string(str, pt, base_path); // accept base_path passed into function
    else
        read_xml2_string(str, pt, map.base_path()); // default to map base_path
#else
    try
    {
        std::istringstream s(str);
        // TODO - support base_path?
        read_xml(s,pt);
    }
    catch (const boost::property_tree::xml_parser_error & ex)
    {
        throw config_error( ex.what() ) ;
    }
#endif

    map_parser parser( strict, base_path);
    parser.parse_map(map, pt, base_path);
}

void map_parser::parse_map( Map & map, ptree const & pt, std::string const& base_path )
{
    try
    {
        ptree const & map_node = pt.get_child("Map");

        std::ostringstream s("");
        s << "background-color,"
          << "background-image,"
          << "srs,"
          << "buffer-size,"
          << "paths-from-xml,"
          << "minimum-version,"
          << "font-directory,"
          << "maximum-extent,"
          << "base";
        ensure_attrs(map_node, "Map", s.str());

        try
        {
            parameters extra_attr;

            // Check if relative paths should be interpreted as relative to/from XML location
            // Default is true, and map_parser::ensure_relative_to_xml will be called to modify path
            optional<boolean> paths_from_xml = get_opt_attr<boolean>(map_node, "paths-from-xml");
            if (paths_from_xml)
            {
                relative_to_xml_ = *paths_from_xml;
            }

            optional<std::string> base_path_from_xml = get_opt_attr<std::string>(map_node, "base");
            if (!base_path.empty())
            {
                map.set_base_path( base_path );
            }
            else if (base_path_from_xml)
            {
                map.set_base_path( *base_path_from_xml );
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

                map.set_base_path( base );
            }

            optional<color> bgcolor = get_opt_attr<color>(map_node, "background-color");
            if (bgcolor)
            {
                map.set_background( * bgcolor );
            }

            optional<std::string> image_filename = get_opt_attr<std::string>(map_node, "background-image");
            if (image_filename)
            {
                map.set_background_image(ensure_relative_to_xml(image_filename));
            }

            map.set_srs( get_attr(map_node, "srs", map.srs() ));

            optional<unsigned> buffer_size = get_opt_attr<unsigned>(map_node,"buffer-size");
            if (buffer_size)
            {
                map.set_buffer_size(*buffer_size);
            }

            optional<std::string> maximum_extent = get_opt_attr<std::string>(map_node,"maximum-extent");
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
                    s << "failed to parse 'maximum-extent'";
                    if ( strict_ )
                        throw config_error(s_err.str());
                    else
                        std::clog << "### WARNING: " << s.str() << std::endl;
                }
            }

            optional<std::string> font_directory = get_opt_attr<std::string>(map_node,"font-directory");
            if (font_directory)
            {
                extra_attr["font-directory"] = *font_directory;
                freetype_engine::register_fonts( ensure_relative_to_xml(font_directory), false);
            }

            optional<std::string> min_version_string = get_opt_attr<std::string>(map_node, "minimum-version");

            if (min_version_string)
            {
                extra_attr["minimum-version"] = *min_version_string;
                boost::char_separator<char> sep(".");
                boost::tokenizer<boost::char_separator<char> > tokens(*min_version_string,sep);
                unsigned i = 0;
                bool success = false;
                int n[3];
                for (boost::tokenizer<boost::char_separator<char> >::iterator beg=tokens.begin();
                     beg!=tokens.end();++beg)
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
            ex.append_context("(in node Map)");
            throw;
        }

        parse_map_include( map, map_node );
    }
    catch (const boost::property_tree::ptree_bad_path &)
    {
        throw config_error("Not a map file. Node 'Map' not found.");
    }
}

void map_parser::parse_map_include( Map & map, ptree const & include )
{
    ptree::const_iterator itr = include.begin();
    ptree::const_iterator end = include.end();

    for (; itr != end; ++itr)
    {
        ptree::value_type const& v = *itr;

        if (v.first == "Include")
        {
            parse_map_include( map, v.second );
        }
        else if (v.first == "Style")
        {
            parse_style( map, v.second );
        }
        else if (v.first == "Layer")
        {
            parse_layer(map, v.second );
        }
        else if (v.first == "FontSet")
        {
            parse_fontset(map, v.second);
        }
        else if (v.first == "MetaWriter")
        {
            parse_metawriter(map, v.second);
        }
        else if (v.first == "FileSource")
        {
            std::string name = get_attr<std::string>( v.second, "name");
            std::string value = get_value<std::string>( v.second, "");
            file_sources_[name] = value;
        }
        else if (v.first == "Datasource")
        {
            std::string name = get_attr(v.second, "name", std::string("Unnamed"));
            parameters params;
            ptree::const_iterator paramIter = v.second.begin();
            ptree::const_iterator endParam = v.second.end();
            for (; paramIter != endParam; ++paramIter)
            {
                ptree const& param = paramIter->second;

                if (paramIter->first == "Parameter")
                {
                    std::string name = get_attr<std::string>(param, "name");
                    std::string value = get_value<std::string>( param,
                                                                "datasource parameter");
                    params[name] = value;
                }
                else if( paramIter->first != "<xmlattr>" &&
                         paramIter->first != "<xmlcomment>" )
                {
                    throw config_error(std::string("Unknown child node in ") +
                                       "'Datasource'. Expected 'Parameter' but got '" +
                                       paramIter->first + "'");
                }
            }
            datasource_templates_[name] = params;
        }
        else if (v.first == "Parameters")
        {
            std::string name = get_attr(v.second, "name", std::string("Unnamed"));
            parameters & params = map.get_extra_parameters();
            ptree::const_iterator paramIter = v.second.begin();
            ptree::const_iterator endParam = v.second.end();
            for (; paramIter != endParam; ++paramIter)
            {
                ptree const& param = paramIter->second;

                if (paramIter->first == "Parameter")
                {
                    std::string name = get_attr<std::string>(param, "name");
                    bool is_string = true;
                    boost::optional<std::string> type = get_opt_attr<std::string>(param, "type");
                    if (type)
                    {
                        if (*type == "int")
                        {
                            is_string = false;
                            int value = get_value<int>( param,"parameter");
                            params[name] = value;
                        }
                        else if (*type == "float")
                        {
                            is_string = false;
                            double value = get_value<double>( param,"parameter");
                            params[name] = value;
                        }
                    }

                    if (is_string)
                    {
                        std::string value = get_value<std::string>( param,
                                                                    "parameter");
                        params[name] = value;
                    }
                }
                else if( paramIter->first != "<xmlattr>" &&
                         paramIter->first != "<xmlcomment>" )
                {
                    throw config_error(std::string("Unknown child node in ") +
                                       "'Parameters'. Expected 'Parameter' but got '" +
                                       paramIter->first + "'");
                }
            }
        }
        else if (v.first != "<xmlcomment>" &&
                 v.first != "<xmlattr>")
        {
            throw config_error(std::string("Unknown child node in 'Map': '") +
                               v.first + "'");
        }
    }


    map.init_metawriters();
}

void map_parser::parse_style( Map & map, ptree const & sty )
{
    std::ostringstream s("");
    s << "name,"
      << "filter-mode";
    ensure_attrs(sty, "Style", s.str());

    std::string name("<missing name>");
    try
    {
        name = get_attr<std::string>(sty, "name");
        feature_type_style style;

        filter_mode_e filter_mode = get_attr<filter_mode_e>(sty, "filter-mode", FILTER_ALL);
        style.set_filter_mode(filter_mode);

        ptree::const_iterator ruleIter = sty.begin();
        ptree::const_iterator endRule = sty.end();

        for (; ruleIter!=endRule; ++ruleIter)
        {
            ptree::value_type const& rule_tag = *ruleIter;
            if (rule_tag.first == "Rule")
            {
                parse_rule( style, rule_tag.second );
            }
            else if (rule_tag.first != "<xmlcomment>" &&
                     rule_tag.first != "<xmlattr>" )
            {
                throw config_error(std::string("Unknown child node in 'Style'. ") +
                                   "Expected 'Rule' but got '" + rule_tag.first + "'");
            }
        }

        map.insert_style(name, style);

    } catch (const config_error & ex) {
        if ( ! name.empty() ) {
            ex.append_context(std::string("in style '") + name + "'");
        }
        ex.append_context(std::string("in map '") + filename_ + "'");
        throw;
    }
}

void map_parser::parse_metawriter(Map & map, ptree const & pt)
{
    ensure_attrs(pt, "MetaWriter", "name,type,file,default-output,output-empty,pixel-coordinates");
    std::string name("<missing name>");
    metawriter_ptr writer;
    try
    {
        name = get_attr<std::string>(pt, "name");
        writer = metawriter_create(pt);
        map.insert_metawriter(name, writer);

    } catch (const config_error & ex) {
        if (!name.empty()) {
            ex.append_context(std::string("in meta writer '") + name + "'");
        }
        ex.append_context(std::string("in map '") + filename_ + "'");
        throw;
    }
}

void map_parser::parse_fontset( Map & map, ptree const & fset )
{
    ensure_attrs(fset, "FontSet", "name,Font");
    std::string name("<missing name>");
    try
    {
        name = get_attr<std::string>(fset, "name");
        font_set fontset(name);

        ptree::const_iterator itr = fset.begin();
        ptree::const_iterator end = fset.end();

        for (; itr != end; ++itr)
        {
            ptree::value_type const& font_tag = *itr;

            if (font_tag.first == "Font")
            {
                parse_font(fontset, font_tag.second);
            }
            else if (font_tag.first != "<xmlcomment>" &&
                     font_tag.first != "<xmlattr>" )
            {
                throw config_error(std::string("Unknown child node in 'FontSet'. ") +
                                   "Expected 'Font' but got '" + font_tag.first + "'");
            }
        }

        map.insert_fontset(name, fontset);

        // XXX Hack because map object isn't accessible by text_symbolizer
        // when it's parsed
        fontsets_.insert(pair<std::string, font_set>(name, fontset));
    } catch (const config_error & ex) {
        if ( ! name.empty() ) {
            ex.append_context(std::string("in FontSet '") + name + "'");
        }
        ex.append_context(std::string("in map '") + filename_ + "'");
        throw;
    }
}

void map_parser::parse_font(font_set & fset, ptree const & f)
{
    ensure_attrs(f, "Font", "face-name");

    optional<std::string> face_name = get_opt_attr<std::string>(f, "face-name");
    if (face_name)
    {
        if ( strict_ )
        {
            ensure_font_face(*face_name);
        }
        fset.add_face_name(*face_name);
    }
    else
    {
        throw config_error(std::string("Must have 'face-name' set"));
    }
}

void map_parser::parse_layer( Map & map, ptree const & lay )
{
    std::string name;
    std::ostringstream s("");
    s << "name,"
      << "srs,"
      << "status,"
      << "minzoom,"
      << "maxzoom,"
      << "queryable,"
      << "clear-label-cache,"
      << "cache-features,"
      << "group-by";
    ensure_attrs(lay, "Layer", s.str());
    try
    {
        name = get_attr(lay, "name", std::string("Unnamed"));

        // XXX if no projection is given inherit from map? [DS]
        std::string srs = get_attr(lay, "srs", map.srs());

        layer lyr(name, srs);

        optional<boolean> status = get_opt_attr<boolean>(lay, "status");
        if (status)
        {
            lyr.setActive( * status );
        }

        optional<double> minZoom = get_opt_attr<double>(lay, "minzoom");
        if (minZoom)
        {
            lyr.setMinZoom( * minZoom );
        }

        optional<double> maxZoom = get_opt_attr<double>(lay, "maxzoom");
        if (maxZoom)
        {
            lyr.setMaxZoom( * maxZoom );
        }

        optional<boolean> queryable = get_opt_attr<boolean>(lay, "queryable");
        if (queryable)
        {
            lyr.setQueryable( * queryable );
        }

        optional<boolean> clear_cache =
            get_opt_attr<boolean>(lay, "clear-label-cache");
        if (clear_cache)
        {
            lyr.set_clear_label_cache( * clear_cache );
        }

        optional<boolean> cache_features =
            get_opt_attr<boolean>(lay, "cache-features");
        if (cache_features)
        {
            lyr.set_cache_features( * cache_features );
        }

        optional<std::string> group_by =
            get_opt_attr<std::string>(lay, "group-by");
        if (group_by)
        {
            lyr.set_group_by( * group_by );
        }

        ptree::const_iterator itr2 = lay.begin();
        ptree::const_iterator end2 = lay.end();

        for(; itr2 != end2; ++itr2)
        {
            ptree::value_type const& child = *itr2;

            if (child.first == "StyleName")
            {
                ensure_attrs(child.second, "StyleName", "none");
                std::string style_name = get_value<std::string>(child.second, "style name");
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
            else if (child.first == "Datasource")
            {
                ensure_attrs(child.second, "Datasource", "base");
                parameters params;
                optional<std::string> base = get_opt_attr<std::string>( child.second, "base" );
                if( base )
                {
                    std::map<std::string,parameters>::const_iterator base_itr = datasource_templates_.find(*base);
                    if (base_itr!=datasource_templates_.end())
                        params = base_itr->second;
                }

                ptree::const_iterator paramIter = child.second.begin();
                ptree::const_iterator endParam = child.second.end();
                for (; paramIter != endParam; ++paramIter)
                {
                    ptree const& param = paramIter->second;

                    if (paramIter->first == "Parameter")
                    {
                        ensure_attrs(param, "Parameter", "name");
                        std::string name = get_attr<std::string>(param, "name");
                        std::string value = get_value<std::string>( param,
                                                                    "datasource parameter");
#ifdef _WINDOWS
                        if (name == "file")
                        {
                            params[name] = wstring2string(utf8ToWide(value));
                        }
                        else
                        {
                            params[name] = value;
                        }
#else
                        params[name] = value;
#endif
                    }
                    else if( paramIter->first != "<xmlattr>"  &&
                             paramIter->first != "<xmlcomment>" )
                    {
                        throw config_error(std::string("Unknown child node in ") +
                                           "'Datasource'. Expected 'Parameter' but got '" +
                                           paramIter->first + "'");
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

                catch (const std::exception & ex )
                {
                    throw config_error( ex.what() );
                }

                catch (...)
                {
                    throw config_error("Unknown exception occured attempting to create datasoure for layer '" + lyr.name() + "'");
                }
            }
            else if (child.first != "<xmlattr>" &&
                     child.first != "<xmlcomment>")
            {
                throw config_error(std::string("Unknown child node in 'Layer'. ") +
                                   "Expected 'StyleName' or 'Datasource' but got '" +
                                   child.first + "'");
            }
        }

        map.addLayer(lyr);

    }
    catch (const config_error & ex)
    {
        if ( ! name.empty() )
        {
            ex.append_context(std::string("(encountered during parsing of layer '") + name + "' in map '" + filename_ + "')");
        }
        throw;
    }
}

void map_parser::parse_rule( feature_type_style & style, ptree const & r )
{
    ensure_attrs(r, "Rule", "name");
    std::string name;
    try
    {
        name = get_attr( r, "name", std::string());
        rule rule(name);

        optional<std::string> filter_expr =
            get_opt_child<std::string>( r, "Filter");
        if (filter_expr)
        {
            // TODO - can we use encoding defined for XML document for filter expressions?
            rule.set_filter(parse_expression(*filter_expr,"utf8"));
        }

        optional<std::string> else_filter =
            get_opt_child<std::string>(r, "ElseFilter");
        if (else_filter)
        {
            rule.set_else(true);
        }

        optional<std::string> also_filter =
            get_opt_child<std::string>(r, "AlsoFilter");
        if (also_filter)
        {
            rule.set_also(true);
        }

        optional<double> min_scale =
            get_opt_child<double>(r, "MinScaleDenominator");
        if (min_scale)
        {
            rule.set_min_scale(*min_scale);
        }

        optional<double> max_scale =
            get_opt_child<double>(r, "MaxScaleDenominator");
        if (max_scale)
        {
            rule.set_max_scale(*max_scale);
        }

        ptree::const_iterator symIter = r.begin();
        ptree::const_iterator endSym = r.end();

        for( ;symIter != endSym; ++symIter)
        {
            ptree::value_type const& sym = *symIter;

            if ( sym.first == "PointSymbolizer")
            {
                parse_point_symbolizer( rule, sym.second );
            }
            else if ( sym.first == "LinePatternSymbolizer")
            {
                parse_line_pattern_symbolizer( rule, sym.second );
            }
            else if ( sym.first == "PolygonPatternSymbolizer")
            {
                parse_polygon_pattern_symbolizer( rule, sym.second );
            }
            else if ( sym.first == "TextSymbolizer")
            {
                parse_text_symbolizer( rule, sym.second );
            }
            else if ( sym.first == "ShieldSymbolizer")
            {
                parse_shield_symbolizer( rule, sym.second );
            }
            else if ( sym.first == "LineSymbolizer")
            {
                parse_line_symbolizer( rule, sym.second );
            }
            else if ( sym.first == "PolygonSymbolizer")
            {
                parse_polygon_symbolizer( rule, sym.second );
            }
            else if ( sym.first == "BuildingSymbolizer")
            {
                parse_building_symbolizer( rule, sym.second );
            }
            else if ( sym.first == "RasterSymbolizer")
            {
                parse_raster_symbolizer( rule, sym.second );
            }
            else if ( sym.first == "MarkersSymbolizer")
            {
                parse_markers_symbolizer(rule, sym.second);
            }

            else if ( sym.first != "MinScaleDenominator" &&
                      sym.first != "MaxScaleDenominator" &&
                      sym.first != "Filter" &&
                      sym.first != "ElseFilter" &&
                      sym.first != "AlsoFilter" &&
                      sym.first != "<xmlcomment>" &&
                      sym.first != "<xmlattr>" )
            {
                throw config_error(std::string("Unknown symbolizer '") +
                                   sym.first + "'");
            }
        }

        style.add_rule(rule);

    }
    catch (const config_error & ex)
    {
        if ( ! name.empty() )
        {
            ex.append_context(std::string("in rule '") + name + "' in map '" + filename_ + "')");
        }
        throw;
    }
}

void map_parser::parse_metawriter_in_symbolizer(symbolizer_base &sym, ptree const &pt)
{
    optional<std::string> writer =  get_opt_attr<std::string>(pt, "meta-writer");
    if (!writer) return;
    optional<std::string> output =  get_opt_attr<std::string>(pt, "meta-output");
    sym.add_metawriter(*writer, output);
}

void map_parser::parse_point_symbolizer( rule & rule, ptree const & sym )
{
    try
    {
        std::stringstream s;
        s << "file,base,allow-overlap,ignore-placement,opacity,placement,transform,meta-writer,meta-output";

        optional<std::string> file =  get_opt_attr<std::string>(sym, "file");
        optional<std::string> base =  get_opt_attr<std::string>(sym, "base");
        optional<boolean> allow_overlap =
            get_opt_attr<boolean>(sym, "allow-overlap");
        optional<boolean> ignore_placement =
            get_opt_attr<boolean>(sym, "ignore-placement");
        optional<float> opacity =
            get_opt_attr<float>(sym, "opacity");

        optional<std::string> transform_wkt = get_opt_attr<std::string>(sym, "transform");

        if (file)
        {
            ensure_attrs(sym, "PointSymbolizer", s.str());
            try
            {
                if( base )
                {
                    std::map<std::string,std::string>::const_iterator itr = file_sources_.find(*base);
                    if (itr!=file_sources_.end())
                    {
                        *file = itr->second + "/" + *file;
                    }
                }

                *file = ensure_relative_to_xml(file);

                point_symbolizer symbol(parse_path(*file));

                if (allow_overlap)
                {
                    symbol.set_allow_overlap( * allow_overlap );
                }
                if (opacity)
                {
                    symbol.set_opacity( * opacity );
                }
                if (ignore_placement)
                {
                    symbol.set_ignore_placement( * ignore_placement );
                }
                point_placement_e placement =
                    get_attr<point_placement_e>(sym, "placement", CENTROID_POINT_PLACEMENT);
                symbol.set_point_placement( placement );

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

                parse_metawriter_in_symbolizer(symbol, sym);
                rule.append(symbol);
            }
            catch (image_reader_exception const & ex )
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
        else
        {
            ensure_attrs(sym, "PointSymbolizer", s.str());
            point_symbolizer symbol;

            if (allow_overlap)
            {
                symbol.set_allow_overlap( * allow_overlap );
            }
            if (opacity)
            {
                symbol.set_opacity( * opacity );
            }
            if (ignore_placement)
            {
                symbol.set_ignore_placement( * ignore_placement );
            }
            point_placement_e placement =
                get_attr<point_placement_e>(sym, "placement", CENTROID_POINT_PLACEMENT);
            symbol.set_point_placement( placement );

            parse_metawriter_in_symbolizer(symbol, sym);
            rule.append(symbol);
        }
    }
    catch (const config_error & ex)
    {
        ex.append_context("in PointSymbolizer");
        throw;
    }
}


void map_parser::parse_markers_symbolizer( rule & rule, ptree const & sym )
{
    try
    {
        std::string filename("");
        optional<std::string> file =  get_opt_attr<std::string>(sym, "file");
        optional<std::string> base =  get_opt_attr<std::string>(sym, "base");
        optional<std::string> transform_wkt = get_opt_attr<std::string>(sym, "transform");

        std::stringstream s;
        //s << "file,opacity,spacing,max-error,allow-overlap,placement,";
        s << "file,base,transform,fill,opacity,"
          << "spacing,max-error,allow-overlap,"
          << "width,height,placement,marker-type,"
          << "stroke,stroke-width,stroke-opacity,stroke-linejoin,"
          << "stroke-linecap,stroke-dashoffset,stroke-dasharray,"
            // note: stroke-gamma intentionally left off here as markers do not support them
          << "meta-writer,meta-output";
        ensure_attrs(sym, "MarkersSymbolizer", s.str());

        if (file)
        {
            //s << "base,transform";
            //ensure_attrs(sym, "MarkersSymbolizer", s.str());
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
        /*else
          {
          //s << "fill,marker-type,width,height";
          //ensure_attrs(sym, "MarkersSymbolizer", s.str());
          }*/

        markers_symbolizer symbol(parse_path(filename));
        optional<float> opacity = get_opt_attr<float>(sym, "opacity");
        if (opacity) symbol.set_opacity( *opacity );

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

        optional<color> c = get_opt_attr<color>(sym, "fill");
        if (c) symbol.set_fill(*c);
        optional<double> spacing = get_opt_attr<double>(sym, "spacing");
        if (spacing) symbol.set_spacing(*spacing);
        optional<double> max_error = get_opt_attr<double>(sym, "max-error");
        if (max_error) symbol.set_max_error(*max_error);
        optional<boolean> allow_overlap = get_opt_attr<boolean>(sym, "allow-overlap");
        if (allow_overlap) symbol.set_allow_overlap(*allow_overlap);

        optional<double> w = get_opt_attr<double>(sym, "width");
        optional<double> h = get_opt_attr<double>(sym, "height");
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

        marker_placement_e placement = get_attr<marker_placement_e>(sym, "placement", MARKER_LINE_PLACEMENT);
        symbol.set_marker_placement( placement );

        marker_type_e dfl_marker_type = ARROW;

        if (placement == MARKER_POINT_PLACEMENT)
            dfl_marker_type = ELLIPSE;

        marker_type_e marker_type = get_attr<marker_type_e>(sym, "marker-type", dfl_marker_type);
        symbol.set_marker_type( marker_type );

        parse_metawriter_in_symbolizer(symbol, sym);
        rule.append(symbol);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in MarkersSymbolizer");
        throw;
    }
}

void map_parser::parse_line_pattern_symbolizer( rule & rule, ptree const & sym )
{
    ensure_attrs(sym, "LinePatternSymbolizer", "file,base,meta-writer,meta-output");
    try
    {
        std::string file = get_attr<std::string>(sym, "file");
        optional<std::string> base = get_opt_attr<std::string>(sym, "base");

        try
        {
            if( base )
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
        catch (image_reader_exception const & ex )
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
        ex.append_context("in LinePatternSymbolizer");
        throw;
    }
}

void map_parser::parse_polygon_pattern_symbolizer( rule & rule,
                                                   ptree const & sym )
{
    ensure_attrs(sym, "PolygonPatternSymbolizer", "file,base,alignment,gamma,meta-writer,meta-output");
    try
    {
        std::string file = get_attr<std::string>(sym, "file");
        optional<std::string> base = get_opt_attr<std::string>(sym, "base");

        try
        {
            if( base )
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
            pattern_alignment_e p_alignment = get_attr<pattern_alignment_e>(sym, "alignment",LOCAL_ALIGNMENT);
            symbol.set_alignment(p_alignment);

            // gamma
            optional<double> gamma = get_opt_attr<double>(sym, "gamma");
            if (gamma)  symbol.set_gamma(*gamma);

            // gamma method
            optional<gamma_method_e> gamma_method = get_opt_attr<gamma_method_e>(sym, "gamma-method");
            if (gamma_method) symbol.set_gamma_method(*gamma_method);

            parse_metawriter_in_symbolizer(symbol, sym);
            rule.append(symbol);
        }
        catch (image_reader_exception const & ex )
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
        ex.append_context("in PolygonPatternSymbolizer");
        throw;
    }
}

void map_parser::parse_text_symbolizer( rule & rule, ptree const & sym )
{
    std::stringstream s_common;
    s_common << "name,face-name,fontset-name,size,fill,orientation,"
             << "dx,dy,placement,vertical-alignment,halo-fill,"
             << "halo-radius,text-ratio,wrap-width,wrap-before,"
             << "wrap-character,text-transform,line-spacing,"
             << "label-position-tolerance,character-spacing,"
             << "spacing,minimum-distance,minimum-padding,minimum-path-length,"
             << "avoid-edges,allow-overlap,opacity,max-char-angle-delta,"
             << "horizontal-alignment,justify-alignment";

    std::stringstream s_symbolizer;
    s_symbolizer << s_common.str() << ",placements,placement-type,"
                 << "meta-writer,meta-output";

    ensure_attrs(sym, "TextSymbolizer", s_symbolizer.str());
    try
    {
        text_placements_ptr placement_finder;
        text_placements_list *list = 0;
        optional<std::string> placement_type = get_opt_attr<std::string>(sym, "placement-type");
        if (placement_type) {
            if (*placement_type == "simple") {
                placement_finder = text_placements_ptr(
                    new text_placements_simple(
                        get_attr<std::string>(sym, "placements", "X")));
            } else if (*placement_type == "list") {
                list = new text_placements_list();
                placement_finder = text_placements_ptr(list);
            } else if (*placement_type != "dummy" && *placement_type != "") {
                throw config_error(std::string("Unknown placement type '"+*placement_type+"'"));
            }
        }
        if (!placement_finder) {
            placement_finder = text_placements_ptr(new text_placements_dummy());
        }

        placement_finder->properties.from_xml(sym, fontsets_);
        if (strict_ &&
                !placement_finder->properties.default_format.fontset.size())
            ensure_font_face(placement_finder->properties.default_format.face_name);
        if (list) {
            ptree::const_iterator symIter = sym.begin();
            ptree::const_iterator endSym = sym.end();
            for( ;symIter != endSym; ++symIter) {
                if (symIter->first.find('<') != std::string::npos) continue;
                if (symIter->first != "Placement")
                {
//                    throw config_error("Unknown element '" + symIter->first + "'"); TODO
                    continue;
                }
                ensure_attrs(symIter->second, "TextSymbolizer/Placement", s_common.str());
                text_symbolizer_properties & p = list->add();
                p.from_xml(symIter->second, fontsets_);
                if (strict_ &&
                        !p.default_format.fontset.size())
                    ensure_font_face(p.default_format.face_name);
            }
        }

        text_symbolizer text_symbol = text_symbolizer(placement_finder);
        parse_metawriter_in_symbolizer(text_symbol, sym);
        rule.append(text_symbol);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in TextSymbolizer");
        throw;
    }
}

void map_parser::parse_shield_symbolizer( rule & rule, ptree const & sym )
{
    std::string s_common(
        "name,face-name,fontset-name,size,fill,orientation,"
        "dx,dy,placement,vertical-alignment,halo-fill,"
        "halo-radius,text-ratio,wrap-width,wrap-before,"
        "wrap-character,text-transform,line-spacing,"
        "label-position-tolerance,character-spacing,"
        "spacing,minimum-distance,minimum-padding,minimum-path-length,"
        "avoid-edges,allow-overlap,opacity,max-char-angle-delta,"
        "horizontal-alignment,justify-alignment");

    std::string s_symbolizer(s_common + ",file,base,"
                             "transform,shield-dx,shield-dy,text-opacity,"
                             "unlock-image"
                             "placements,placement-type,meta-writer,meta-output");

    ensure_attrs(sym, "ShieldSymbolizer", s_symbolizer);
    try
    {
        text_placements_ptr placement_finder;
        text_placements_list *list = 0;
        optional<std::string> placement_type = get_opt_attr<std::string>(sym, "placement-type");
        if (placement_type) {
            if (*placement_type == "simple") {
                placement_finder = text_placements_ptr(
                    new text_placements_simple(
                        get_attr<std::string>(sym, "placements", "X")));
            } else if (*placement_type == "list") {
                list = new text_placements_list();
                placement_finder = text_placements_ptr(list);
            } else if (*placement_type != "dummy" && *placement_type != "") {
                throw config_error(std::string("Unknown placement type '"+*placement_type+"'"));
            }
        }
        if (!placement_finder) {
            placement_finder = text_placements_ptr(new text_placements_dummy());
        }

        placement_finder->properties.from_xml(sym, fontsets_);
        if (strict_) ensure_font_face(placement_finder->properties.default_format.face_name);
        if (list) {
            ptree::const_iterator symIter = sym.begin();
            ptree::const_iterator endSym = sym.end();
            for( ;symIter != endSym; ++symIter) {
                if (symIter->first.find('<') != std::string::npos) continue;
                if (symIter->first != "Placement")
                {
//                    throw config_error("Unknown element '" + symIter->first + "'"); TODO
                    continue;
                }
                ensure_attrs(symIter->second, "TextSymbolizer/Placement", s_common);
                text_symbolizer_properties & p = list->add();
                p.from_xml(symIter->second, fontsets_);
                if (strict_) ensure_font_face(p.default_format.face_name);
            }
        }

        shield_symbolizer shield_symbol = shield_symbolizer(placement_finder);
        /* Symbolizer specific attributes. */
        optional<std::string> transform_wkt = get_opt_attr<std::string>(sym, "transform");
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
        double shield_dx = get_attr(sym, "shield-dx", 0.0);
        double shield_dy = get_attr(sym, "shield-dy", 0.0);
        shield_symbol.set_shield_displacement(shield_dx,shield_dy);

        // opacity
        optional<double> opacity = get_opt_attr<double>(sym, "opacity");
        if (opacity)
        {
            shield_symbol.set_opacity(*opacity);
        }

        // text-opacity
        // TODO: Could be problematic because it is named opacity in TextSymbolizer but opacity has a diffrent meaning here.
        optional<double> text_opacity =
            get_opt_attr<double>(sym, "text-opacity");
        if (text_opacity)
        {
            shield_symbol.set_text_opacity( * text_opacity );
        }

        // unlock_image
        optional<boolean> unlock_image =
            get_opt_attr<boolean>(sym, "unlock-image");
        if (unlock_image)
        {
            shield_symbol.set_unlock_image( * unlock_image );
        }

        parse_metawriter_in_symbolizer(shield_symbol, sym);

        std::string image_file = get_attr<std::string>(sym, "file");
        optional<std::string> base = get_opt_attr<std::string>(sym, "base");

        try
        {
            if( base )
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
        catch (image_reader_exception const & ex )
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
        ex.append_context("in ShieldSymbolizer");
        throw;
    }
}

void map_parser::parse_stroke(stroke & strk, ptree const & sym)
{
    // stroke color
    optional<color> c = get_opt_attr<color>(sym, "stroke");
    if (c) strk.set_color(*c);

    // stroke-width
    optional<double> width =  get_opt_attr<double>(sym, "stroke-width");
    if (width) strk.set_width(*width);

    // stroke-opacity
    optional<double> opacity = get_opt_attr<double>(sym, "stroke-opacity");
    if (opacity) strk.set_opacity(*opacity);

    // stroke-linejoin
    optional<line_join_e> line_join = get_opt_attr<line_join_e>(sym, "stroke-linejoin");
    if (line_join) strk.set_line_join(*line_join);

    // stroke-linecap
    optional<line_cap_e> line_cap = get_opt_attr<line_cap_e>(sym, "stroke-linecap");
    if (line_cap) strk.set_line_cap(*line_cap);

    // stroke-gamma
    optional<double> gamma = get_opt_attr<double>(sym, "stroke-gamma");
    if (gamma) strk.set_gamma(*gamma);

    // stroke-gamma-method
    optional<gamma_method_e> gamma_method = get_opt_attr<gamma_method_e>(sym, "stroke-gamma-method");
    if (gamma_method) strk.set_gamma_method(*gamma_method);

    // stroke-dashoffset
    optional<double> dash_offset = get_opt_attr<double>(sym, "stroke-dashoffset");
    if (dash_offset) strk.set_dash_offset(*dash_offset);

    // stroke-dasharray
    optional<std::string> str = get_opt_attr<std::string>(sym,"stroke-dasharray");
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
            catch ( boost::bad_lexical_cast &)
            {
                throw config_error(std::string("Failed to parse dasharray ") +
                                   "'. Expected a " +
                                   "list of floats but got '" + (*str) + "'");
            }
        }
        if (dash_array.size())
        {
            size_t size = dash_array.size();
            if ( size % 2)
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

void map_parser::parse_line_symbolizer( rule & rule, ptree const & sym )
{
    std::stringstream s;
    s << "stroke,stroke-width,stroke-opacity,stroke-linejoin,"
      << "stroke-linecap,stroke-gamma,stroke-dash-offset,stroke-dasharray,"
      << "rasterizer,"
      << "meta-writer,meta-output";

    ensure_attrs(sym, "LineSymbolizer", s.str());
    try
    {
        stroke strk;
        parse_stroke(strk,sym);
        line_symbolizer symbol = line_symbolizer(strk);

        // rasterizer method
        line_rasterizer_e rasterizer = get_attr<line_rasterizer_e>(sym, "rasterizer", RASTERIZER_FULL);
        //optional<line_rasterizer_e> rasterizer_method = get_opt_attr<line_rasterizer_e>(sym, "full");
        symbol.set_rasterizer(rasterizer);

        parse_metawriter_in_symbolizer(symbol, sym);
        rule.append(symbol);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in LineSymbolizer");
        throw;
    }
}


void map_parser::parse_polygon_symbolizer( rule & rule, ptree const & sym )
{
    ensure_attrs(sym, "PolygonSymbolizer", "fill,fill-opacity,gamma,gamma-method,meta-writer,meta-output");
    try
    {
        polygon_symbolizer poly_sym;
        // fill
        optional<color> fill = get_opt_attr<color>(sym, "fill");
        if (fill) poly_sym.set_fill(*fill);
        // fill-opacity
        optional<double> opacity = get_opt_attr<double>(sym, "fill-opacity");
        if (opacity) poly_sym.set_opacity(*opacity);
        // gamma
        optional<double> gamma = get_opt_attr<double>(sym, "gamma");
        if (gamma)  poly_sym.set_gamma(*gamma);
        // gamma method
        optional<gamma_method_e> gamma_method = get_opt_attr<gamma_method_e>(sym, "gamma-method");
        if (gamma_method) poly_sym.set_gamma_method(*gamma_method);

        parse_metawriter_in_symbolizer(poly_sym, sym);
        rule.append(poly_sym);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in PolygonSymbolizer");
        throw;
    }
}


void map_parser::parse_building_symbolizer( rule & rule, ptree const & sym )
{
    ensure_attrs(sym, "PolygonSymbolizer", "fill,fill-opacity,height,meta-writer,meta-output");
    try
    {
        building_symbolizer building_sym;

        // fill
        optional<color> fill = get_opt_attr<color>(sym, "fill");
        if (fill) building_sym.set_fill(*fill);
        // fill-opacity
        optional<double> opacity = get_opt_attr<double>(sym, "fill-opacity");
        if (opacity) building_sym.set_opacity(*opacity);
        // height
        optional<std::string> height = get_opt_attr<std::string>(sym, "height");
        if (height) building_sym.set_height(parse_expression(*height, "utf8"));

        parse_metawriter_in_symbolizer(building_sym, sym);
        rule.append(building_sym);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in BuildingSymbolizer");
        throw;
    }
}

void map_parser::parse_raster_symbolizer( rule & rule, ptree const & sym )
{
    // no support for meta-writer,meta-output
    ensure_attrs(sym, "RasterSymbolizer", "mode,scaling,opacity,filter-factor,mesh-size");
    try
    {
        raster_symbolizer raster_sym;

        // mode
        optional<std::string> mode = get_opt_attr<std::string>(sym, "mode");
        if (mode) raster_sym.set_mode(*mode);

        // scaling
        optional<std::string> scaling = get_opt_attr<std::string>(sym, "scaling");
        if (scaling) raster_sym.set_scaling(*scaling);

        // opacity
        optional<double> opacity = get_opt_attr<double>(sym, "opacity");
        if (opacity) raster_sym.set_opacity(*opacity);

        // filter factor
        optional<double> filter_factor = get_opt_attr<double>(sym, "filter-factor");
        if (filter_factor) raster_sym.set_filter_factor(*filter_factor);

        // mesh-size
        optional<unsigned> mesh_size = get_opt_attr<unsigned>(sym, "mesh-size");
        if (mesh_size) raster_sym.set_mesh_size(*mesh_size);


        ptree::const_iterator cssIter = sym.begin();
        ptree::const_iterator endCss = sym.end();

        for(; cssIter != endCss; ++cssIter)
        {
            ptree::value_type const& css_tag = *cssIter;

            if (css_tag.first == "RasterColorizer")
            {
                raster_colorizer_ptr colorizer(new raster_colorizer());
                raster_sym.set_colorizer(colorizer);
                parse_raster_colorizer(colorizer, css_tag.second);
            }
            else if (css_tag.first != "<xmlcomment>" &&
                     css_tag.first != "<xmlattr>" )
            {
                throw config_error(std::string("Unknown child node. ") +
                                   "Expected 'RasterColorizer' but got '" + css_tag.first + "'");
            }
        }
        //Note: raster_symbolizer doesn't support metawriters
        rule.append(raster_sym);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in RasterSymbolizer");
        throw;
    }
}

void map_parser::parse_raster_colorizer(raster_colorizer_ptr const& rc,
                                        ptree const& node )
{
    try
    {
        ensure_attrs(node, "RasterColorizer", "default-mode,default-color,epsilon");
        // mode
        colorizer_mode default_mode =
            get_attr<colorizer_mode>(node, "default-mode", COLORIZER_LINEAR);

        if(default_mode == COLORIZER_INHERIT) {
            throw config_error("RasterColorizer mode must not be INHERIT. ");
        }
        rc->set_default_mode( default_mode );

        // default colour
        optional<color> default_color = get_opt_attr<color>(node, "default-color");
        if (default_color)
        {
            rc->set_default_color( *default_color );
        }


        // epsilon
        optional<float> eps = get_opt_attr<float>(node, "epsilon");
        if (eps)
        {
            if(*eps < 0) {
                throw config_error("RasterColorizer epsilon must be > 0. ");
            }
            rc->set_epsilon( *eps );
        }


        ptree::const_iterator stopIter = node.begin();
        ptree::const_iterator endStop = node.end();
        float maximumValue = -std::numeric_limits<float>::max();

        for(; stopIter != endStop; ++stopIter)
        {
            ptree::value_type const& stop_tag = *stopIter;
            ptree const & stop = stopIter->second;

            if (stop_tag.first == "stop")
            {
                ensure_attrs(stop_tag.second, "stop", "color,mode,value,label");
                // colour is optional.
                optional<color> stopcolor = get_opt_attr<color>(stop, "color");
                if (!stopcolor) {
                    *stopcolor = *default_color;
                }

                // mode default to INHERIT
                colorizer_mode mode =
                    get_attr<colorizer_mode>(stop, "mode", COLORIZER_INHERIT);

                // value is required, and it must be bigger than the previous
                optional<float> value =
                    get_opt_attr<float>(stop, "value");

                if(!value) {
                    throw config_error("stop tag missing value");
                }

                if(value < maximumValue) {
                    throw config_error("stop tag values must be in ascending order");
                }
                maximumValue = *value;

                optional<std::string> label =
                    get_opt_attr<std::string>(stop, "label");

                //append the stop
                colorizer_stop tmpStop;
                tmpStop.set_color(*stopcolor);
                tmpStop.set_mode(mode);
                tmpStop.set_value(*value);
                if (label)
                    tmpStop.set_label(*label);

                rc->add_stop(tmpStop);
            }
            else if (stop_tag.first != "<xmlcomment>" &&
                     stop_tag.first != "<xmlattr>" )
            {
                throw config_error(std::string("Unknown child node. ") +
                                   "Expected 'stop' but got '" + stop_tag.first + "'");
            }
        }
    }
    catch (const config_error & ex)
    {
        ex.append_context("in RasterColorizer");
        throw;
    }
}

void map_parser::ensure_font_face( const std::string & face_name )
{
    if ( ! font_manager_.get_face( face_name ) )
    {
        throw config_error("Failed to find font face '" +
                           face_name + "'");
    }
}

std::string map_parser::ensure_relative_to_xml( boost::optional<std::string> opt_path )
{
    if (relative_to_xml_)
    {
        boost::filesystem::path xml_path = filename_;
        boost::filesystem::path rel_path = *opt_path;
        if ( !rel_path.has_root_path() )
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

void map_parser::ensure_attrs(ptree const& sym, std::string name, std::string attrs)
{
    typedef ptree::key_type::value_type Ch;
    optional<const ptree &> attribs = sym.get_child_optional( boost::property_tree::xml_parser::xmlattr<Ch>() );
    if (attribs)
    {
        std::set<std::string> attr_set;
        boost::split(attr_set, attrs, boost::is_any_of(","));
        std::ostringstream s("");
        s << "### " << name << " properties warning: ";
        int missing = 0;
        for (ptree::const_iterator it = attribs.get().begin(); it != attribs.get().end(); ++it)
        {
            std::string name = it->first;
            bool found = (attr_set.find(name) != attr_set.end());
            if (!found)
            {
                if (missing)
                {
                    s << ",";
                }
                s << "'" << name << "'";
                ++missing;
            }
        }
        if (missing) {
            if (missing > 1)
            {
                s << " are";
            }
            else
            {
                s << " is";
            }
            s << " invalid, acceptable values are:\n'" << attrs << "'\n";
            std::clog << s.str();
        }
    }
}

} // end of namespace mapnik
