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

#include <mapnik/color_factory.hpp>

#include <mapnik/svg/svg_parser.hpp>
#include <mapnik/svg/svg_path_parser.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

namespace mapnik { namespace svg {


namespace qi = boost::spirit::qi;

typedef std::vector<std::pair<std::string, std::string> > pairs_type;

template <typename Iterator,typename SkipType>
struct key_value_sequence_ordered 
    : qi::grammar<Iterator, pairs_type(), SkipType>
{
    key_value_sequence_ordered()
        : key_value_sequence_ordered::base_type(query)
    {
        query =  pair >> *( qi::lit(';') >> pair);
        pair  =  key >> -(':' >> value);
        key   =  qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9-");
        value = +(qi::char_ - qi::lit(';'));
    }
    
    qi::rule<Iterator, pairs_type(), SkipType> query;
    qi::rule<Iterator, std::pair<std::string, std::string>(), SkipType> pair;
    qi::rule<Iterator, std::string(), SkipType> key, value;
};

agg::rgba8 parse_color(const char* str)
{
    mapnik::color c(100,100,100);
    try
    {
        mapnik::color_factory::init_from_string(c,str);
    }
    catch (mapnik::config_error & ex) 
    {
        std::cerr << ex.what() << std::endl;
    }
    return agg::rgba8(c.red(), c.green(), c.blue(), c.alpha());
}

double parse_double(const char* str)
{
    using namespace boost::spirit::qi;
    double val = 0.0;
    parse(str, str+ strlen(str),double_,val);
    return val;    
}

bool parse_style (const char* str, pairs_type & v)
{
    using namespace boost::spirit::qi;
    typedef boost::spirit::ascii::space_type skip_type;
    key_value_sequence_ordered<const char*, skip_type> kv_parser;
    return phrase_parse(str, str + strlen(str), kv_parser, skip_type(), v);
}

svg_parser::svg_parser(svg_converter<agg::path_storage,agg::pod_bvector<mapnik::svg::path_attributes> > & path)
    : path_(path) {}
   
svg_parser::~svg_parser() {}

void svg_parser::parse(std::string const& filename)
{
    xmlTextReaderPtr reader = xmlNewTextReaderFilename(filename.c_str());
    if (reader != 0) 
    {
        int ret = xmlTextReaderRead(reader);
        while (ret == 1) 
        {
            process_node(reader);
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) 
        {
            std::cerr << "Failed to parse " << filename << std::endl;
        }
    } else {
        std::cerr << "Unable to open " <<  filename << std::endl;
    }
}

void svg_parser::process_node(xmlTextReaderPtr reader)
{
    int node_type = xmlTextReaderNodeType(reader);
    switch (node_type)
    {
    case 1: //start element
        start_element(reader);
        break;
    case 15:// end element
        end_element(reader);
        break;
    default:
        break;
    }
}

void svg_parser::start_element(xmlTextReaderPtr reader)
{
    const xmlChar *name;
    name = xmlTextReaderConstName(reader);   
    
    
    if (xmlStrEqual(name, BAD_CAST "g"))
    {
        path_.push_attr();
        parse_attr(reader);
    }
    else if (xmlStrEqual(name, BAD_CAST "path"))
    {
        parse_path(reader);
    } 
    else if (xmlStrEqual(name, BAD_CAST "polygon"))
    {
        parse_polygon(reader);
    } 
    else if (xmlStrEqual(name, BAD_CAST "polyline"))
    {
        parse_polyline(reader);
    }
    else if (xmlStrEqual(name, BAD_CAST "line"))
    {
        parse_line(reader);
    } 
    else if (xmlStrEqual(name, BAD_CAST "rect"))
    {
        parse_rect(reader);
    } 
    else if (xmlStrEqual(name, BAD_CAST "circle"))
    {
        parse_circle(reader);
    }
    else if (xmlStrEqual(name, BAD_CAST "ellipse"))
    {
        parse_ellipse(reader);
    }
}

void svg_parser::end_element(xmlTextReaderPtr reader)
{
    const xmlChar *name;
    name = xmlTextReaderConstName(reader);   
    if (xmlStrEqual(name, BAD_CAST "g"))
    {
        path_.pop_attr();
    }
}

void svg_parser::parse_attr(const xmlChar * name, const xmlChar * value )
{
    if (xmlStrEqual(name, BAD_CAST "transform"))
    {
        agg::trans_affine tr;
        std::string wkt((const char*) value);
        mapnik::svg::parse_transform(wkt,tr);
        path_.transform().premultiply(tr);
    }
    else if (xmlStrEqual(name, BAD_CAST "fill"))
    {
        if (xmlStrEqual(value, BAD_CAST "none"))
        {
            path_.fill_none();
        }
        else
        {
            path_.fill(parse_color((const char*) value));
        }
    }
    else if (xmlStrEqual(name, BAD_CAST "fill-opacity"))
    {
        path_.fill_opacity(parse_double((const char*) value));  
    }
    else if (xmlStrEqual(name, BAD_CAST "fill-rule"))
    {
        if (xmlStrEqual(value, BAD_CAST "evenodd"))
        {
            path_.even_odd(true);
        }
    }
    else if (xmlStrEqual(name, BAD_CAST "stroke"))
    {
        if (xmlStrEqual(value, BAD_CAST "none"))
        {
            path_.stroke_none();
        }
        else
        {
            path_.stroke(parse_color((const char*) value));
        }
    }
    else if (xmlStrEqual(name, BAD_CAST "stroke-width"))
    {
        path_.stroke_width(parse_double((const char*)value));
    }
    else if (xmlStrEqual(name, BAD_CAST "stroke-opacity"))
    {
        path_.stroke_opacity(parse_double((const char*)value));
    }
    else if(xmlStrEqual(name,BAD_CAST "stroke-width"))
    {
        path_.stroke_width(parse_double((const char*) value));
    }
    else if(xmlStrEqual(name,BAD_CAST "stroke-linecap"))
    {
        if(xmlStrEqual(value,BAD_CAST "butt"))        
            path_.line_cap(agg::butt_cap);
        else if(xmlStrEqual(value,BAD_CAST "round"))  
            path_.line_cap(agg::round_cap);
        else if(xmlStrEqual(value,BAD_CAST "square")) 
            path_.line_cap(agg::square_cap);
    }
    else if(xmlStrEqual(name,BAD_CAST "stroke-linejoin"))
    {
        if(xmlStrEqual(value,BAD_CAST "miter"))     
            path_.line_join(agg::miter_join);
        else if(xmlStrEqual(value,BAD_CAST "round")) 
            path_.line_join(agg::round_join);
        else if(xmlStrEqual(value,BAD_CAST "bevel")) 
            path_.line_join(agg::bevel_join);
    }
    else if(xmlStrEqual(name,BAD_CAST "stroke-miterlimit"))
    {
        path_.miter_limit(parse_double((const char*)value));
    }
    
    else if(xmlStrEqual(name, BAD_CAST "opacity"))
    {
        double opacity = parse_double((const char*)value);
        path_.opacity(opacity);
    }
}


void svg_parser::parse_attr(xmlTextReaderPtr reader)
{
    const xmlChar *name, *value;
    while (xmlTextReaderMoveToNextAttribute(reader))
    {
        name = xmlTextReaderConstName(reader);
        value = xmlTextReaderConstValue(reader);
        if (xmlStrEqual(name, BAD_CAST "style"))
        {
            typedef std::vector<std::pair<std::string,std::string> > cont_type; 
            typedef cont_type::value_type value_type;
            cont_type vec;
            parse_style((const char*)value, vec);
            BOOST_FOREACH(value_type kv , vec )
            {
                parse_attr(BAD_CAST kv.first.c_str(),BAD_CAST kv.second.c_str()); 
            }       
        }
        else
        {
            parse_attr(name,value);
        }
    }
}
void svg_parser::parse_path(xmlTextReaderPtr reader)
{
    const xmlChar *value;

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "d");
    if (value) 
    {
        path_.begin_path();
        parse_attr(reader);

        std::string wkt((const char*) value);
        if (!mapnik::svg::parse_path(wkt, path_))
        {
            std::runtime_error("can't parse PATH\n");
        }
        path_.end_path();
    }
}

void svg_parser::parse_polygon(xmlTextReaderPtr reader)
{
    const xmlChar *value;
    
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "points");
    if (value) 
    {
        path_.begin_path();
        parse_attr(reader);
        if (!mapnik::svg::parse_points((const char*) value, path_))
        {
            throw std::runtime_error("Failed to parse <polygon>\n");
        }
        path_.close_subpath();
        path_.end_path();
    }
}

