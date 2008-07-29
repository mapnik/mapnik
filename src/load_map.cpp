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
// mapnik
#include <mapnik/load_map.hpp>

#include <mapnik/image_reader.hpp>
#include <mapnik/color.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/font_set.hpp>

#include <mapnik/ptree_helpers.hpp>
#include <mapnik/libxml2_loader.hpp>

// boost
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/static_assert.hpp>
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
         map_parser( bool strict ) : 
            strict_( strict ),
            font_manager_(font_engine_) {}
         
         void parse_map( Map & map, ptree const & sty);
      private:
         void parse_style( Map & map, ptree const & sty);
         void parse_layer( Map & map, ptree const & lay);
        
         void parse_fontset(Map & map, ptree const & fset);
         void parse_font(FontSet & fset, ptree const & f);
     
         void parse_rule( feature_type_style & style, ptree const & r);
         
         void parse_point_symbolizer( rule_type & rule, ptree const & sym);
         void parse_line_pattern_symbolizer( rule_type & rule, ptree const & sym);
         void parse_polygon_pattern_symbolizer( rule_type & rule, ptree const & sym);
         void parse_text_symbolizer( rule_type & rule, ptree const & sym);
         void parse_shield_symbolizer( rule_type & rule, ptree const & sym);
         void parse_line_symbolizer( rule_type & rule, ptree const & sym);
         void parse_polygon_symbolizer( rule_type & rule, ptree const & sym);
         void parse_building_symbolizer( rule_type & rule, ptree const & sym );
         void parse_markers_symbolizer( rule_type & rule, ptree const & sym );
         
         void ensure_font_face( const std::string & face_name );
         
         bool strict_;
         std::map<std::string,parameters> datasource_templates_;
         freetype_engine font_engine_;
         face_manager<freetype_engine> font_manager_;
         std::map<std::string,std::string> file_sources_;
         std::map<std::string,FontSet> fontsets_;
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

        map_parser parser( strict );
        parser.parse_map(map, pt);
    }

    void map_parser::parse_map( Map & map, ptree const & pt )
    {
        try
        {
            ptree const & map_node = pt.get_child("Map");

            try
            {
                optional<Color> bgcolor = get_opt_attr<Color>(map_node, "bgcolor");
                if (bgcolor) {
                    map.set_background( * bgcolor );
                }

                map.set_srs( get_attr(map_node, "srs", map.srs() ));
            }
            catch (const config_error & ex)
            {
                ex.append_context("in node Map");
                throw;
            }

            ptree::const_iterator itr = map_node.begin();
            ptree::const_iterator end = map_node.end();

            for (; itr != end; ++itr)
            {
                ptree::value_type const& v = *itr;

                if (v.first == "Style")
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
                else if (v.first == "FileSource")
                {
                  std::string name = get_attr<string>( v.second, "name");
                  std::string value = get_own<string>( v.second, "");
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
                            std::string value = get_own<string>( param,
                                    "datasource parameter");
                            params[name] = value; 
                        }
                        else if( paramIter->first != "<xmlattr>" )
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
                    throw config_error(std::string("Unknown child node in 'Map'. ") +
                            "Expected 'Style' or 'Layer' but got '" + v.first + "'");
                }
            }
        }
        catch (const boost::property_tree::ptree_bad_path &)
        {
            throw config_error("Not a map file. Node 'Map' not found.");
        }
    }

    void map_parser::parse_style( Map & map, ptree const & sty )
    {
        string name("<missing name>");
        try
        {
            name = get_attr<string>(sty, "name");
            feature_type_style style;

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
                    throw config_error(std::string("Unknown child node in 'Style'.") +
                            "Expected 'Rule' but got '" + rule_tag.first + "'");
                }
            }

            map.insert_style(name, style);

        } catch (const config_error & ex) {
            if ( ! name.empty() ) {
                ex.append_context(string("in style '") + name + "'");
            }
            throw;
        }
    }

    void map_parser::parse_fontset( Map & map, ptree const & fset )
    {
        string name("<missing name>");
        try
        {
            name = get_attr<string>(fset, "name");
            FontSet fontset(name);

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
                    throw config_error(std::string("Unknown child node in 'FontSet'.") +
                        "Expected 'Font' but got '" + font_tag.first + "'");
                }
            }

            map.insert_fontset(name, fontset);
				
            // XXX Hack because map object isn't accessible by text_symbolizer 
            // when it's parsed
            fontsets_.insert(pair<std::string, FontSet>(name, fontset));
        } catch (const config_error & ex) {
            if ( ! name.empty() ) {
                ex.append_context(string("in FontSet '") + name + "'");
            }
            throw;
        }
    }

    void map_parser::parse_font(FontSet & fset, ptree const & f)
	{
        std::string face_name;

        try
        {
            face_name = get_attr(f, "face_name", string());
        
            if ( strict_ )
            {
                ensure_font_face( face_name );
            }
        }
        catch (const config_error & ex)
        {
            if (!face_name.empty())
            {
                ex.append_context(string("in Font '") + face_name + "'");
            }
            throw;
        }

        fset.add_face_name(face_name);
	 }

    void map_parser::parse_layer( Map & map, ptree const & lay )
    {
        std::string name;
        try
        {
            name = get_attr(lay, "name", string("Unnamed"));
            // XXX if no projection is given inherit from map? [DS]
            std::string srs = get_attr(lay, "srs", map.srs());

            Layer lyr(name, srs);

            optional<boolean> status = get_opt_attr<boolean>(lay, "status");
            if (status)
            {
                lyr.setActive( * status );
            }

            optional<boolean> clear_cache =
                get_opt_attr<boolean>(lay, "clear_label_cache");
            if (clear_cache)
            {
                lyr.set_clear_label_cache( * clear_cache );
            }


            ptree::const_iterator itr2 = lay.begin();
            ptree::const_iterator end2 = lay.end();

            for(; itr2 != end2; ++itr2)
            {
                ptree::value_type const& child = *itr2;

                if (child.first == "StyleName")
                {
                    // TODO check references [DS]
                    lyr.add_style(child.second.data());
                }
                else if (child.first == "Datasource")
                {
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
                            std::string name = get_attr<string>(param, "name");
                            std::string value = get_own<string>( param,
                                    "datasource parameter");
                            params[name] = value; 
                        }
                        else if( paramIter->first != "<xmlattr>" )
                        {
                            throw config_error(std::string("Unknown child node in ") +
                                    "'Datasource'. Expected 'Parameter' but got '" +
                                    paramIter->first + "'");
                        }
                    }
                    //now we're ready to create datasource 
                    try 
                    {
                        boost::shared_ptr<datasource> ds =
                            datasource_cache::instance()->create(params);
                        lyr.set_datasource(ds);
                    }
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

        } catch (const config_error & ex) {
            if ( ! name.empty() ) {
                ex.append_context(string("in layer '") + name + "'");
            }
            throw;
        }
    }

    void map_parser::parse_rule( feature_type_style & style, ptree const & r )
    {
        std::string name;
        try
        {
            name = get_attr( r, "name", string());
            std::string title = get_attr( r, "title", string());

            rule_type rule(name,title);

            optional<std::string> filter_expr =
                get_opt_child<string>( r, "Filter");
            if (filter_expr)
            {
               // can we use encoding defined for XML document for filter expressions?
               rule.set_filter(create_filter(*filter_expr,"utf8"));
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
                    rule.append(raster_symbolizer());
                } 
                else if ( sym.first == "MarkersSymbolizer")
                {
                    rule.append(markers_symbolizer());
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
                ex.append_context(string("in rule '") + name + "'");
            }
            throw;
        }
    }

    void map_parser::parse_point_symbolizer( rule_type & rule, ptree const & sym )
    {
        try 
        {
            optional<std::string> file =  get_opt_attr<string>(sym, "file");
            optional<std::string> base =  get_opt_attr<string>(sym, "base");
            optional<std::string> type =  get_opt_attr<string>(sym, "type"); 
            optional<boolean> allow_overlap = 
                get_opt_attr<boolean>(sym, "allow_overlap");

            optional<unsigned> width = get_opt_attr<unsigned>(sym, "width"); 
            optional<unsigned> height = get_opt_attr<unsigned>(sym, "height"); 

            if (file && type && width && height)
            {
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
                   point_symbolizer symbol(*file,*type,*width,*height);
                   if (allow_overlap)
                   {
                      symbol.set_allow_overlap( * allow_overlap );
                   }
                   rule.append(symbol);
                }
                catch (ImageReaderException const & ex )
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
            else if (file || type || width || height)
            {
                std::ostringstream os;
                os << "Missing required attributes: ";
                if ( ! file ) os << "file ";
                if ( ! type ) os << "type ";
                if ( ! width ) os << "width ";
                if ( ! height ) os << "height ";
                throw config_error( os.str() );
            }
            else 
            {
                rule.append(point_symbolizer());
            }
        }
        catch (const config_error & ex) 
        {
            ex.append_context("in PointSymbolizer");
            throw;
        }
    }

    void map_parser::parse_line_pattern_symbolizer( rule_type & rule, ptree const & sym )
    {
        try 
        {
            std::string file = get_attr<string>(sym, "file");
            optional<std::string> base = get_opt_attr<string>(sym, "base");
            std::string type = get_attr<string>(sym, "type"); 
            unsigned width = get_attr<unsigned>(sym, "width");
            unsigned height = get_attr<unsigned>(sym, "height");

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
               rule.append(line_pattern_symbolizer(file,type,width,height));
            }
            catch (ImageReaderException const & ex )
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

    void map_parser::parse_polygon_pattern_symbolizer( rule_type & rule,
                ptree const & sym )
    {
        try 
        {
            std::string file = get_attr<string>(sym, "file");
            optional<std::string> base = get_opt_attr<string>(sym, "base");
            std::string type = get_attr<string>(sym, "type"); 
            unsigned width = get_attr<unsigned>(sym, "width");
            unsigned height = get_attr<unsigned>(sym, "height");

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
                rule.append(polygon_pattern_symbolizer(file,type,width,height)); 
            }
            catch (ImageReaderException const & ex )
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

    void map_parser::parse_text_symbolizer( rule_type & rule, ptree const & sym )
    {
        try
        {
            std::string name = get_attr<string>(sym, "name"); 

            optional<std::string> face_name =
                 get_opt_attr<std::string>(sym, "face_name");
            
            optional<std::string> fontset_name =
                 get_opt_attr<std::string>(sym, "fontset_name");

            unsigned size = get_attr(sym, "size", 10U);

            Color c = get_attr(sym, "fill", Color(0,0,0));
            
            text_symbolizer text_symbol = text_symbolizer(name, size, c);                
            
            if (fontset_name && face_name)
            {
                throw config_error(std::string("Can't have both face_name and fontset_name"));
            }
            else if (fontset_name)
            {
                std::map<std::string,FontSet>::const_iterator itr = fontsets_.find(*fontset_name);
                if (itr != fontsets_.end())
                {
                    text_symbol.set_fontset(itr->second);                
                }
            }
            else if (face_name)
            {
                text_symbol.set_face_name(*face_name);                
            }
            else
            {
                throw config_error(std::string("Must have face_name or fontset_name"));
            }

            int dx = get_attr(sym, "dx", 0);
            int dy = get_attr(sym, "dy", 0);
            text_symbol.set_displacement(dx,dy);

            label_placement_e placement =
                    get_attr<label_placement_e>(sym, "placement", POINT_PLACEMENT);
            text_symbol.set_label_placement( placement );

            // halo fill and radius
            optional<Color> halo_fill = get_opt_attr<Color>(sym, "halo_fill");
            if (halo_fill)
            {
                text_symbol.set_halo_fill( * halo_fill );
            }
            optional<unsigned> halo_radius = 
                get_opt_attr<unsigned>(sym, "halo_radius");
            if (halo_radius)
            {
                text_symbol.set_halo_radius(*halo_radius);
            }

            // text ratio and wrap width
            optional<unsigned> text_ratio = 
                get_opt_attr<unsigned>(sym, "text_ratio");

            optional<unsigned> wrap_width = 
                get_opt_attr<unsigned>(sym, "wrap_width");
            if (wrap_width)
            {
                text_symbol.set_wrap_width(*wrap_width);
            }

            // spacing between repeated labels on lines
            optional<unsigned> spacing = get_opt_attr<unsigned>(sym, "spacing");
            if (spacing)
            {
                text_symbol.set_label_spacing(*spacing);
            }

            // minimum distance between labels
            optional<unsigned> min_distance =
                get_opt_attr<unsigned>(sym, "min_distance");
            if (min_distance)
            {
                text_symbol.set_minimum_distance(*min_distance);
            }

            // don't render labels around edges
            optional<boolean> avoid_edges =
                get_opt_attr<boolean>(sym, "avoid_edges");
            if (avoid_edges)
            {
                text_symbol.set_avoid_edges( * avoid_edges);
            }

            // allow_overlap 
            optional<boolean> allow_overlap = 
                get_opt_attr<boolean>(sym, "allow_overlap");
            if (allow_overlap)
            {
                text_symbol.set_allow_overlap( * allow_overlap );
            }
            
            // max_char_angle_delta
            optional<double> max_char_angle_delta = 
               get_opt_attr<double>(sym, "max_char_angle_delta");
            if (max_char_angle_delta)
            {
               text_symbol.set_max_char_angle_delta( * max_char_angle_delta);
            }
               
            if ( strict_ )
            {
                ensure_font_face( text_symbol.get_face_name() );
            }

            rule.append(text_symbol);
        }
        catch (const config_error & ex) 
        {
            ex.append_context("in TextSymbolizer");
            throw;
        }
    }

    void map_parser::parse_shield_symbolizer( rule_type & rule, ptree const & sym )
    {
        try
        {
            std::string name =  get_attr<string>(sym, "name");

            optional<std::string> face_name =
                 get_opt_attr<std::string>(sym, "face_name");

            optional<std::string> fontset_name =
                 get_opt_attr<std::string>(sym, "fontset_name");

            unsigned size = get_attr(sym, "size", 10U);
            Color fill = get_attr(sym, "fill", Color(0,0,0));

            std::string image_file = get_attr<string>(sym, "file");
            optional<std::string> base = get_opt_attr<string>(sym, "base");
            std::string type = get_attr<string>(sym, "type");
            unsigned width =  get_attr<unsigned>(sym, "width");
            unsigned height =  get_attr<unsigned>(sym, "height");

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
                shield_symbolizer shield_symbol(name,size,fill,
                                                image_file,type,width,height);

                if (fontset_name && face_name)
                {
                    throw config_error(std::string("Can't have both face_name and fontset_name"));
                }
                else if (fontset_name)
                {
                    std::map<std::string,FontSet>::const_iterator itr = fontsets_.find(*fontset_name);
                    if (itr != fontsets_.end())
                    {
                        shield_symbol.set_fontset(itr->second);                
                    }
                }
                else if (face_name)
                {
                    shield_symbol.set_face_name(*face_name);                
                }
                else
                {
                    throw config_error(std::string("Must have face_name or fontset_name"));
                }

                // minimum distance between labels
                optional<unsigned> min_distance = 
                    get_opt_attr<unsigned>(sym, "min_distance");
                if (min_distance)
                {
                    shield_symbol.set_minimum_distance(*min_distance);
                }
                rule.append(shield_symbol);
            }
            catch (ImageReaderException const & ex )
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

    void map_parser::parse_line_symbolizer( rule_type & rule, ptree const & sym )
    {
        try
        {
            stroke strk;
            ptree::const_iterator cssIter = sym.begin();
            ptree::const_iterator endCss = sym.end();

            for(; cssIter != endCss; ++cssIter)
            {
                ptree const & css = cssIter->second;
                std::string css_name  = get_attr<string>(css, "name");
                if (css_name == "stroke")
                {
                    Color c = get_css<Color>(css, css_name);
                    strk.set_color(c);
                }
                else if (css_name == "stroke-width")
                {
                    float width = get_css<float>(css, css_name);
                    strk.set_width(width);
                }
                else if (css_name == "stroke-opacity")
                {
                    float opacity = get_css<float>(css, css_name);
                    strk.set_opacity(opacity);
                }
                else if (css_name == "stroke-linejoin")
                {
                    line_join_e line_join = get_css<line_join_e>(css, css_name);
                    strk.set_line_join( line_join );
                }
                else if (css_name == "stroke-linecap")
                {
                    line_cap_e line_cap = get_css<line_cap_e>(css, css_name);
                    strk.set_line_cap( line_cap );
                }
                else if (css_name == "stroke-dasharray")
                {
                    tokenizer<> tok ( css.data() );
                    std::vector<float> dash_array;
                    tokenizer<>::iterator itr = tok.begin();
                    for (; itr != tok.end(); ++itr)
                    {
                        try 
                        {
                            float f = boost::lexical_cast<float>(*itr);
                            dash_array.push_back(f);
                        }
                        catch ( boost::bad_lexical_cast &)
                        {
                            throw config_error(std::string("Failed to parse CSS ") +
                                    "parameter '" + css_name + "'. Expected a " +
                                    "list of floats but got '" + css.data() + "'");
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
                        std::vector<float>::const_iterator pos = dash_array.begin();
                        while (pos != dash_array.end())
                        {
                            strk.add_dash(*pos,*(pos + 1));
                            pos +=2;
                        }
                    }
                }
            }
            rule.append(line_symbolizer(strk));
        }
        catch (const config_error & ex) 
        {
            ex.append_context("in LineSymbolizer");
            throw;
        }
    }


    void map_parser::parse_polygon_symbolizer( rule_type & rule, ptree const & sym )
    {
        try
        {
            polygon_symbolizer poly_sym;

            ptree::const_iterator cssIter = sym.begin();
            ptree::const_iterator endCss = sym.end();

            for(; cssIter != endCss; ++cssIter)
            {
                ptree const & css = cssIter->second;
                std::string css_name  = get_attr<string>(css, "name");
                if (css_name == "fill")
                {
                    Color c = get_css<Color>(css, css_name);
                    poly_sym.set_fill(c);
                }
                else if (css_name == "fill-opacity")
                {
                    float opacity = get_css<float>(css, css_name);
                    poly_sym.set_opacity(opacity);
                }
            }
            rule.append(poly_sym);

        }
        catch (const config_error & ex) 
        {
            ex.append_context("in PolygonSymbolizer");
            throw;
        }
    }


    void map_parser::parse_building_symbolizer( rule_type & rule, ptree const & sym )
    {
        try {
            building_symbolizer building_sym;

            ptree::const_iterator cssIter = sym.begin();
            ptree::const_iterator endCss = sym.end();

            for(; cssIter != endCss; ++cssIter)
            {
                ptree const& css = cssIter->second;

                std::string css_name  = get_attr<string>(css, "name");
                std::string data = css.data();
                if (css_name == "fill")
                {
                    Color c = get_css<Color>(css, css_name);
                    building_sym.set_fill(c);
                }
                else if (css_name == "fill-opacity")
                {
                    float opacity = get_css<float>(css, css_name);
                    building_sym.set_opacity(opacity);
                }
                else if (css_name == "height")
                {
                   float height = get_css<float>(css,css_name);
                   building_sym.set_height(height);
                }
            }
            rule.append(building_sym);
        }
        catch (const config_error & ex) 
        {
            ex.append_context("in BuildingSymbolizer");
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
} // end of namespace mapnik
