/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/safe_cast.hpp>
#include <mapnik/svg/svg_parser_exception.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/util/dasharray_parser.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_ellipse.h"
#include "agg_rounded_rect.h"
#include "agg_span_gradient.h"
#include "agg_color_rgba.h"
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/adapted/struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/detail/xml_parser_read_rapidxml.hpp>
#pragma GCC diagnostic pop

#include <string>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <fstream>

namespace mapnik { namespace svg {
struct viewbox
{
    double x0;
    double y0;
    double x1;
    double y1;
};
}}

BOOST_FUSION_ADAPT_STRUCT (
    mapnik::svg::viewbox,
    (double, x0)
    (double, y0)
    (double, x1)
    (double, y1)
    )

namespace mapnik { namespace svg {

namespace rapidxml = boost::property_tree::detail::rapidxml;

bool traverse_tree(svg_parser & parser,rapidxml::xml_node<char> const* node);
void end_element(svg_parser & parser,rapidxml::xml_node<char> const* node);
void parse_path(svg_parser & parser,rapidxml::xml_node<char> const* node);
void parse_dimensions(svg_parser & parser,rapidxml::xml_node<char> const* node);
void parse_polygon(svg_parser & parser,rapidxml::xml_node<char> const* node);
void parse_polyline(svg_parser & parser,rapidxml::xml_node<char> const* node);
void parse_line(svg_parser & parser,rapidxml::xml_node<char> const* node);
void parse_rect(svg_parser & parser,rapidxml::xml_node<char> const* node);
void parse_circle(svg_parser & parser,rapidxml::xml_node<char> const* node);
void parse_ellipse(svg_parser & parser,rapidxml::xml_node<char> const* node);
void parse_linear_gradient(svg_parser & parser,rapidxml::xml_node<char> const* node);
void parse_radial_gradient(svg_parser & parser,rapidxml::xml_node<char> const* node);
bool parse_common_gradient(svg_parser & parser,rapidxml::xml_node<char> const* node);
void parse_gradient_stop(svg_parser & parser,rapidxml::xml_node<char> const* node);
void parse_attr(svg_parser & parser,rapidxml::xml_node<char> const* node);
void parse_attr(svg_parser & parser,char const * name, char const* value);


namespace grammar {

namespace x3 = boost::spirit::x3;

using color_lookup_type = std::vector<std::pair<double, agg::rgba8> >;
using pairs_type = std::vector<std::pair<std::string, std::string> >;

x3::rule<class key_value_sequence_ordered, pairs_type> const key_value_sequence_ordered("key_value_sequence_ordered");
x3::rule<class key_value, std::pair<std::string, std::string> > key_value("key_value");
x3::rule<class key, std::string> key("key");
x3::rule<class value_, std::string> value("value");

auto const key_def = x3::char_("a-zA-Z_") > *x3::char_("a-zA-Z_0-9-");
auto const value_def = +(x3::char_ - ';');
auto const key_value_def = key > -(':' > value);
auto const key_value_sequence_ordered_def = key_value % ';';

BOOST_SPIRIT_DEFINE(key);
BOOST_SPIRIT_DEFINE(value);
BOOST_SPIRIT_DEFINE(key_value);
BOOST_SPIRIT_DEFINE(key_value_sequence_ordered);

}

template <typename T>
mapnik::color parse_color(T & error_messages, const char* str)
{
    mapnik::color c(100,100,100);
    try
    {
        c = mapnik::parse_color(str);
    }
    catch (mapnik::config_error const& ex)
    {
        error_messages.emplace_back(ex.what());
    }
    return c;
}

template <typename T>
agg::rgba8 parse_color_agg(T & error_messages, const char* str)
{
    auto c = parse_color(error_messages, str);
    return agg::rgba8(c.red(), c.green(), c.blue(), c.alpha());
}

template <typename T>
double parse_double(T & error_messages, const char* str)
{
    using namespace boost::spirit::x3;
    double val = 0.0;
    if (!parse(str, str + std::strlen(str), double_, val))
    {
        error_messages.emplace_back("Failed to parse double: \"" + std::string(str) + "\"");
    }
    return val;
}

// https://www.w3.org/TR/SVG/coords.html#Units
template <typename T, int DPI = 90>
double parse_svg_value(T & error_messages, const char* str, bool & is_percent)
{
    namespace x3 = boost::spirit::x3;
    double val = 0.0;
    x3::symbols<double> units;
    units.add
        ("px", 1.0)
        ("pt", DPI/72.0)
        ("pc", DPI/6.0)
        ("mm", DPI/25.4)
        ("cm", DPI/2.54)
        ("in", static_cast<double>(DPI))
        ;
    const char* cur = str; // phrase_parse modifies the first iterator
    const char* end = str + std::strlen(str);

    auto apply_value =   [&](auto const& ctx) { val = _attr(ctx); is_percent = false; };
    auto apply_units =   [&](auto const& ctx) { val *= _attr(ctx); };
    auto apply_percent = [&](auto const& ctx) { val *= 0.01; is_percent = true; };

    if (!x3::phrase_parse(cur, end,
                          x3::double_[apply_value]
                          > - (units[apply_units]
                               |
                               x3::lit('%')[apply_percent]),
                          x3::space))
    {
        error_messages.emplace_back("Failed to parse SVG value: '" + std::string(str) + "'");
    }
    else if (cur != end)
    {
        error_messages.emplace_back("Failed to parse SVG value: '" + std::string(str) +
                                    "', trailing garbage: '" + cur + "'");
    }
    return val;
}

template <typename T, typename V>
bool parse_viewbox(T & error_messages, const char* str, V & viewbox)
{
    namespace x3 = boost::spirit::x3;
    if ( !x3::phrase_parse(str, str + std::strlen(str),
                           x3::double_ > -x3::lit(',') >
                           x3::double_ > -x3::lit(',') >
                           x3::double_ > -x3::lit(',') >
                           x3::double_, x3::space, viewbox))
    {
        error_messages.emplace_back("failed to parse SVG viewbox from " + std::string(str));
        return false;
    }
    return true;
}

bool parse_style (char const* str, grammar::pairs_type & v)
{
    namespace x3 = boost::spirit::x3;
    return x3::phrase_parse(str, str + std::strlen(str), grammar::key_value_sequence_ordered , x3::space, v);
}

bool parse_id_from_url (char const* str, std::string & id)
{
    namespace x3 = boost::spirit::x3;
    auto extract_id = [&](auto const& ctx) { id += _attr(ctx); };
    return x3::phrase_parse(str, str + std::strlen(str),
                            x3::lit("url") > '(' > '#' > *(x3::char_ - ')')[extract_id] > ')',
                            x3::space);
}

bool traverse_tree(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    auto const* name = node->name();
    switch (node->type())
    {
    case rapidxml::node_element:
    {
        if (std::strcmp(name, "defs") == 0)
        {
            if (node->first_node() != nullptr)
            {
                parser.is_defs_ = true;
            }
        }
        // the gradient tags *should* be in defs, but illustrator seems not to put them in there so
        // accept them anywhere
        else if (std::strcmp(name, "linearGradient") == 0)
        {
            parse_linear_gradient(parser, node);
        }
        else if (std::strcmp(name, "radialGradient") == 0)
        {
            parse_radial_gradient(parser, node);
        }
        else if (std::strcmp(name, "stop") == 0)
        {
            parse_gradient_stop(parser, node);
        }

        if (!parser.is_defs_) // FIXME
        {
            if (std::strcmp(name, "g") == 0)
            {
                if (node->first_node() != nullptr)
                {
                    parser.path_.push_attr();
                    parse_attr(parser, node);
                }
            }
            else
            {
                parser.path_.push_attr();
                parse_attr(parser, node);
                if (parser.path_.display())
                {
                    if (std::strcmp(name, "path") == 0)
                    {
                        parse_path(parser, node);
                    }
                    else if (std::strcmp("polygon", name) == 0)
                    {
                        parse_polygon(parser, node);
                    }
                    else if (std::strcmp("polyline", name) == 0)
                    {
                        parse_polyline(parser, node);
                    }
                    else if (std::strcmp(name, "line") == 0)
                    {
                        parse_line(parser, node);
                    }
                    else if (std::strcmp(name,  "rect") == 0)
                    {
                        parse_rect(parser, node);
                    }
                    else if (std::strcmp(name,  "circle") == 0)
                    {
                        parse_circle(parser, node);
                    }
                    else if (std::strcmp(name,  "ellipse") == 0)
                    {
                        parse_ellipse(parser, node);
                    }
                    else if (std::strcmp(name,  "svg") == 0)
                    {
                        parse_dimensions(parser, node);
                    }
                    else
                    {
                        //std::cerr << "unprocessed node  <--[" << node->name() << "]\n";
                    }
                }
                parser.path_.pop_attr();
            }
        }

        for (auto const* child = node->first_node();
             child; child = child->next_sibling())
        {
            traverse_tree(parser, child);
        }

        end_element(parser, node);
    }
    break;
#if 0 //
    // Data nodes
    case rapidxml::node_data:
    case rapidxml::node_cdata:
    {

        if (node->value_size() > 0) // Don't add empty text nodes
        {
            // parsed text values should have leading and trailing
            // whitespace trimmed.
            //std::string trimmed = node->value();
            //mapnik::util::trim(trimmed);
            std::cerr << "CDATA:" << node->value() << std::endl;
        }
    }
    break;
#endif
    default:
        break;
    }
    return true;
}


void end_element(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    auto const* name = node->name();
    if (!parser.is_defs_ && std::strcmp(name, "g") == 0)
    {
        if (node->first_node() != nullptr)
        {
            parser.path_.pop_attr();
        }
    }
    else if (std::strcmp(name,  "defs") == 0)
    {
        if (node->first_node() != nullptr)
        {
            parser.is_defs_ = false;
        }
    }
    else if (std::strcmp(name, "linearGradient") == 0 || std::strcmp(name, "radialGradient") == 0)
    {
        parser.gradient_map_[parser.temporary_gradient_.first] = parser.temporary_gradient_.second;
    }
}

void parse_attr(svg_parser & parser, char const* name, char const* value )
{
    if (std::strcmp(name, "transform") == 0)
    {
        agg::trans_affine tr;
        mapnik::svg::parse_svg_transform(value,tr);
        parser.path_.transform().premultiply(tr);
    }
    else if (std::strcmp(name, "fill") == 0)
    {
        std::string id;
        if (std::strcmp(value, "none") == 0)
        {
            parser.path_.fill_none();
        }
        else if (parse_id_from_url(value, id))
        {
            // see if we have a known gradient fill
            if (parser.gradient_map_.count(id) > 0)
            {
                parser.path_.add_fill_gradient(parser.gradient_map_[id]);
            }
            else
            {
                std::stringstream ss;
                ss << "Failed to find gradient fill: " << id;
                parser.error_messages_.push_back(ss.str());
            }
        }
        else
        {
            parser.path_.fill(parse_color_agg(parser.error_messages_, value));
        }
    }
    else if (std::strcmp(name,"fill-opacity") == 0)
    {
        parser.path_.fill_opacity(parse_double(parser.error_messages_, value));
    }
    else if (std::strcmp(name, "fill-rule") == 0)
    {
        if (std::strcmp(value, "evenodd") == 0)
        {
            parser.path_.even_odd(true);
        }
    }
    else if (std::strcmp(name, "stroke") == 0)
    {
        std::string id;
        if (std::strcmp(value, "none") == 0)
        {
            parser.path_.stroke_none();
        }
        else if (parse_id_from_url(value, id))
        {
            // see if we have a known gradient fill
            if (parser.gradient_map_.count(id) > 0)
            {
                parser.path_.add_stroke_gradient(parser.gradient_map_[id]);
            }
            else
            {
                std::stringstream ss;
                ss << "Failed to find gradient stroke: " << id;
                parser.error_messages_.push_back(ss.str());
            }
        }
        else
        {
            parser.path_.stroke(parse_color_agg(parser.error_messages_, value));
        }
    }
    else if (std::strcmp(name, "stroke-width") == 0)
    {
        bool percent;
        parser.path_.stroke_width(parse_svg_value(parser.error_messages_, value, percent));
    }
    else if (std::strcmp(name, "stroke-opacity") == 0)
    {
        parser.path_.stroke_opacity(parse_double(parser.error_messages_, value));
    }
    else if(std::strcmp(name, "stroke-linecap") == 0)
    {
        if(std::strcmp(value, "butt") == 0)
            parser.path_.line_cap(agg::butt_cap);
        else if(std::strcmp(value, "round") == 0)
            parser.path_.line_cap(agg::round_cap);
        else if(std::strcmp(value, "square") == 0)
            parser.path_.line_cap(agg::square_cap);
    }
    else if(std::strcmp(name, "stroke-linejoin") == 0)
    {
        if(std::strcmp(value, "miter") == 0)
            parser.path_.line_join(agg::miter_join);
        else if(std::strcmp(value, "round") == 0)
            parser.path_.line_join(agg::round_join);
        else if(std::strcmp(value, "bevel") == 0)
            parser.path_.line_join(agg::bevel_join);
    }
    else if(std::strcmp(name, "stroke-miterlimit") == 0)
    {
        parser.path_.miter_limit(parse_double(parser.error_messages_,value));
    }
    else if (std::strcmp(name,"stroke-dasharray") == 0)
    {
        dash_array dash;
        if (util::parse_dasharray(value, dash))
        {
            parser.path_.dash_array(std::move(dash));
        }
    }
    else if (std::strcmp(name,"stroke-dashoffset") == 0)
    {
        double offset = parse_double(parser.error_messages_, value);
        parser.path_.dash_offset(offset);
    }
    else if(std::strcmp(name,  "opacity") == 0)
    {
        double opacity = parse_double(parser.error_messages_, value);
        parser.path_.opacity(opacity);
    }
    else if (std::strcmp(name,  "visibility") == 0)
    {
        parser.path_.visibility(std::strcmp(value,  "hidden") != 0);
    }
    else if (std::strcmp(name,  "display") == 0  && std::strcmp(value,  "none") == 0)
    {
        parser.path_.display(false);
    }
}

void parse_attr(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    for (rapidxml::xml_attribute<char> const* attr = node->first_attribute();
         attr; attr = attr->next_attribute())
    {
        auto const* name = attr->name();
        if (std::strcmp(name, "style") == 0)
        {
            using cont_type = std::vector<std::pair<std::string,std::string> >;
            using value_type = cont_type::value_type;
            cont_type vec;
            parse_style(attr->value(), vec);
            for (value_type kv : vec )
            {
                parse_attr(parser, kv.first.c_str(), kv.second.c_str());
            }
        }
        else
        {
            parse_attr(parser,name, attr->value());
        }
    }
}

void parse_dimensions(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    double width = 0;
    double height = 0;
    double aspect_ratio = 1;
    viewbox vbox = {0,0,0,0};
    bool has_viewbox = false;
    bool has_percent_height = true;
    bool has_percent_width = true;

    auto const* width_attr = node->first_attribute("width");
    if (width_attr)
    {
        width = parse_svg_value(parser.error_messages_, width_attr->value(), has_percent_width);
    }
    auto const* height_attr = node->first_attribute("height");
    if (height_attr)
    {
        height = parse_svg_value(parser.error_messages_, height_attr->value(), has_percent_height);
    }
    auto const* viewbox_attr = node->first_attribute("viewBox");
    if (viewbox_attr)
    {
        has_viewbox = parse_viewbox(parser.error_messages_, viewbox_attr->value(), vbox);
    }

    if (has_percent_width && !has_percent_height && has_viewbox)
    {
        aspect_ratio = vbox.x1 / vbox.y1;
        width = aspect_ratio * height;
    }
    else if (!has_percent_width && has_percent_height && has_viewbox)
    {
        aspect_ratio = vbox.x1 / vbox.y1;
        height = height / aspect_ratio;
    }
    else if (has_percent_width && has_percent_height && has_viewbox)
    {
        width = vbox.x1;
        height = vbox.y1;
    }

    parser.path_.set_dimensions(width, height);
}

void parse_path(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    auto const* attr = node->first_attribute("d");
    if (attr != nullptr)
    {
        auto const* value = attr->value();
        if (std::strlen(value) > 0)
        {
            parser.path_.begin_path();

            if (!mapnik::svg::parse_path(value, parser.path_))
            {
                auto const* id_attr = node->first_attribute("xml:id");
                if (id_attr == nullptr) id_attr = node->first_attribute("id");
                if (id_attr)
                {
                    parser.error_messages_.push_back(std::string("unable to parse invalid svg <path> with id '")
                                                     + id_attr->value() + "'");
                }
                else
                {
                    parser.error_messages_.push_back(std::string("unable to parse invalid svg <path>"));
                }
            }
            parser.path_.end_path();
        }
    }
}

void parse_polygon(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    auto const* attr = node->first_attribute("points");
    if (attr != nullptr)
    {
        parser.path_.begin_path();
        if (!mapnik::svg::parse_points(attr->value(), parser.path_))
        {
            parser.error_messages_.push_back(std::string("Failed to parse <polygon> 'points'"));
        }
        parser.path_.close_subpath();
        parser.path_.end_path();
    }
}

void parse_polyline(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    auto const* attr = node->first_attribute("points");
    if (attr != nullptr)
    {
        parser.path_.begin_path();
        if (!mapnik::svg::parse_points(attr->value(), parser.path_))
        {
            parser.error_messages_.push_back(std::string("Failed to parse <polyline> 'points'"));
        }
        parser.path_.end_path();
    }
}

void parse_line(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    double x1 = 0.0;
    double y1 = 0.0;
    double x2 = 0.0;
    double y2 = 0.0;
    bool percent;
    auto const* x1_attr = node->first_attribute("x1");
    if (x1_attr) x1 = parse_svg_value(parser.error_messages_, x1_attr->value(), percent);

    auto const* y1_attr = node->first_attribute("y1");
    if (y1_attr) y1 = parse_svg_value(parser.error_messages_, y1_attr->value(), percent);

    auto const* x2_attr = node->first_attribute("x2");
    if (x2_attr) x2 = parse_svg_value(parser.error_messages_, x2_attr->value(), percent);

    auto const* y2_attr = node->first_attribute("y2");
    if (y2_attr) y2 = parse_svg_value(parser.error_messages_, y2_attr->value(), percent);

    parser.path_.begin_path();
    parser.path_.move_to(x1, y1);
    parser.path_.line_to(x2, y2);
    parser.path_.end_path();
}

void parse_circle(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    double cx = 0.0;
    double cy = 0.0;
    double r = 0.0;
    bool percent;
    auto * attr = node->first_attribute("cx");
    if (attr != nullptr)
    {
        cx = parse_svg_value(parser.error_messages_, attr->value(), percent);
    }

    attr = node->first_attribute("cy");
    if (attr != nullptr)
    {
        cy = parse_svg_value(parser.error_messages_, attr->value(), percent);
    }

    attr = node->first_attribute("r");
    if (attr != nullptr)
    {
        r = parse_svg_value(parser.error_messages_, attr->value(), percent);
    }

    parser.path_.begin_path();
    if(r != 0.0)
    {
        if (r < 0.0)
        {
            parser.error_messages_.emplace_back("parse_circle: Invalid radius");
        }
        else
        {
            agg::ellipse c(cx, cy, r, r);
            parser.path_.storage().concat_path(c);
        }
    }
    parser.path_.end_path();
}

void parse_ellipse(svg_parser & parser, rapidxml::xml_node<char> const  * node)
{
    double cx = 0.0;
    double cy = 0.0;
    double rx = 0.0;
    double ry = 0.0;
    bool percent;
    auto * attr = node->first_attribute("cx");
    if (attr != nullptr)
    {
        cx = parse_svg_value(parser.error_messages_, attr->value(), percent);
    }

    attr = node->first_attribute("cy");
    if (attr)
    {
        cy = parse_svg_value(parser.error_messages_, attr->value(), percent);
    }

    attr = node->first_attribute("rx");
    if (attr != nullptr)
    {
        rx = parse_svg_value(parser.error_messages_, attr->value(), percent);
    }

    attr = node->first_attribute("ry");
    if (attr != nullptr)
    {
        ry = parse_svg_value(parser.error_messages_, attr->value(), percent);
    }

    if (rx != 0.0 && ry != 0.0)
    {

        if (rx < 0.0)
        {
            parser.error_messages_.emplace_back("parse_ellipse: Invalid rx");
        }
        else if (ry < 0.0)
        {
            parser.error_messages_.emplace_back("parse_ellipse: Invalid ry");
        }
        else
        {
            parser.path_.begin_path();
            agg::ellipse c(cx, cy, rx, ry);
            parser.path_.storage().concat_path(c);
            parser.path_.end_path();
        }
    }
}

void parse_rect(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    // http://www.w3.org/TR/SVGTiny12/shapes.html#RectElement
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;
    double rx = 0.0;
    double ry = 0.0;
    bool percent;
    auto * attr = node->first_attribute("x");
    if (attr != nullptr)
    {
        x = parse_svg_value(parser.error_messages_, attr->value(), percent);
    }

    attr = node->first_attribute("y");
    if (attr != nullptr)
    {
        y = parse_svg_value(parser.error_messages_, attr->value(), percent);
    }

    attr = node->first_attribute("width");
    if (attr != nullptr)
    {
        w = parse_svg_value(parser.error_messages_, attr->value(), percent);
    }
    attr = node->first_attribute("height");
    if (attr)
    {
        h = parse_svg_value(parser.error_messages_, attr->value(), percent);
    }

    bool rounded = true;
    attr = node->first_attribute("rx");
    if (attr != nullptr)
    {
        rx = parse_svg_value(parser.error_messages_, attr->value(), percent);
        if ( rx > 0.5 * w ) rx = 0.5 * w;
    }
    else rounded = false;

    attr = node->first_attribute("ry");
    if (attr != nullptr)
    {
        ry = parse_svg_value(parser.error_messages_, attr->value(), percent);
        if ( ry > 0.5 * h ) ry = 0.5 * h;
        if (!rounded)
        {
            rx = ry;
            rounded = true;
        }
    }
    else if (rounded)
    {
        ry = rx;
    }

    if (w != 0.0 && h != 0.0)
    {
        if(w < 0.0)
        {
            parser.error_messages_.emplace_back("parse_rect: Invalid width");
        }
        else if(h < 0.0)
        {
            parser.error_messages_.emplace_back("parse_rect: Invalid height");
        }
        else if(rx < 0.0)
        {
            parser.error_messages_.emplace_back("parse_rect: Invalid rx");
        }
        else if(ry < 0.0)
        {
            parser.error_messages_.emplace_back("parse_rect: Invalid ry");
        }
        else
        {
            parser.path_.begin_path();

            if(rounded)
            {
                agg::rounded_rect r;
                r.rect(x,y,x+w,y+h);
                r.radius(rx,ry);
                parser.path_.storage().concat_path(r);
            }
            else
            {
                parser.path_.move_to(x,     y);
                parser.path_.line_to(x + w, y);
                parser.path_.line_to(x + w, y + h);
                parser.path_.line_to(x,     y + h);
                parser.path_.close_subpath();
            }
            parser.path_.end_path();
        }
    }
}

void parse_gradient_stop(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    double offset = 0.0;
    mapnik::color stop_color;
    double opacity = 1.0;

    auto * attr = node->first_attribute("offset");
    if (attr != nullptr)
    {
        offset = parse_double(parser.error_messages_,attr->value());
    }

    attr = node->first_attribute("style");
    if (attr != nullptr)
    {
        using cont_type = std::vector<std::pair<std::string,std::string> >;
        using value_type = cont_type::value_type;
        cont_type vec;
        parse_style(attr->value(), vec);

        for (value_type kv : vec )
        {
            if (kv.first == "stop-color")
            {
                stop_color = parse_color(parser.error_messages_, kv.second.c_str());
            }
            else if (kv.first == "stop-opacity")
            {
                opacity = parse_double(parser.error_messages_,kv.second.c_str());
            }
        }
    }

    attr = node->first_attribute("stop-color");
    if (attr != nullptr)
    {
        stop_color = parse_color(parser.error_messages_, attr->value());
    }

    attr = node->first_attribute("stop-opacity");
    if (attr != nullptr)
    {
        opacity = parse_double(parser.error_messages_, attr->value());
    }

    stop_color.set_alpha(static_cast<uint8_t>(opacity * 255));
    parser.temporary_gradient_.second.add_stop(offset, stop_color);
}

bool parse_common_gradient(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    std::string id;
    auto * attr = node->first_attribute("xml:id");
    if (attr == nullptr) attr = node->first_attribute("id");

    if (attr != nullptr)
    {
        // start a new gradient
        parser.temporary_gradient_ = std::make_pair(std::string(attr->value()), gradient());
    }
    else
    {
        // no point without an ID
        return false;
    }

    // check if we should inherit from another tag
    attr = node->first_attribute("xlink:href");
    if (attr != nullptr)
    {
        auto const* value = attr->value();
        if (std::strlen(value) > 1 && value[0] == '#')
        {
            std::string linkid(&value[1]); // FIXME !!!
            if (parser.gradient_map_.count(linkid))
            {
                parser.temporary_gradient_.second = parser.gradient_map_[linkid];
            }
            else
            {
                std::stringstream ss;
                ss << "Failed to find linked gradient " << linkid;
                parser.error_messages_.push_back(ss.str());
                return false;
            }
        }
    }

    attr = node->first_attribute("gradientUnits");
    if (attr != nullptr)
    {
        if (std::strcmp(attr->value(), "userSpaceOnUse") == 0)
        {
            parser.temporary_gradient_.second.set_units(USER_SPACE_ON_USE);
        }
        else
        {
            parser.temporary_gradient_.second.set_units(OBJECT_BOUNDING_BOX);
        }
    }

    attr = node->first_attribute("gradientTransform");
    if (attr != nullptr)
    {
        agg::trans_affine tr;
        mapnik::svg::parse_svg_transform(attr->value(),tr);
        parser.temporary_gradient_.second.set_transform(tr);
    }
    return true;
}

void parse_radial_gradient(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    parse_common_gradient(parser, node);
    double cx = 0.5;
    double cy = 0.5;
    double fx = 0.0;
    double fy = 0.0;
    double r = 0.5;
    bool has_percent=true;

    auto * attr = node->first_attribute("cx");
    if (attr != nullptr)
    {
        cx = parse_svg_value(parser.error_messages_, attr->value(), has_percent);
    }

    attr = node->first_attribute("cy");
    if (attr != nullptr)
    {
        cy = parse_svg_value(parser.error_messages_, attr->value(), has_percent);
    }

    attr = node->first_attribute("fx");
    if (attr != nullptr)
    {
        fx = parse_svg_value(parser.error_messages_,attr->value(), has_percent);
    }
    else
        fx = cx;

    attr = node->first_attribute("fy");
    if (attr != nullptr)
    {
        fy = parse_svg_value(parser.error_messages_, attr->value(), has_percent);
    }
    else
        fy = cy;

    attr = node->first_attribute("r");
    if (attr != nullptr)
    {
        r = parse_svg_value(parser.error_messages_, attr->value(), has_percent);
    }
    // this logic for detecting %'s will not support mixed coordinates.
    if (has_percent && parser.temporary_gradient_.second.get_units() == USER_SPACE_ON_USE)
    {
        parser.temporary_gradient_.second.set_units(USER_SPACE_ON_USE_BOUNDING_BOX);
    }

    parser.temporary_gradient_.second.set_gradient_type(RADIAL);
    parser.temporary_gradient_.second.set_control_points(fx,fy,cx,cy,r);
    // add this here in case we have no end tag, will be replaced if we do
    parser.gradient_map_[parser.temporary_gradient_.first] = parser.temporary_gradient_.second;

    //MAPNIK_LOG_DEBUG(svg_parser) << "Found Radial Gradient: " << " " << cx << " " << cy << " " << fx << " " << fy << " " << r;
}

void parse_linear_gradient(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    parse_common_gradient(parser, node);

    double x1 = 0.0;
    double x2 = 1.0;
    double y1 = 0.0;
    double y2 = 1.0;

    bool has_percent=true;
    auto * attr = node->first_attribute("x1");
    if (attr != nullptr)
    {
        x1 = parse_svg_value(parser.error_messages_, attr->value(), has_percent);
    }

    attr = node->first_attribute("x2");
    if (attr != nullptr)
    {
        x2 = parse_svg_value(parser.error_messages_, attr->value(), has_percent);
    }

    attr = node->first_attribute("y1");
    if (attr != nullptr)
    {
        y1 = parse_svg_value(parser.error_messages_, attr->value(), has_percent);
    }

    attr = node->first_attribute("y2");
    if (attr != nullptr)
    {
        y2 = parse_svg_value(parser.error_messages_, attr->value(), has_percent);
    }
    // this logic for detecting %'s will not support mixed coordinates.
    if (has_percent && parser.temporary_gradient_.second.get_units() == USER_SPACE_ON_USE)
    {
        parser.temporary_gradient_.second.set_units(USER_SPACE_ON_USE_BOUNDING_BOX);
    }

    parser.temporary_gradient_.second.set_gradient_type(LINEAR);
    parser.temporary_gradient_.second.set_control_points(x1,y1,x2,y2);
    // add this here in case we have no end tag, will be replaced if we do
    parser.gradient_map_[parser.temporary_gradient_.first] = parser.temporary_gradient_.second;
}

svg_parser::svg_parser(svg_converter<svg_path_adapter,
                       agg::pod_bvector<mapnik::svg::path_attributes> > & path)
    : path_(path),
      is_defs_(false) {}

svg_parser::~svg_parser() {}

bool svg_parser::parse(std::string const& filename)
{
#ifdef _WINDOWS
    std::basic_ifstream<char> stream(mapnik::utf8_to_utf16(filename));
#else
    std::basic_ifstream<char> stream(filename.c_str());
#endif
    if (!stream)
    {
        std::stringstream ss;
        ss << "Unable to open '" << filename << "'";
        error_messages_.push_back(ss.str());
        return false;
    }

    stream.unsetf(std::ios::skipws);
    std::vector<char> buffer(std::istreambuf_iterator<char>(stream.rdbuf()),
                             std::istreambuf_iterator<char>());
    buffer.push_back(0);

    const int flags = rapidxml::parse_trim_whitespace | rapidxml::parse_validate_closing_tags;
    rapidxml::xml_document<> doc;
    try
    {
        doc.parse<flags>(buffer.data());
    }
    catch (rapidxml::parse_error const& ex)
    {
        std::stringstream ss;
        ss << "svg_parser::parse - Unable to parse '" << filename << "'";
        error_messages_.push_back(ss.str());
        return false;
    }

    for (rapidxml::xml_node<char> const* child = doc.first_node();
         child; child = child->next_sibling())
    {
        traverse_tree(*this, child);
    }
    return error_messages_.empty() ? true : false;
}

bool svg_parser::parse_from_string(std::string const& svg)
{
    const int flags = rapidxml::parse_trim_whitespace | rapidxml::parse_validate_closing_tags;
    rapidxml::xml_document<> doc;
    std::vector<char> buffer(svg.begin(), svg.end());
    buffer.push_back(0);
    try
    {
        doc.parse<flags>(buffer.data());
    }
    catch (rapidxml::parse_error const& ex)
    {
        std::stringstream ss;
        ss << "Unable to parse '" << svg << "'";
        error_messages_.push_back(ss.str());
        return false;
    }
    for (rapidxml::xml_node<char> const* child = doc.first_node();
         child; child = child->next_sibling())
    {
        traverse_tree(*this, child);
    }
    return error_messages_.empty() ? true : false;
}

svg_parser::error_message_container const& svg_parser::error_messages() const
{
    return error_messages_;
}

}}