void svg_parser::parse_polyline(xmlTextReaderPtr reader)
{
    const xmlChar *value;
    
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "points");
    if (value) 
    {
        path_.begin_path();
        parse_attr(reader);
        if (!mapnik::svg::parse_points((const char*) value, path_))
        {
            throw std::runtime_error("Failed to parse <polygon>\n");
        }
        
        path_.end_path();
    }
}

void svg_parser::parse_line(xmlTextReaderPtr reader)
{
    const xmlChar *value;
    double x1 = 0.0;
    double y1 = 0.0;
    double x2 = 0.0;
    double y2 = 0.0;
    
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "x1");
    if (value) x1 = parse_double((const char*)value);
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "y1");
    if (value) y1 = parse_double((const char*)value);
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "x2");
    if (value) x2 = parse_double((const char*)value);
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "y2");
    if (value) y2 = parse_double((const char*)value);
    
    path_.begin_path();    
    parse_attr(reader);
    path_.move_to(x1, y1);
    path_.line_to(x2, y2);
    path_.end_path();
    
}

void svg_parser::parse_circle(xmlTextReaderPtr reader)
{
    const xmlChar *value;
    double cx = 0.0;
    double cy = 0.0;
    double r = 0.0;
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "cx");
    if (value) cx = parse_double((const char*)value);
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "cy");
    if (value) cy = parse_double((const char*)value);
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "r");
    if (value) r = parse_double((const char*)value);

    path_.begin_path();     
    parse_attr(reader);
    
    if(r != 0.0)
    {
        if(r < 0.0) throw std::runtime_error("parse_circle: Invalid radius");
        path_.move_to(cx+r,cy);
        path_.arc_to(r,r,0,1,0,cx-r,cy);
        path_.arc_to(r,r,0,1,0,cx+r,cy);
    }
    
    path_.end_path();
}

