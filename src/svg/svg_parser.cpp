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

#include <mapnik/debug.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/svg/svg_parser.hpp>
#include <mapnik/svg/svg_path_parser.hpp>
#include <mapnik/config_error.hpp>

#include "agg_ellipse.h"
#include "agg_rounded_rect.h"
#include "agg_span_gradient.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

namespace mapnik { namespace svg {


typedef std::vector<std::pair<double, agg::rgba8> > color_lookup_type;

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
        c = mapnik::parse_color(str);
    }
    catch (mapnik::config_error & ex)
    {
        MAPNIK_LOG_ERROR(svg_parser) << ex.what();
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

/*
 * parse a double that might end with a %
 * if it does then set the ref bool true and divide the result by 100
 */
double parse_double_optional_percent(const char* str, bool &percent)
{
    using namespace boost::spirit::qi;
    using boost::phoenix::ref;
    using qi::_1;

    double val = 0.0;
    char unit='\0';
    parse(str, str+ strlen(str),double_[ref(val)=_1] >> *char_('%')[ref(unit)=_1]);
    if (unit =='%')
    {
        percent = true;
        val/=100.0;
    }
    else
    {
        percent = false;
    }
    return val;
}

bool parse_style (const char* str, pairs_type & v)
{
    using namespace boost::spirit::qi;
    typedef boost::spirit::ascii::space_type skip_type;
    key_value_sequence_ordered<const char*, skip_type> kv_parser;
    return phrase_parse(str, str + strlen(str), kv_parser, skip_type(), v);
}

svg_parser::svg_parser(svg_converter<svg_path_adapter,
                       agg::pod_bvector<mapnik::svg::path_attributes> > & path)
    : path_(path),
      is_defs_(false)
{}

svg_parser::~svg_parser() {}

void svg_parser::parse(std::string const& filename)
{
    xmlTextReaderPtr reader = xmlNewTextReaderFilename(filename.c_str());
    if (reader == NULL)
    {
        MAPNIK_LOG_ERROR(svg_parser) << "Unable to open '" << filename << "'";
    }
    else if (!parse_reader(reader))
    {
        MAPNIK_LOG_ERROR(svg_parser) << "Unable to parse '" << filename << "'";
    }
}

void svg_parser::parse_from_string(std::string const& svg)
{
    xmlTextReaderPtr reader = xmlReaderForMemory(svg.c_str(),svg.size(),NULL,NULL,
        (XML_PARSE_NOBLANKS | XML_PARSE_NOCDATA | XML_PARSE_NOERROR | XML_PARSE_NOWARNING));
    if (reader == NULL)
    {
        MAPNIK_LOG_ERROR(svg_parser) << "Unable to parse '" << svg << "'";
    }
    else if (!parse_reader(reader))
    {
        MAPNIK_LOG_ERROR(svg_parser) << "Unable to parse '" << svg << "'";
    }
}

bool svg_parser::parse_reader(xmlTextReaderPtr reader)
{
    int ret = xmlTextReaderRead(reader);
    try {
        while (ret == 1)
        {
            process_node(reader);
            ret = xmlTextReaderRead(reader);
        }
    }
    catch (std::exception const& ex)
    {
        xmlFreeTextReader(reader);
        throw;
    }
    xmlFreeTextReader(reader);
    if (ret != 0)
    {
        // parsing failure
        return false;
    }
    return true;
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

    if (xmlStrEqual(name, BAD_CAST "defs"))
    {
        if (xmlTextReaderIsEmptyElement(reader) == 0)
            is_defs_ = true;
    }
    // the gradient tags *should* be in defs, but illustrator seems not to put them in there so
    // accept them anywhere
    else if (xmlStrEqual(name, BAD_CAST "linearGradient"))
    {
        parse_linear_gradient(reader);
    }
    else if (xmlStrEqual(name, BAD_CAST "radialGradient"))
    {
        parse_radial_gradient(reader);
    }
    else if (xmlStrEqual(name, BAD_CAST "stop"))
    {
        parse_gradient_stop(reader);
    }
    if ( !is_defs_ )
    {

        if (xmlStrEqual(name, BAD_CAST "g"))
        {
            path_.push_attr();
            parse_attr(reader);
        }
        else
        {
            path_.push_attr();
            parse_attr(reader);
            if (path_.display())
            {
                if (xmlStrEqual(name, BAD_CAST "path"))
                {
                    parse_path(reader);
                }            
                else if (xmlStrEqual(name, BAD_CAST "polygon") )
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
#ifdef MAPNIK_LOG
                else if (!xmlStrEqual(name, BAD_CAST "svg"))
                {
                    MAPNIK_LOG_WARN(svg_parser) << "svg_parser: Unhandled svg element=" << name;
                }
#endif
            }
            path_.pop_attr();
        }
    }
}

void svg_parser::end_element(xmlTextReaderPtr reader)
{
    const xmlChar *name;
    name = xmlTextReaderConstName(reader);


    if (!is_defs_ && xmlStrEqual(name, BAD_CAST "g"))
    {
        path_.pop_attr();
    }
    else if (xmlStrEqual(name, BAD_CAST "defs"))
    {
        is_defs_ = false;
    }
    else if ((xmlStrEqual(name, BAD_CAST "linearGradient")) || (xmlStrEqual(name, BAD_CAST "radialGradient")))
    {
        gradient_map_[temporary_gradient_.first] = temporary_gradient_.second;
    }

}

void svg_parser::parse_attr(const xmlChar * name, const xmlChar * value )
{
    if (xmlStrEqual(name, BAD_CAST "transform"))
    {
        agg::trans_affine tr;
        mapnik::svg::parse_transform((const char*) value,tr);
        path_.transform().premultiply(tr);
    }
    else if (xmlStrEqual(name, BAD_CAST "fill"))
    {
        if (xmlStrEqual(value, BAD_CAST "none"))
        {
            path_.fill_none();
        }
        else if (boost::starts_with((const char*)value, "url(#"))
        {
            // see if we have a known gradient fill
            std::string id = std::string((const char*)&value[5]);
            // get rid of the trailing )
            id.erase(id.end()-1);
            if (gradient_map_.count(id) > 0)
            {
                path_.add_fill_gradient(gradient_map_[id]);
            }
            else
            {
                MAPNIK_LOG_ERROR(svg_parser) << "Failed to find gradient fill: " << id;
            }
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
        else if (boost::starts_with((const char*)value, "url(#"))
        {
            // see if we have a known gradient fill
            std::string id = std::string((const char*)&value[5]);
            // get rid of the trailing )
            id.erase(id.end()-1);
            if (gradient_map_.count(id) > 0)
            {
                path_.add_stroke_gradient(gradient_map_[id]);
            }
            else
            {
                MAPNIK_LOG_ERROR(svg_parser) << "Failed to find gradient fill: " << id;
            }
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
        path_.stroke_opacity(opacity);
        path_.fill_opacity(opacity);
    }
    else if (xmlStrEqual(name, BAD_CAST "visibility"))
    {
        path_.visibility(!xmlStrEqual(value, BAD_CAST "hidden"));
    }
    else if (xmlStrEqual(name, BAD_CAST "display") && xmlStrEqual(value, BAD_CAST "none"))
    {
        path_.display(false);
    }
}


void svg_parser::parse_attr(xmlTextReaderPtr reader)
{
    const xmlChar *name, *value;
    
    if (xmlTextReaderMoveToFirstAttribute(reader) == 1)
    {
        do
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
        } while(xmlTextReaderMoveToNextAttribute(reader) == 1);
    }
    xmlTextReaderMoveToElement(reader);
}
void svg_parser::parse_path(xmlTextReaderPtr reader)
{
    xmlChar *value;

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "d");
    if (value)
    {
        // d="" (empty paths) are valid
        if (strlen((const char*)value) < 1)
        {
            xmlFree(value);
        }
        else
        {
            path_.begin_path();

            if (!mapnik::svg::parse_path((const char*) value, path_))
            {
                xmlFree(value);
                xmlChar *id_value;
                id_value = xmlTextReaderGetAttribute(reader, BAD_CAST "id");
                if (id_value)
                {
                    std::string id_string((const char *) id_value);
                    xmlFree(id_value);
                    throw std::runtime_error(std::string("unable to parse invalid svg <path> with id '") + id_string + "'");
                }
                else
                {
                    throw std::runtime_error("unable to parse invalid svg <path>");
                }
            }
            path_.end_path();
            xmlFree(value);
        }
    }
}

void svg_parser::parse_polygon(xmlTextReaderPtr reader)
{
    xmlChar *value;

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "points");
    if (value)
    {
        path_.begin_path();
        if (!mapnik::svg::parse_points((const char*) value, path_))
        {
            xmlFree(value);
            throw std::runtime_error("Failed to parse <polygon>");
        }
        path_.close_subpath();
        path_.end_path();
        xmlFree(value);
    }
}

void svg_parser::parse_polyline(xmlTextReaderPtr reader)
{
    xmlChar *value;

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "points");
    if (value)
    {
        path_.begin_path();
        if (!mapnik::svg::parse_points((const char*) value, path_))
        {
            xmlFree(value);
            throw std::runtime_error("Failed to parse <polygon>");
        }

        path_.end_path();
        xmlFree(value);
    }
}

