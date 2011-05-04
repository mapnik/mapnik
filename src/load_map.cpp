/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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

    void parse_map(Map & map, ptree const & sty);
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
    void parse_glyph_symbolizer(rule & rule, ptree const & sym );

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

void load_map_string(Map & map, std::string const& str, bool strict, std::string const& base_url)
{
    ptree pt;
#ifdef HAVE_LIBXML2
    read_xml2_string(str, pt, base_url);
#else
    try
    {
        std::istringstream s(str);
        read_xml(s,pt);
    }
    catch (const boost::property_tree::xml_parser_error & ex)
    {
        throw config_error( ex.what() ) ;
    }
#endif

    map_parser parser( strict, base_url);
    parser.parse_map(map, pt);
}

void map_parser::parse_map( Map & map, ptree const & pt )
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
          << "maximum-extent";
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

            optional<color> bgcolor = get_opt_attr<color>(map_node, "background-color");
            if (bgcolor) 
            {
                map.set_background( * bgcolor );
            }
            
            optional<std::string> image_filename = get_opt_attr<string>(map_node, "background-image");
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
                    std::ostringstream s;
                    s << "failed to parse 'maximum-extent'";
                    if ( strict_ )
                        throw config_error(s.str());
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
            std::string name = get_attr<string>( v.second, "name");
            std::string value = get_value<string>( v.second, "");
            file_sources_[name] = value;
        }
        else if (v.first == "Datasource")
        {
            std::string name = get_attr(v.second, "name", string("Unnamed"));
            parameters params;
            ptree::const_iterator paramIter = v.second.begin();
            ptree::const_iterator endParam = v.second.end();
            for (; paramIter != endParam; ++paramIter)
            {
                ptree const& param = paramIter->second;

                if (paramIter->first == "Parameter")
                {
                    std::string name = get_attr<string>(param, "name");
                    std::string value = get_value<string>( param,
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

    string name("<missing name>");
    try
    {
        name = get_attr<string>(sty, "name");
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
            ex.append_context(string("in style '") + name + "'");
        }
        ex.append_context(string("in map '") + filename_ + "'");
        throw;
    }
}

void map_parser::parse_metawriter(Map & map, ptree const & pt)
{
    ensure_attrs(pt, "MetaWriter", "name,type,file,default-output,output-empty");
    string name("<missing name>");
    metawriter_ptr writer;
    try
    {
        name = get_attr<string>(pt, "name");
        writer = metawriter_create(pt);
        map.insert_metawriter(name, writer);

    } catch (const config_error & ex) {
        if (!name.empty()) {
            ex.append_context(string("in meta writer '") + name + "'");
        }
        ex.append_context(string("in map '") + filename_ + "'");
        throw;
    }
}

void map_parser::parse_fontset( Map & map, ptree const & fset )
{
    ensure_attrs(fset, "FontSet", "name,Font");
    string name("<missing name>");
    try
    {
        name = get_attr<string>(fset, "name");
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
            ex.append_context(string("in FontSet '") + name + "'");
        }
        ex.append_context(string("in map '") + filename_ + "'");
        throw;
    }
}

void map_parser::parse_font(font_set & fset, ptree const & f)
{
    ensure_attrs(f, "Font", "face-name");

    std::string face_name = get_attr(f, "face-name", string());

    if ( strict_ )
    {
        ensure_font_face( face_name );
    }

    fset.add_face_name(face_name);
}

void map_parser::parse_layer( Map & map, ptree const & lay )
{
    std::string name;
    std::ostringstream s("");
    s << "name,"
      << "srs,"
      << "status,"
      << "title,"
      << "abstract,"
      << "minzoom,"
      << "maxzoom,"
      << "queryable,"
      << "clear-label-cache";
    ensure_attrs(lay, "Layer", s.str());
    try
    {
        name = get_attr(lay, "name", string("Unnamed"));

        // XXX if no projection is given inherit from map? [DS]
        std::string srs = get_attr(lay, "srs", map.srs());

        layer lyr(name, srs);

        optional<boolean> status = get_opt_attr<boolean>(lay, "status");
        if (status)
        {
            lyr.setActive( * status );
        }

        optional<std::string> title =  get_opt_attr<string>(lay, "title");
        if (title)
        {
            lyr.set_title( * title );
        }

        optional<std::string> abstract =  get_opt_attr<string>(lay, "abstract");
        if (abstract)
        {
            lyr.set_abstract( * abstract );
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


        ptree::const_iterator itr2 = lay.begin();
        ptree::const_iterator end2 = lay.end();

        for(; itr2 != end2; ++itr2)
        {
            ptree::value_type const& child = *itr2;

            if (child.first == "StyleName")
            {
                ensure_attrs(child.second, "StyleName", "none");
                std::string style_name = child.second.data();
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
                        std::string name = get_attr<string>(param, "name");
                        std::string value = get_value<string>( param,
                                                               "datasource parameter");
                        params[name] = value;
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

                // catch problem at datasource registration
                catch (const mapnik::config_error & ex )
                {
                    throw config_error( ex.what() );
                }

                // catch problem at the datasource creation
                catch (const mapnik::datasource_exception & ex )
                {
                    throw config_error( ex.what() );
                }

                catch (...)
                {
                    //throw config_error("exception...");
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
    ensure_attrs(r, "Rule", "name,title");
    std::string name;
    try
    {
        name = get_attr( r, "name", string());
        std::string title = get_attr( r, "title", string());

        rule rule(name,title);

        optional<std::string> filter_expr =
            get_opt_child<string>( r, "Filter");
        if (filter_expr)
        {
            // TODO - can we use encoding defined for XML document for filter expressions?
            rule.set_filter(parse_expression(*filter_expr,"utf8"));
        }

        optional<std::string> else_filter =
            get_opt_child<string>(r, "ElseFilter");
        if (else_filter)
        {
            rule.set_else(true);
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
            else if ( sym.first == "GlyphSymbolizer")
            {
                parse_glyph_symbolizer( rule, sym.second );
            }

            else if ( sym.first != "MinScaleDenominator" &&
                      sym.first != "MaxScaleDenominator" &&
                      sym.first != "Filter" &&
                      sym.first != "ElseFilter" &&
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
            ex.append_context(string("in rule '") + name + "' in map '" + filename_ + "')");
        }
        throw;
    }
}

void map_parser::parse_metawriter_in_symbolizer(symbolizer_base &sym, ptree const &pt)
{
    optional<std::string> writer =  get_opt_attr<string>(pt, "meta-writer");
    if (!writer) return;
    optional<std::string> output =  get_opt_attr<string>(pt, "meta-output");
    sym.add_metawriter(*writer, output);
}

void map_parser::parse_point_symbolizer( rule & rule, ptree const & sym )
{
    try
    {
        std::stringstream s;
        s << "file,base,allow-overlap,ignore-placement,opacity,placement,transform,meta-writer,meta-output";
        
        optional<std::string> file =  get_opt_attr<string>(sym, "file");
        optional<std::string> base =  get_opt_attr<string>(sym, "base");
        optional<boolean> allow_overlap =
            get_opt_attr<boolean>(sym, "allow-overlap");
        optional<boolean> ignore_placement =
            get_opt_attr<boolean>(sym, "ignore-placement");
        optional<float> opacity =
            get_opt_attr<float>(sym, "opacity");
        
        optional<std::string> transform_wkt = get_opt_attr<string>(sym, "transform");

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
                    if (!mapnik::svg::parse_transform(*transform_wkt,tr))
                    {
                        std::stringstream ss;
                        ss << "Could not parse transform from '" << transform_wkt 
                           << "', expected string like: 'matrix(1, 0, 0, 1, 0, 0)'";
                        if (strict_)
                            throw config_error(ss.str()); // value_error here?
                        else
                            clog << "### WARNING: " << ss << endl;         
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
                string msg("Failed to load image file '" + * file +
                           "': " + ex.what());
                if (strict_)
                {
                    throw config_error(msg);
                }
                else
                {
                    clog << "### WARNING: " << msg << endl;
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
        optional<std::string> file =  get_opt_attr<string>(sym, "file");
        optional<std::string> base =  get_opt_attr<string>(sym, "base");
        optional<std::string> transform_wkt = get_opt_attr<string>(sym, "transform");

        std::stringstream s;
        //s << "file,opacity,spacing,max-error,allow-overlap,placement,";
        s << "file,base,transform,fill,opacity,"
          << "spacing,max-error,allow-overlap,"
          << "width,height,placement,marker-type,"
          << "stroke,stroke-width,stroke-opacity,stroke-linejoin,"
          << "stroke-linecap,stroke-gamma,stroke-dashoffet,stroke-dasharray,"
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
                string msg("Failed to load marker file '" + *file + "'!");
                if (strict_)
                {
                    throw config_error(msg);
                }
                else
                {
                    clog << "### WARNING: " << msg << endl;
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
            if (!mapnik::svg::parse_transform(*transform_wkt,tr))
            {
                std::stringstream ss;
                ss << "Could not parse transform from '" << transform_wkt
                   << "', expected string like: 'matrix(1, 0, 0, 1, 0, 0)'";
                if (strict_)
                    throw config_error(ss.str()); // value_error here?
                else
                    clog << "### WARNING: " << ss << endl;         
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
        std::string file = get_attr<string>(sym, "file");
        optional<std::string> base = get_opt_attr<string>(sym, "base");
            
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
            string msg("Failed to load image file '" + file +
                       "': " + ex.what());
            if (strict_)
            {
                throw config_error(msg);
            }
            else
            {
                clog << "### WARNING: " << msg << endl;
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
    ensure_attrs(sym, "PolygonPatternSymbolizer", "file,base,alignment,meta-writer,meta-output");
    try
    {
        std::string file = get_attr<string>(sym, "file");
        optional<std::string> base = get_opt_attr<string>(sym, "base");
            
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

            parse_metawriter_in_symbolizer(symbol, sym);
            rule.append(symbol);
        }
        catch (image_reader_exception const & ex )
        {
            string msg("Failed to load image file '" + file +
                       "': " + ex.what());
            if (strict_)
            {
                throw config_error(msg);
            }
            else
            {
                clog << "### WARNING: " << msg << endl;
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
    std::stringstream s;
    s << "name,face-name,fontset-name,size,fill,orientation,"
      << "dx,dy,placement,vertical-alignment,halo-fill,"
      << "halo-radius,text-ratio,wrap-width,wrap-before,"
      << "wrap-character,text-transform,line-spacing,"
      << "label-position-tolerance,character-spacing,"
      << "spacing,minimum-distance,minimum-padding,"
      << "avoid-edges,allow-overlap,opacity,max-char-angle-delta,"
      << "horizontal-alignment,justify-alignment,"
      << "placements,placement-type,"
      << "meta-writer,meta-output";
    
    ensure_attrs(sym, "TextSymbolizer", s.str());
    try
    {
        text_placements_ptr placement_finder;
        optional<std::string> placement_type = get_opt_attr<std::string>(sym, "placement-type");
        if (placement_type) {
            if (*placement_type == "simple") {
                placement_finder = text_placements_ptr(
                    new text_placements_simple(
                        get_attr<std::string>(sym, "placements", "X")));
            } else if (*placement_type != "dummy" && *placement_type != "") {
                throw config_error(std::string("Unknown placement type '"+*placement_type+"'"));
            }
        }
        if (!placement_finder) {
            placement_finder = text_placements_ptr(new text_placements_dummy());
        }

        std::string name = get_attr<string>(sym, "name");
        
        optional<std::string> face_name =
            get_opt_attr<std::string>(sym, "face-name");

        optional<std::string> fontset_name =
            get_opt_attr<std::string>(sym, "fontset-name");

        unsigned size = get_attr(sym, "size", 10U);

        color c = get_attr(sym, "fill", color(0,0,0));

        text_symbolizer text_symbol = text_symbolizer(parse_expression(name, "utf8"), size, c, placement_finder);

        optional<std::string> orientation = get_opt_attr<std::string>(sym, "orientation");
        if (orientation)
        {
            text_symbol.set_orientation(parse_expression(*orientation, "utf8"));
        }
        
        if (fontset_name && face_name)
        {
            throw config_error(std::string("Can't have both face-name and fontset-name"));
        }
        else if (fontset_name)
        {
            std::map<std::string,font_set>::const_iterator itr = fontsets_.find(*fontset_name);
            if (itr != fontsets_.end())
            {
                text_symbol.set_fontset(itr->second);
            }
            else
            {
                throw config_error("Unable to find any fontset named '" + *fontset_name + "'");
            }
        }
        else if (face_name)
        {
            if ( strict_ )
            {
                ensure_font_face(*face_name);
            }
            text_symbol.set_face_name(*face_name);
        }
        else
        {
            throw config_error(std::string("Must have face-name or fontset-name"));
        }

        double dx = get_attr(sym, "dx", 0.0);
        double dy = get_attr(sym, "dy", 0.0);
        text_symbol.set_displacement(dx,dy);

        label_placement_e placement =
            get_attr<label_placement_e>(sym, "placement", POINT_PLACEMENT);
        text_symbol.set_label_placement( placement );

        // vertical alignment
        vertical_alignment_e default_vertical_alignment = V_AUTO;
            
        vertical_alignment_e valign = get_attr<vertical_alignment_e>(sym, "vertical-alignment", default_vertical_alignment);
        text_symbol.set_vertical_alignment(valign);

        // halo fill and radius
        optional<color> halo_fill = get_opt_attr<color>(sym, "halo-fill");
        if (halo_fill)
        {
            text_symbol.set_halo_fill( * halo_fill );
        }
        optional<double> halo_radius =
            get_opt_attr<double>(sym, "halo-radius");
        if (halo_radius)
        {
            text_symbol.set_halo_radius(*halo_radius);
        }
        
        // text ratio and wrap width
        optional<unsigned> text_ratio =
            get_opt_attr<unsigned>(sym, "text-ratio");
        if (text_ratio)
        {
            text_symbol.set_text_ratio(*text_ratio);
        }

        optional<unsigned> wrap_width =
            get_opt_attr<unsigned>(sym, "wrap-width");
        if (wrap_width)
        {
            text_symbol.set_wrap_width(*wrap_width);
        }

        optional<boolean> wrap_before =
            get_opt_attr<boolean>(sym, "wrap-before");
        if (wrap_before)
        {
            text_symbol.set_wrap_before(*wrap_before);
        }

        // character used to break long strings
        optional<std::string> wrap_char =
            get_opt_attr<std::string>(sym, "wrap-character");
        if (wrap_char && (*wrap_char).size() > 0)
        {
            text_symbol.set_wrap_char((*wrap_char)[0]);
        }

        // text conversion before rendering
        text_transform_e tconvert =
            get_attr<text_transform_e>(sym, "text-transform", NONE);
        text_symbol.set_text_transform(tconvert);

        // spacing between text lines
        optional<unsigned> line_spacing = get_opt_attr<unsigned>(sym, "line-spacing");
        if (line_spacing)
        {
            text_symbol.set_line_spacing(*line_spacing);
        }

        // tolerance between label spacing along line
        optional<unsigned> label_position_tolerance = get_opt_attr<unsigned>(sym, "label-position-tolerance");
        if (label_position_tolerance)
        {
            text_symbol.set_label_position_tolerance(*label_position_tolerance);
        }

        // spacing between characters in text
        optional<unsigned> character_spacing = get_opt_attr<unsigned>(sym, "character-spacing");
        if (character_spacing)
        {
            text_symbol.set_character_spacing(*character_spacing);
        }

        // spacing between repeated labels on lines
        optional<unsigned> spacing = get_opt_attr<unsigned>(sym, "spacing");
        if (spacing)
        {
            text_symbol.set_label_spacing(*spacing);
        }

        // minimum distance between labels
        optional<unsigned> min_distance = get_opt_attr<unsigned>(sym, "minimum-distance");
        if (min_distance)
        {
            text_symbol.set_minimum_distance(*min_distance);
        }
        
        // minimum distance from edge of the map
        optional<unsigned> min_padding = get_opt_attr<unsigned>(sym, "minimum-padding");
        if (min_padding)
        {
            text_symbol.set_minimum_padding(*min_padding);
        }
        
        // do not render labels around edges
        optional<boolean> avoid_edges =
            get_opt_attr<boolean>(sym, "avoid-edges");
        if (avoid_edges)
        {
            text_symbol.set_avoid_edges( * avoid_edges);
        }

        // allow_overlap
        optional<boolean> allow_overlap =
            get_opt_attr<boolean>(sym, "allow-overlap");
        if (allow_overlap)
        {
            text_symbol.set_allow_overlap( * allow_overlap );
        }

        // opacity
        optional<double> opacity =
            get_opt_attr<double>(sym, "opacity");
        if (opacity)
        {
            text_symbol.set_text_opacity( * opacity );
        }
        
        // max_char_angle_delta
        optional<double> max_char_angle_delta =
            get_opt_attr<double>(sym, "max-char-angle-delta");
        if (max_char_angle_delta)
        {
            text_symbol.set_max_char_angle_delta( (*max_char_angle_delta)*(M_PI/180));
        }
            
        // horizontal alignment
        horizontal_alignment_e halign = get_attr<horizontal_alignment_e>(sym, "horizontal-alignment", H_AUTO);
        text_symbol.set_horizontal_alignment(halign);

        // justify alignment
        justify_alignment_e jalign = get_attr<justify_alignment_e>(sym, "justify-alignment", J_MIDDLE);
        text_symbol.set_justify_alignment(jalign);

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

    std::stringstream s;
    //std::string a[] = {"a","b"};
    s << "name,face-name,fontset-name,size,fill,"
      << "dx,dy,placement,vertical-alignment,halo-fill,"
      << "halo-radius,text-ratio,wrap-width,wrap-before,"
      << "wrap-character,text-transform,line-spacing,"
      << "label-position-tolerance,character-spacing,"
      << "spacing,minimum-distance,minimum-padding,"
      << "avoid-edges,allow-overlap,opacity,max-char-angle-delta,"
      << "horizontal-alignment,justify-alignment,"
      // additional for shield
      /* transform instead of orientation */ 
      << "file,base,transform,shield-dx,shield-dy,"
      << "text-opacity,unlock-image,no-text,"
      << "meta-writer,meta-output";      
    
    ensure_attrs(sym, "ShieldSymbolizer", s.str());
    try
    {
        std::string name =  get_attr<string>(sym, "name");

        optional<std::string> face_name =
            get_opt_attr<std::string>(sym, "face-name");

        optional<std::string> fontset_name =
            get_opt_attr<std::string>(sym, "fontset-name");

        unsigned size = get_attr(sym, "size", 10U);
        color fill = get_attr(sym, "fill", color(0,0,0));

        std::string image_file = get_attr<string>(sym, "file");
        optional<std::string> base = get_opt_attr<string>(sym, "base");
        
        optional<std::string> transform_wkt = get_opt_attr<string>(sym, "transform");
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

            shield_symbolizer shield_symbol(parse_expression(name, "utf8"),size,fill,parse_path(image_file));
                
            if (fontset_name && face_name)
            {
                throw config_error(std::string("Can't have both face-name and fontset-name"));
            }
            else if (fontset_name)
            {
                std::map<std::string,font_set>::const_iterator itr = fontsets_.find(*fontset_name);
                if (itr != fontsets_.end())
                {
                    shield_symbol.set_fontset(itr->second);
                }
                else
                {
                    throw config_error("Unable to find any fontset named '" + *fontset_name + "'");
                }
            }
            else if (face_name)
            {
                if ( strict_ )
                {
                    ensure_font_face(*face_name);
                }
                shield_symbol.set_face_name(*face_name);
            }
            else
            {
                throw config_error(std::string("Must have face-name or fontset-name"));
            }
            // text displacement (relative to shield_displacement)
            double dx = get_attr(sym, "dx", 0.0);
            double dy = get_attr(sym, "dy", 0.0);
            shield_symbol.set_displacement(dx,dy);
            // shield displacement
            double shield_dx = get_attr(sym, "shield-dx", 0.0);
            double shield_dy = get_attr(sym, "shield-dy", 0.0);
            shield_symbol.set_shield_displacement(shield_dx,shield_dy);
            
            label_placement_e placement =
                get_attr<label_placement_e>(sym, "placement", POINT_PLACEMENT);
            shield_symbol.set_label_placement( placement );

            // don't render shields around edges
            optional<boolean> avoid_edges =
                get_opt_attr<boolean>(sym, "avoid-edges");
            if (avoid_edges)
            {
                shield_symbol.set_avoid_edges( *avoid_edges);
            }

            // halo fill and radius
            optional<color> halo_fill = get_opt_attr<color>(sym, "halo-fill");
            if (halo_fill)
            {
                shield_symbol.set_halo_fill( * halo_fill );
            }
            optional<double> halo_radius =
                get_opt_attr<double>(sym, "halo-radius");
            if (halo_radius)
            {
                shield_symbol.set_halo_radius(*halo_radius);
            }

            // minimum distance between labels
            optional<unsigned> min_distance = get_opt_attr<unsigned>(sym, "minimum-distance");
            if (min_distance)
            {
                shield_symbol.set_minimum_distance(*min_distance);
            }

            // minimum distance from edge of the map
            optional<unsigned> min_padding = get_opt_attr<unsigned>(sym, "minimum-padding");
            if (min_padding)
            {
                shield_symbol.set_minimum_padding(*min_padding);
            }
            
            // spacing between repeated labels on lines
            optional<unsigned> spacing = get_opt_attr<unsigned>(sym, "spacing");
            if (spacing)
            {
                shield_symbol.set_label_spacing(*spacing);
            }

            // allow_overlap
            optional<boolean> allow_overlap =
                get_opt_attr<boolean>(sym, "allow-overlap");
            if (allow_overlap)
            {
                shield_symbol.set_allow_overlap( * allow_overlap );
            }

            // vertical alignment
            vertical_alignment_e valign = get_attr<vertical_alignment_e>(sym, "vertical-alignment", V_MIDDLE);
            shield_symbol.set_vertical_alignment(valign);

            // horizontal alignment
            horizontal_alignment_e halign = get_attr<horizontal_alignment_e>(sym, "horizontal-alignment", H_MIDDLE);
            shield_symbol.set_horizontal_alignment(halign);

            // justify alignment
            justify_alignment_e jalign = get_attr<justify_alignment_e>(sym, "justify-alignment", J_MIDDLE);
            shield_symbol.set_justify_alignment(jalign);

            optional<unsigned> wrap_width =
                get_opt_attr<unsigned>(sym, "wrap-width");
            if (wrap_width)
            {
                shield_symbol.set_wrap_width(*wrap_width);
            }

            optional<boolean> wrap_before =
                get_opt_attr<boolean>(sym, "wrap-before");
            if (wrap_before)
            {
                shield_symbol.set_wrap_before(*wrap_before);
            }

            // character used to break long strings
            optional<std::string> wrap_char =
                get_opt_attr<std::string>(sym, "wrap-character");
            if (wrap_char && (*wrap_char).size() > 0)
            {
                shield_symbol.set_wrap_char((*wrap_char)[0]);
            }

            // text conversion before rendering
            text_transform_e tconvert =
                get_attr<text_transform_e>(sym, "text-transform", NONE);
            shield_symbol.set_text_transform(tconvert);

            // spacing between text lines
            optional<unsigned> line_spacing = get_opt_attr<unsigned>(sym, "line-spacing");
            if (line_spacing)
            {
                shield_symbol.set_line_spacing(*line_spacing);
            }

            // spacing between characters in text
            optional<unsigned> character_spacing = get_opt_attr<unsigned>(sym, "character-spacing");
            if (character_spacing)
            {
                shield_symbol.set_character_spacing(*character_spacing);
            }

            // opacity
            optional<double> opacity =
                get_opt_attr<double>(sym, "opacity");
            if (opacity)
            {
                shield_symbol.set_opacity( * opacity );
            }
            
            // text-opacity
            optional<double> text_opacity =
                get_opt_attr<double>(sym, "text-opacity");
            if (text_opacity)
            {
                shield_symbol.set_text_opacity( * text_opacity );
            }

            if (transform_wkt)
            {
                agg::trans_affine tr;
                if (!mapnik::svg::parse_transform(*transform_wkt,tr))
                {
                    std::stringstream ss;
                    ss << "Could not parse transform from '" << transform_wkt << "', expected string like: 'matrix(1, 0, 0, 1, 0, 0)'";
                    if (strict_)
                        throw config_error(ss.str()); // value_error here?
                    else
                        clog << "### WARNING: " << ss << endl;         
                }
                boost::array<double,6> matrix;
                tr.store_to(&matrix[0]);
                shield_symbol.set_transform(matrix);
            }
            
            // unlock_image
            optional<boolean> unlock_image =
                get_opt_attr<boolean>(sym, "unlock-image");
            if (unlock_image)
            {
                shield_symbol.set_unlock_image( * unlock_image );
            }

            // no text
            optional<boolean> no_text =
                get_opt_attr<boolean>(sym, "no-text");
            if (no_text)
            {
                shield_symbol.set_no_text( * no_text );
            }

            parse_metawriter_in_symbolizer(shield_symbol, sym);
            rule.append(shield_symbol);
        }
        catch (image_reader_exception const & ex )
        {
            string msg("Failed to load image file '" + image_file +
                       "': " + ex.what());
            if (strict_)
            {
                throw config_error(msg);
            }
            else
            {
                clog << "### WARNING: " << msg << endl;
            }
        }

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
    // stroke-dashaffset
    optional<double> offset = get_opt_attr<double>(sym, "stroke-dashoffet");
    if (offset) strk.set_dash_offset(*offset);
    // stroke-dasharray
    optional<string> str = get_opt_attr<string>(sym,"stroke-dasharray");
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
      << "stroke-linecap,stroke-gamma,stroke-dashoffet,stroke-dasharray,"
      << "meta-writer,meta-output";

    ensure_attrs(sym, "LineSymbolizer", s.str());
    try
    {
        stroke strk;
        parse_stroke(strk,sym);
        line_symbolizer symbol = line_symbolizer(strk);

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
    ensure_attrs(sym, "PolygonSymbolizer", "fill,fill-opacity,gamma,meta-writer,meta-output");
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
        // TODO - expression
        optional<double> height = get_opt_attr<double>(sym, "height");
        if (height) building_sym.set_height(*height);

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
    ensure_attrs(sym, "RasterSymbolizer", "mode,scaling,opacity,filter-factor");
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

void map_parser::parse_glyph_symbolizer(rule & rule, ptree const & sym)
{
    ensure_attrs(sym, "GlyphSymbolizer", "face-name,char,angle,angle-mode,value,size,color,halo-fill,halo-radius,allow-overlap,avoid-edges,dx,dy,meta-writer,meta-output");
    try
    {
        // Parse required constructor args
        std::string face_name = get_attr<std::string>(sym, "face-name");
        std::string _char = get_attr<std::string>(sym, "char");

        glyph_symbolizer glyph_sym = glyph_symbolizer(
            face_name,
            parse_expression(_char, "utf8")
            );

        //
        // parse and set optional attrs.
        //

        // angle
        optional<std::string> angle =
            get_opt_attr<std::string>(sym, "angle");
        if (angle)
            glyph_sym.set_angle(parse_expression(*angle, "utf8"));

        angle_mode_e angle_mode =
            get_attr<angle_mode_e>(sym, "angle-mode", TRIGONOMETRIC);
        glyph_sym.set_angle_mode(angle_mode);
                    
        // value
        optional<std::string> value =
            get_opt_attr<std::string>(sym, "value");
        if (value)
            glyph_sym.set_value(parse_expression(*value, "utf8"));

        // size
        std::string size =
            get_attr<std::string>(sym, "size");
        glyph_sym.set_size(parse_expression(size, "utf8"));

        // color
        optional<std::string> _color =
            get_opt_attr<std::string>(sym, "color");
        if (_color)
            glyph_sym.set_color(parse_expression(*_color, "utf8"));

        // halo_fill
        optional<color> halo_fill = get_opt_attr<color>(sym, "halo-fill");
        if (halo_fill)
            glyph_sym.set_halo_fill(*halo_fill);

        // halo_radius
        optional<double> halo_radius = get_opt_attr<double>(
            sym,
            "halo-radius");
        if (halo_radius)
            glyph_sym.set_halo_radius(*halo_radius);
        
        // allow_overlap
        optional<boolean> allow_overlap = get_opt_attr<boolean>(
            sym,
            "allow-overlap"
            );
        if (allow_overlap)
            glyph_sym.set_allow_overlap(*allow_overlap);

        // avoid_edges
        optional<boolean> avoid_edges = get_opt_attr<boolean>(
            sym,
            "avoid-edges"
            );
        if (avoid_edges)
            glyph_sym.set_avoid_edges(*avoid_edges);

        // displacement
        optional<double> dx = get_opt_attr<double>(sym, "dx");
        optional<double> dy = get_opt_attr<double>(sym, "dy");
        if (dx && dy)
            glyph_sym.set_displacement(*dx, *dy);

        // colorizer
        ptree::const_iterator childIter = sym.begin();
        ptree::const_iterator endChild = sym.end();

        for (; childIter != endChild; ++childIter)
        {
            ptree::value_type const& tag = *childIter;

            if (tag.first == "RasterColorizer")
            {
                raster_colorizer_ptr colorizer(new raster_colorizer());
                glyph_sym.set_colorizer(colorizer);
                parse_raster_colorizer(colorizer, tag.second);
            }
            else if (tag.first!="<xmlcomment>" && tag.first!="<xmlattr>" )
            {
                throw config_error(std::string("Unknown child node. ") +
                                   "Expected 'RasterColorizer' but got '" +
                                   tag.first + "'");
            }
        }

        parse_metawriter_in_symbolizer(glyph_sym, sym);
        rule.append(glyph_sym);
    }
    catch (const config_error & ex)
    {
        ex.append_context("in GlyphSymbolizer");
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
                ensure_attrs(stop_tag.second, "stop", "color,mode,value");
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


                //append the stop
                colorizer_stop tmpStop;
                tmpStop.set_color(*stopcolor);
                tmpStop.set_mode(mode);
                tmpStop.set_value(*value);
                
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
            boost::filesystem::path full = boost::filesystem::absolute(xml_path.branch_path()/rel_path).normalize();
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
    //typedef boost::property_tree::xml_parser::xmlattr<Ch> x_att;
    
    std::set<std::string> attr_set;
    boost::split(attr_set, attrs, boost::is_any_of(","));
    for (ptree::const_iterator itr = sym.begin(); itr != sym.end(); ++itr)
    {
       //ptree::value_type const& v = *itr;
       if (itr->first == boost::property_tree::xml_parser::xmlattr<Ch>())
       {
           optional<const ptree &> attribs = sym.get_child_optional( boost::property_tree::xml_parser::xmlattr<Ch>() );
           if (attribs)
           {
               std::ostringstream s("");
               s << "### " << name << " properties warning: ";
               int missing = 0;
               for (ptree::const_iterator it = attribs.get().begin(); it != attribs.get().end(); ++it)
               {
                   std::string name = it->first;
                   bool found = (attr_set.find(name) != attr_set.end());
                   if (!found)
                   {
                       if (missing) s << ",";
                       s << "'" << name << "'";
                       ++missing;
                   }
               }
               if (missing) {
                   if (missing > 1) s << " are";
                   else s << " is";
                   s << " invalid, acceptable values are:\n'" << attrs << "'\n";
                   std::clog << s.str();
               }
           }
       }
   }
}

} // end of namespace mapnik