void svg_parser::parse_ellipse(xmlTextReaderPtr reader)
{
    const xmlChar *value;
    double cx = 0.0;
    double cy = 0.0;
    double rx = 0.0;
    double ry = 0.0;

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "cx");
    if (value) cx = parse_double((const char*)value);
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "cy");
    if (value) cy = parse_double((const char*)value);
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "rx");
    if (value) rx = parse_double((const char*)value);
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "ry");
    if (value) ry = parse_double((const char*)value);
    
    path_.begin_path();   
    parse_attr(reader);
    
    if(rx != 0.0 && ry != 0.0)
    {
        if(rx < 0.0) throw std::runtime_error("parse_ellipse: Invalid rx");
        if(ry < 0.0) throw std::runtime_error("parse_ellipse: Invalid ry");
        path_.move_to(cx+rx,cy);
        path_.arc_to(rx,ry,0,1,0,cx-rx,cy);
        path_.arc_to(rx,ry,0,1,0,cx+rx,cy);
    }
    
    path_.end_path();
}

void svg_parser::parse_rect(xmlTextReaderPtr reader)
{
    const xmlChar *value;
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;
    double rx = 0.0;
    double ry = 0.0;
 
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "x");
    if (value) x = parse_double((const char*)value);
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "y");
    if (value) y = parse_double((const char*)value);
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "width");
    if (value) w = parse_double((const char*)value);
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "height");
    if (value) h = parse_double((const char*)value);
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "rx");
    if (value) rx = parse_double((const char*)value);
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "ry");
    if (value) ry = parse_double((const char*)value);

    if(w != 0.0 && h != 0.0)
    {
        if(w < 0.0) throw std::runtime_error("parse_rect: Invalid width");
        if(h < 0.0) throw std::runtime_error("parse_rect: Invalid height");
        if(rx < 0.0) throw std::runtime_error("parse_rect: Invalid rx");
        if(ry < 0.0) throw std::runtime_error("parse_rect: Invalid ry");
        
        path_.begin_path();
        parse_attr(reader);
        
        if(rx > 0.0 && ry > 0.0)
        {
            path_.move_to(x + rx,y);
            path_.line_to(x + w -rx,y);         
            path_.arc_to (rx,ry,0,0,1,x + w, y + ry);
            path_.line_to(x + w, y + h - ry);
            path_.arc_to (rx,ry,0,0,1,x + w - rx, y + h);
            path_.line_to(x + rx, y + h);
            path_.arc_to(rx,ry,0,0,1,x,y + h - ry);
            path_.line_to(x,y+ry);
            path_.arc_to(rx,ry,0,0,1,x + rx,y);
        }
        else
        {
            path_.move_to(x,     y);
            path_.line_to(x + w, y);
            path_.line_to(x + w, y + h);
            path_.line_to(x,     y + h);
            path_.close_subpath();
        }
        path_.end_path();
    }
}

}}