void svg_parser::parse_line(xmlTextReaderPtr reader)
{
    xmlChar *value;
    double x1 = 0.0;
    double y1 = 0.0;
    double x2 = 0.0;
    double y2 = 0.0;

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "x1");
    if (value) 
    {
        x1 = parse_double((const char*)value);
        xmlFree(value);
    }
    
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "y1");
    if (value) 
    {
        y1 = parse_double((const char*)value);
        xmlFree(value);
    }
    
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "x2");
    if (value) 
    {
        x2 = parse_double((const char*)value);
        xmlFree(value);
    }
    
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "y2");
    if (value) 
    {
        y2 = parse_double((const char*)value);
        xmlFree(value);
    }
    
    path_.begin_path();
    path_.move_to(x1, y1);
    path_.line_to(x2, y2);
    path_.end_path();

}

void svg_parser::parse_circle(xmlTextReaderPtr reader)
{
    xmlChar *value;
    double cx = 0.0;
    double cy = 0.0;
    double r = 0.0;
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "cx");
    if (value) 
    {
        cx = parse_double((const char*)value);
        xmlFree(value);
    }

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "cy");
    if (value) 
    {
        cy = parse_double((const char*)value);
        xmlFree(value);
    }
    
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "r");
    if (value) 
    {
        r = parse_double((const char*)value);
        xmlFree(value);
    }
    
    path_.begin_path();

    if(r != 0.0)
    {
        if(r < 0.0) throw std::runtime_error("parse_circle: Invalid radius");
        agg::ellipse c(cx, cy, r, r);
        path_.storage().concat_path(c);
    }

    path_.end_path();
}

void svg_parser::parse_ellipse(xmlTextReaderPtr reader)
{
    xmlChar *value;
    double cx = 0.0;
    double cy = 0.0;
    double rx = 0.0;
    double ry = 0.0;

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "cx");
    if (value) 
    {
        cx = parse_double((const char*)value);
        xmlFree(value);
    }
    
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "cy");
    if (value) 
    {
        cy = parse_double((const char*)value);
        xmlFree(value);
    }
    
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "rx");
    if (value) 
    {
        rx = parse_double((const char*)value);
        xmlFree(value);
    }
    
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "ry");
    if (value) 
    {
        ry = parse_double((const char*)value);
        xmlFree(value);
    }
    
    path_.begin_path();

    if(rx != 0.0 && ry != 0.0)
    {
        if(rx < 0.0) throw std::runtime_error("parse_ellipse: Invalid rx");
        if(ry < 0.0) throw std::runtime_error("parse_ellipse: Invalid ry");
        agg::ellipse c(cx, cy, rx, ry);
        path_.storage().concat_path(c);
    }

    path_.end_path();

}

void svg_parser::parse_rect(xmlTextReaderPtr reader)
{
    xmlChar *value;
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;
    double rx = 0.0;
    double ry = 0.0;

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "x");
    if (value)
    {
        x = parse_double((const char*)value);
        xmlFree(value);
    }

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "y");
    if (value)
    {
        y = parse_double((const char*)value);
        xmlFree(value);
    }

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "width");
    if (value)
    {
        w = parse_double((const char*)value);
        xmlFree(value);
    }
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "height");
    if (value)
    {
        h = parse_double((const char*)value);
        xmlFree(value);
    }

    bool rounded = true;
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "rx");
    if (value)
    {
        rx = parse_double((const char*)value);
        xmlFree(value);
    }
    else rounded = false;

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "ry");
    if (value)
    {
        ry = parse_double((const char*)value);
        if (!rounded)
        {
            rx = ry;
            rounded = true;
        }
        xmlFree(value);
    }
    else if (rounded)
    {
        ry = rx;
    }

    if(w != 0.0 && h != 0.0)
    {
        if(w < 0.0) throw std::runtime_error("parse_rect: Invalid width");
        if(h < 0.0) throw std::runtime_error("parse_rect: Invalid height");
        if(rx < 0.0) throw std::runtime_error("parse_rect: Invalid rx");
        if(ry < 0.0) throw std::runtime_error("parse_rect: Invalid ry");
        path_.begin_path();
        
        if(rounded)
        {
            agg::rounded_rect r;
            r.rect(x,y,x+w,y+h);
            r.radius(rx,ry);
            path_.storage().concat_path(r);
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


/*
 *       <stop
 style="stop-color:#ffffff;stop-opacity:1;"
 offset="1"
 id="stop3763" />
*/
void svg_parser::parse_gradient_stop(xmlTextReaderPtr reader)
{
    xmlChar *value;

    double offset = 0.0;
    mapnik::color stop_color;
    double opacity = 1.0;

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "offset");
    if (value) 
    {
        offset = parse_double((const char*)value);
        xmlFree(value);
    }

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "style");
    if (value)
    {
        typedef std::vector<std::pair<std::string,std::string> > cont_type;
        typedef cont_type::value_type value_type;
        cont_type vec;
        parse_style((const char*)value, vec);

        BOOST_FOREACH(value_type kv , vec )
        {
            if (kv.first == "stop-color")
            {
                try
                {
                    stop_color = mapnik::parse_color(kv.second.c_str());
                }
                catch (mapnik::config_error & ex)
                {
                    MAPNIK_LOG_ERROR(svg_parser) << ex.what();
                }
            }
            else if (kv.first == "stop-opacity")
            {
                opacity = parse_double(kv.second.c_str());
            }
        }
        xmlFree(value);
    }

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "stop-color");
    if (value)
    {
        try
        {
            stop_color = mapnik::parse_color((const char *) value);
        }
        catch (mapnik::config_error & ex)
        {
            MAPNIK_LOG_ERROR(svg_parser) << ex.what();
        }
        xmlFree(value);
    }

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "stop-opacity");
    if (value)
    {
        opacity = parse_double((const char *) value);
        xmlFree(value);
    }


    stop_color.set_alpha(opacity*255);

    temporary_gradient_.second.add_stop(offset, stop_color);

    /*
    MAPNIK_LOG_DEBUG(svg_parser) << "\tFound Stop: " << offset << " "
        << (unsigned)stop_color.red() << " "
        << (unsigned)stop_color.green() << " "
        << (unsigned)stop_color.blue() << " "
        << (unsigned)stop_color.alpha();
    */
}

bool svg_parser::parse_common_gradient(xmlTextReaderPtr reader)
{
    xmlChar *value;

    std::string id;
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "id");
    if (value)
    {
        // start a new gradient
        gradient new_grad;
        id = std::string((const char *) value);
        temporary_gradient_ = std::make_pair(id, new_grad);
        xmlFree(value);
    }
    else
    {
        // no point without an ID
        return false;
    }

    // check if we should inherit from another tag
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "xlink:href");
    if (value) 
    {
        if (value[0] == '#')
        {
            std::string linkid = (const char *) &value[1];
            if (gradient_map_.count(linkid))
            {
                temporary_gradient_.second = gradient_map_[linkid];
            }
            else
            {
                MAPNIK_LOG_ERROR(svg_parser) << "Failed to find linked gradient " << linkid;
            }
        }
        xmlFree(value);
    }
    
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "gradientUnits");
    if (value)
    {
        if (xmlStrEqual(value, BAD_CAST "userSpaceOnUse"))
        {
            temporary_gradient_.second.set_units(USER_SPACE_ON_USE);
        }
        else
        {
            temporary_gradient_.second.set_units(OBJECT_BOUNDING_BOX);
        }
        xmlFree(value);
    }
    
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "gradientTransform");
    if (value)
    {
        agg::trans_affine tr;
        mapnik::svg::parse_transform((const char*) value,tr);
        temporary_gradient_.second.set_transform(tr);
        xmlFree(value);
    }

    return true;
}

/**
 *         <radialGradient
 collect="always"
 xlink:href="#linearGradient3759"
 id="radialGradient3765"
 cx="-1.2957155"
 cy="-21.425594"
 fx="-1.2957155"
 fy="-21.425594"
 r="5.1999998"
 gradientUnits="userSpaceOnUse" />
*/
void svg_parser::parse_radial_gradient(xmlTextReaderPtr reader)
{
    if (!parse_common_gradient(reader))
        return;

    xmlChar *value;
    double cx = 0.5;
    double cy = 0.5;
    double fx = 0.0;
    double fy = 0.0;
    double r = 0.5;
    bool has_percent=true;

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "cx");
    if (value) 
    {
        cx = parse_double_optional_percent((const char*)value, has_percent);
        xmlFree(value);
    }

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "cy");
    if (value) 
    {
        cy = parse_double_optional_percent((const char*)value, has_percent);
        xmlFree(value);
    }

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "fx");
    if (value)
    {
        fx = parse_double_optional_percent((const char*)value, has_percent);
        xmlFree(value);
    }
    else
        fx = cx;

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "fy");
    if (value)
    {
        fy = parse_double_optional_percent((const char*)value, has_percent);
        xmlFree(value);
    }
    else
        fy = cy;

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "r");
    if (value)
    {
        r = parse_double_optional_percent((const char*)value, has_percent);
        xmlFree(value);
    }
    // this logic for detecting %'s will not support mixed coordinates.
    if (has_percent && temporary_gradient_.second.get_units() == USER_SPACE_ON_USE)
    {
        temporary_gradient_.second.set_units(USER_SPACE_ON_USE_BOUNDING_BOX);
    }

    temporary_gradient_.second.set_gradient_type(RADIAL);
    temporary_gradient_.second.set_control_points(fx,fy,cx,cy,r);
    // add this here in case we have no end tag, will be replaced if we do
    gradient_map_[temporary_gradient_.first] = temporary_gradient_.second;

    //MAPNIK_LOG_DEBUG(svg_parser) << "Found Radial Gradient: " << " " << cx << " " << cy << " " << fx << " " << fy << " " << r;
}

void svg_parser::parse_linear_gradient(xmlTextReaderPtr reader)
{
    if (!parse_common_gradient(reader))
        return;

    xmlChar *value;
    double x1 = 0.0;
    double x2 = 1.0;
    double y1 = 0.0;
    double y2 = 1.0;

    bool has_percent=true;
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "x1");
    if (value) 
    {
        x1 = parse_double_optional_percent((const char*)value, has_percent);
        xmlFree(value);
    }

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "x2");
    if (value) 
    {
        x2 = parse_double_optional_percent((const char*)value, has_percent);
        xmlFree(value);
    }

    value = xmlTextReaderGetAttribute(reader, BAD_CAST "y1");
    if (value) 
    {
        y1 = parse_double_optional_percent((const char*)value, has_percent);
        xmlFree(value);
    }
    
    value = xmlTextReaderGetAttribute(reader, BAD_CAST "y2");
    if (value) 
    {
        y2 = parse_double_optional_percent((const char*)value, has_percent);
        xmlFree(value);
    }
    // this logic for detecting %'s will not support mixed coordinates.
    if (has_percent && temporary_gradient_.second.get_units() == USER_SPACE_ON_USE)
    {
        temporary_gradient_.second.set_units(USER_SPACE_ON_USE_BOUNDING_BOX);
    }

    temporary_gradient_.second.set_gradient_type(LINEAR);
    temporary_gradient_.second.set_control_points(x1,y1,x2,y2);
    // add this here in case we have no end tag, will be replaced if we do
    gradient_map_[temporary_gradient_.first] = temporary_gradient_.second;

    //MAPNIK_LOG_DEBUG(svg_parser) << "Found Linear Gradient: " << "(" << x1 << " " << y1 << "),(" << x2 << " " << y2 << ")";
}

void svg_parser::parse_pattern(xmlTextReaderPtr reader)
{
    //const xmlChar *value;
}

}}
