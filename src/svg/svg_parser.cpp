/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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
#include <mapnik/util/name_to_int.hpp>
#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_ellipse.h"
#include "agg_rounded_rect.h"
#include "agg_span_gradient.h"
#include "agg_color_rgba.h"
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/detail/xml_parser_read_rapidxml.hpp>
#pragma GCC diagnostic pop

#include <string>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <fstream>
#include <array>

namespace mapnik { namespace svg {

using util::name_to_int;

struct viewbox
{
    double x0;
    double y0;
    double width;
    double height;
};
}}

BOOST_FUSION_ADAPT_STRUCT (
    mapnik::svg::viewbox,
    (double, x0)
    (double, y0)
    (double, width)
    (double, height)
    )

namespace mapnik { namespace svg {

namespace rapidxml = boost::property_tree::detail::rapidxml;

void traverse_tree(svg_parser& parser, rapidxml::xml_node<char> const* node);
void end_element(svg_parser& parser, rapidxml::xml_node<char> const* node);
void parse_path(svg_parser& parser, rapidxml::xml_node<char> const* node);
void parse_element(svg_parser& parser, char const* name, rapidxml::xml_node<char> const* node);
void parse_use(svg_parser& parser, rapidxml::xml_node<char> const* node);
void parse_dimensions(svg_parser& parser, rapidxml::xml_node<char> const* node);
void parse_polygon(svg_parser& parser, rapidxml::xml_node<char> const* node);
void parse_polyline(svg_parser& parser, rapidxml::xml_node<char> const* node);
void parse_line(svg_parser& parser, rapidxml::xml_node<char> const* node);
void parse_rect(svg_parser& parser, rapidxml::xml_node<char> const* node);
void parse_circle(svg_parser& parser, rapidxml::xml_node<char> const* node);
void parse_ellipse(svg_parser& parser, rapidxml::xml_node<char> const* node);
void parse_linear_gradient(svg_parser& parser, rapidxml::xml_node<char> const* node);
void parse_radial_gradient(svg_parser& parser, rapidxml::xml_node<char> const* node);
bool parse_common_gradient(svg_parser& parser, std::string const& id,
                           mapnik::gradient& gr, rapidxml::xml_node<char> const* node);
void parse_gradient_stop(svg_parser& parser, mapnik::gradient& gr, rapidxml::xml_node<char> const* node);
void parse_attr(svg_parser& parser, rapidxml::xml_node<char> const* node);
void parse_attr(svg_parser& parser, char const* name, char const* value);

namespace {

static std::array<unsigned, 7> const unsupported_elements
{ {name_to_int("symbol"),
   name_to_int("marker"),
   name_to_int("view"),
   name_to_int("text"),
   name_to_int("switch"),
   name_to_int("image"),
   name_to_int("a")}
};

#if 0 // disable to reduce verbosity
static std::array<unsigned, 43> const unsupported_attributes
{ {name_to_int("alignment-baseline"),
   name_to_int("baseline-shift"),
   name_to_int("clip"),
   name_to_int("clip-path"),
   name_to_int("clip-rule"),
   name_to_int("color-interpolation"),
   name_to_int("color-interpolation-filters"),
   name_to_int("color-profile"),
   name_to_int("color-rendering"),
   name_to_int("cursor"),
   name_to_int("direction"),
   name_to_int("dominant-baseline"),
   name_to_int("enable-background"),
   name_to_int("filter"),
   name_to_int("flood-color"),
   name_to_int("flood-opacity"),
   name_to_int("font-family"),
   name_to_int("font-size"),
   name_to_int("font-size-adjust"),
   name_to_int("font-stretch"),
   name_to_int("font-style"),
   name_to_int("font-variant"),
   name_to_int("font-weight"),
   name_to_int("glyph-orientation-horizontal"),
   name_to_int("glyph-orientation-vertical"),
   name_to_int("image-rendering"),
   name_to_int("kerning"),
   name_to_int("letter-spacing"),
   name_to_int("lighting-color"),
   name_to_int("marker-end"),
   name_to_int("marker-mid"),
   name_to_int("marker-start"),
   name_to_int("mask"),
   name_to_int("overflow"),
   name_to_int("pointer-events"),
   name_to_int("shape-rendering"),
   name_to_int("text-anchor"),
   name_to_int("text-decoration"),
   name_to_int("text-rendering"),
   name_to_int("unicode-bidi"),
   name_to_int("word-spacing"),
   name_to_int("writing-mode")}
};

#endif

template <typename T>
void handle_unsupported(svg_parser& parser, T const& ar, char const* name)
{
    unsigned element = name_to_int(name);
    for (auto const& e : ar)
    {
        if (e == element)
        {
            parser.err_handler().on_error(std::string("SVG support error: <" + std::string(name) + "> element is not supported"));
        }
    }
}

using color_lookup_type = std::vector<std::pair<double, agg::rgba8> >;
namespace qi = boost::spirit::qi;
using pairs_type = std::vector<std::pair<std::string, std::string> >;

template <typename Iterator,typename SkipType>
struct key_value_sequence_ordered
    : qi::grammar<Iterator, pairs_type(), SkipType>
{
    key_value_sequence_ordered()
        : key_value_sequence_ordered::base_type(query)
    {
        qi::lit_type lit;
        qi::char_type char_;
        query =  pair >> *( lit(';') >> pair);
        pair  =  key >> -(':' >> value);
        key   =  char_("a-zA-Z_") >> *char_("a-zA-Z_0-9-");
        value = +(char_ - lit(';'));
    }

    qi::rule<Iterator, pairs_type(), SkipType> query;
    qi::rule<Iterator, std::pair<std::string, std::string>(), SkipType> pair;
    qi::rule<Iterator, std::string(), SkipType> key, value;
};

template <typename T>
mapnik::color parse_color(T & err_handler, const char* str)
{
    mapnik::color c(100,100,100);
    try
    {
        c = mapnik::parse_color(str);
    }
    catch (mapnik::config_error const& ex)
    {
        err_handler.on_error("SVG parse error: failed to parse <color> with value \"" + std::string(str) + "\"");
    }
    return c;
}

template <typename T>
agg::rgba8 parse_color_agg(T & err_handler, const char* str)
{
    auto c = parse_color(err_handler, str);
    return agg::rgba8(c.red(), c.green(), c.blue(), c.alpha());
}

template <typename T>
double parse_double(T & err_handler, const char* str)
{
    using namespace boost::spirit::qi;
    double_type double_;
    double val = 0.0;
    if (!parse(str, str + std::strlen(str),double_,val))
    {
        err_handler.on_error("SVG parse error: failed to parse <number> with value \"" + std::string(str) + "\"");
    }
    return val;
}

// https://www.w3.org/TR/SVG/coords.html#Units
template <typename T, int DPI = 90>
double parse_svg_value(T & err_handler, const char* str, bool & percent)
{
    using skip_type = boost::spirit::ascii::space_type;
    using boost::phoenix::ref;
    qi::double_type double_;
    qi::lit_type lit;
    qi::_1_type _1;
    double val = 0.0;
    qi::symbols<char, double> units;
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
    if (!qi::phrase_parse(cur, end,
                      double_[ref(val) = _1][ref(percent) = false]
                      > - (units[ ref(val) *= _1]
                           |
                           lit('%')[ref(val) *= 0.01][ref(percent) = true]),
                      skip_type()) || cur != end)
    {
        err_handler.on_error("SVG parse error: failed to parse <number> with value \"" + std::string(str) + "\"");
    }
    return val;
}

template <typename T, typename V>
bool parse_viewbox(T & err_handler, const char* str, V & viewbox)
{
    using namespace boost::spirit::qi;
    using boost::phoenix::ref;
    double_type double_;
    lit_type lit;
    using skip_type = boost::spirit::ascii::space_type;

    if (!phrase_parse(str, str + std::strlen(str),
                      double_ >> -lit(',') >>
                      double_ >> -lit(',') >>
                      double_ >> -lit(',') >>
                      double_, skip_type(), viewbox))
    {
        err_handler.on_error("SVG parse error: failed to parse <viewbox> with value \"" + std::string(str) + "\"");
        return false;
    }
    return true;
}

bool parse_style (char const* str, pairs_type & v)
{
    using namespace boost::spirit::qi;
    using skip_type = boost::spirit::ascii::space_type;
    key_value_sequence_ordered<const char*, skip_type> kv_parser;
    return phrase_parse(str, str + std::strlen(str), kv_parser, skip_type(), v);
}

bool parse_id_from_url (char const* str, std::string & id)
{
    using namespace boost::spirit::qi;
    using skip_type = boost::spirit::ascii::space_type;
    qi::_1_type _1;
    qi::char_type char_;
    qi::lit_type lit;
    return phrase_parse(str, str + std::strlen(str),
                        lit("url") > "(" > "#" > *(char_ - lit(')'))[boost::phoenix::ref(id) += _1] > ")",
                        skip_type());
}

}

boost::property_tree::detail::rapidxml::xml_attribute<char> const * parse_id(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    auto const* id_attr = node->first_attribute("xml:id");
    if (id_attr == nullptr) id_attr = node->first_attribute("id");

    if (id_attr && parser.node_cache_.count(id_attr->value()) == 0)
    {
        parser.node_cache_.emplace(id_attr->value(), node);
    }
    return id_attr;
}

boost::property_tree::detail::rapidxml::xml_attribute<char> const * parse_href(rapidxml::xml_node<char> const* node)
{
    auto const* attr = node->first_attribute("xlink:href");
    if (attr == nullptr) attr = node->first_attribute("href");
    return attr;
}

enum aspect_ratio_alignment
{
    none = 0,
    xMinYMin,
    xMidYMin,
    xMaxYMin,
    xMinYMid,
    xMidYMid,
    xMaxYMid,
    xMinYMax,
    xMidYMax,
    xMaxYMax
};

template <typename T>
std::pair<unsigned,bool> parse_preserve_aspect_ratio(T & err_handler, char const* str)
{
    std::pair<unsigned,bool> preserve_aspect_ratio {xMidYMid, true };
    using skip_type = boost::spirit::ascii::space_type;
    using boost::phoenix::ref;
    qi::lit_type lit;
    qi::_1_type _1;
    qi::symbols<char, unsigned> align;
    align.add
        ("none", none)
        ("xMinYMin", xMinYMin)
        ("xMidYMin", xMidYMin)
        ("xMaxYMin", xMaxYMin)
        ("xMinYMid", xMinYMid)
        ("xMidYMid", xMidYMid)
        ("xMaxYMid", xMaxYMid)
        ("xMinYMax", xMinYMax)
        ("xMidYMax", xMidYMax)
        ("xMaxYMax", xMaxYMax);


    char const* cur = str; // phrase_parse mutates the first iterator
    char const* end = str + std::strlen(str);
    try
    {
        qi::phrase_parse(cur, end, -lit("defer") // only applicable to <image> which we don't support currently
                         > align[ref(preserve_aspect_ratio.first) = _1]
                         > -(lit("meet") | lit("slice")[ref(preserve_aspect_ratio.second) = false]), skip_type());
    }
    catch (qi::expectation_failure<char const*> const& ex)
    {
        err_handler.on_error("SVG parse error: failed to parse <preserveAspectRatio> with value \""  + std::string(str)  + "\"");
        return {xMidYMid, true} ; // default
    }
    return preserve_aspect_ratio;
}


void traverse_tree(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    auto const* name = node->name();
    switch (node->type())
    {
    case rapidxml::node_element:
    {
        switch(name_to_int(name))
        {
        case name_to_int("defs"):
        {
            if (node->first_node() != nullptr)
            {
                parser.is_defs_ = true;
            }
            break;
        }
        // the gradient tags *should* be in defs, but illustrator seems not to put them in there so
        // accept them anywhere
        case name_to_int("linearGradient"):
            parse_linear_gradient(parser, node);
            break;
        case name_to_int("radialGradient"):
            parse_radial_gradient(parser, node);
            break;
        case name_to_int("symbol"):
            parse_id(parser, node);
            //parse_dimensions(parser, node);
            break;
        }

        if (!parser.is_defs_) // FIXME
        {
            switch (name_to_int(name))
            {
            case name_to_int("g"):
                if (node->first_node() != nullptr)
                {
                    parser.path_.push_attr();
                    parse_id(parser, node);
                    parse_attr(parser, node);
                }
                break;
            case name_to_int("use"):
                parser.path_.push_attr();
                parse_id(parser, node);
                parse_attr(parser, node);
                parse_use(parser, node);
                parser.path_.pop_attr();
                break;
            default:
                parser.path_.push_attr();
                parse_id(parser, node);
                parse_attr(parser, node);
                if (parser.path_.display())
                {
                    parse_element(parser, name, node);
                }
                parser.path_.pop_attr();
            }
        }
        else
        {
            // save node for later
            parse_id(parser, node);
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
            //std::cerr << "CDATA:" << node->value() << std::endl;
        }
    }
    break;
#endif
    default:
        break;
    }
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
}

void parse_element(svg_parser & parser, char const* name, rapidxml::xml_node<char> const* node)
{
    switch (name_to_int(name))
    {
    case name_to_int("path"):
        parser.path_.transform().multiply(parser.viewbox_tr_);
        parse_path(parser, node);
        break;
    case name_to_int("polygon"):
        parser.path_.transform().multiply(parser.viewbox_tr_);
        parse_polygon(parser, node);
        break;
    case name_to_int("polyline"):
        parser.path_.transform().multiply(parser.viewbox_tr_);
        parse_polyline(parser, node);
        break;
    case name_to_int("line"):
        parser.path_.transform().multiply(parser.viewbox_tr_);
        parse_line(parser, node);
        break;
    case name_to_int("rect"):
        parser.path_.transform().multiply(parser.viewbox_tr_);
        parse_rect(parser, node);
        break;
    case name_to_int("circle"):
        parser.path_.transform().multiply(parser.viewbox_tr_);
        parse_circle(parser, node);
        break;
    case name_to_int("ellipse"):
        parser.path_.transform().multiply(parser.viewbox_tr_);
        parse_ellipse(parser, node);
        break;
    case name_to_int("svg"):
        parse_dimensions(parser, node);
        break;
    default:
        handle_unsupported(parser, unsupported_elements, name);
        break;
    }
}

void parse_stroke(svg_parser& parser, char const* value)
{
    std::string id;
    if (std::strcmp(value, "none") == 0)
    {
        parser.path_.stroke_none();
    }
    else if (parse_id_from_url(value, id))
    {
        // see if we have a known gradient stroke
        if (parser.gradient_map_.count(id) > 0)
        {
            parser.path_.add_stroke_gradient(parser.gradient_map_[id]);
        }
        else if (parser.node_cache_.count(id) > 0)
        {
            // try parsing again
            auto const* gradient_node = parser.node_cache_[id];
            traverse_tree(parser, gradient_node);
            if (parser.gradient_map_.count(id) > 0)
            {
                parser.path_.add_stroke_gradient(parser.gradient_map_[id]);
            }
            else
            {
                std::stringstream ss;
                ss << "SVG parse error: failed to locate <gradient> stroke with <id> \"" << id << "\"";
                parser.err_handler().on_error(ss.str());
            }
        }
        else
        {
            std::stringstream ss;
            ss << "SVG parse error: failed to locate <gradient> stroke with <id> \"" << id << "\"";
            parser.err_handler().on_error(ss.str());
        }
    }
    else
    {
        parser.path_.stroke(parse_color_agg(parser.err_handler(), value));
    }
}

void parse_fill(svg_parser& parser, char const* value)
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
        else if (parser.node_cache_.count(id) > 0)
        {
            // try parsing again
            auto const* gradient_node = parser.node_cache_[id];
            traverse_tree(parser, gradient_node);
            if (parser.gradient_map_.count(id) > 0)
            {
                parser.path_.add_stroke_gradient(parser.gradient_map_[id]);
            }
            else
            {
                std::stringstream ss;
                ss << "SVG parse error: failed to locate <gradient> fill with <id> \"" << id << "\"";
                parser.err_handler().on_error(ss.str());
            }
        }
        else
        {
            std::stringstream ss;
            ss << "SVG parse error: failed to locate <gradient> fill with <id> \"" << id << "\"";
            parser.err_handler().on_error(ss.str());
        }
    }
    else
    {
        parser.path_.fill(parse_color_agg(parser.err_handler(), value));
    }
}

void parse_transform(svg_parser & parser, char const* value)
{
    agg::trans_affine tr;
    mapnik::svg::parse_svg_transform(value,tr);
    parser.path_.transform().premultiply(tr);
}

void parse_stroke_dash(svg_parser & parser, char const* value)
{
    dash_array dash;
    if (util::parse_dasharray(value, dash))
    {
        parser.path_.dash_array(std::move(dash));
    }
}

void parse_attr(svg_parser & parser, char const* name, char const* value )
{
    switch (name_to_int(name))
    {
    case name_to_int("transform"):
        parse_transform(parser, value);
        break;
    case name_to_int("fill"):
        parse_fill(parser, value);
        break;
    case name_to_int("fill-opacity"):
        parser.path_.fill_opacity(parse_double(parser.err_handler(), value));
        break;
    case name_to_int("fill-rule"):
        if (std::strcmp(value, "evenodd") == 0)
        {
            parser.path_.even_odd(true);
        }
        break;
    case name_to_int("stroke"):
        parse_stroke(parser, value);
        break;
    case name_to_int("stroke-width"):
        bool percent;
        parser.path_.stroke_width(parse_svg_value(parser.err_handler(), value, percent));
        break;
    case name_to_int("stroke-opacity"):
        parser.path_.stroke_opacity(parse_double(parser.err_handler(), value));
        break;
    case name_to_int("stroke-linecap"):
        if(std::strcmp(value, "butt") == 0)
            parser.path_.line_cap(agg::butt_cap);
        else if(std::strcmp(value, "round") == 0)
            parser.path_.line_cap(agg::round_cap);
        else if(std::strcmp(value, "square") == 0)
            parser.path_.line_cap(agg::square_cap);
        break;
    case name_to_int("stroke-linejoin"):
        if (std::strcmp(value, "miter") == 0)
            parser.path_.line_join(agg::miter_join);
        else if (std::strcmp(value, "round") == 0)
            parser.path_.line_join(agg::round_join);
        else if (std::strcmp(value, "bevel") == 0)
            parser.path_.line_join(agg::bevel_join);
        break;
    case name_to_int("stroke-miterlimit"):
        parser.path_.miter_limit(parse_double(parser.err_handler(),value));
        break;
    case name_to_int("stroke-dasharray"):
        parse_stroke_dash(parser, value);
        break;
    case name_to_int("stroke-dashoffset"):
        parser.path_.dash_offset(parse_double(parser.err_handler(), value));
        break;
    case name_to_int("opacity"):
        parser.path_.opacity(parse_double(parser.err_handler(), value));
        break;
    case name_to_int("visibility"):
        parser.path_.visibility(std::strcmp(value,  "hidden") != 0);
        break;
    case name_to_int("display"):
        if (std::strcmp(value,  "none") == 0)
        {
            parser.path_.display(false);
        }
        break;
    default:
        //handle_unsupported(parser, unsupported_attributes, name);
        // disable for now to reduce verbosity
        break;
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
    viewbox vbox = {0, 0, 0, 0};
    bool has_viewbox = false;
    bool has_percent_height = true;
    bool has_percent_width = true;

    auto const* width_attr = node->first_attribute("width");
    if (width_attr)
    {
        width = parse_svg_value(parser.err_handler(), width_attr->value(), has_percent_width);
    }
    auto const* height_attr = node->first_attribute("height");
    if (height_attr)
    {
        height = parse_svg_value(parser.err_handler(), height_attr->value(), has_percent_height);
    }

    auto const* viewbox_attr = node->first_attribute("viewBox");
    if (viewbox_attr)
    {
        has_viewbox = parse_viewbox(parser.err_handler(), viewbox_attr->value(), vbox);
        if (width > 0 && height > 0 && vbox.width > 0 && vbox.height > 0)
        {
            agg::trans_affine t{};
            std::pair<unsigned,bool> preserve_aspect_ratio {xMidYMid, true};
            auto const* aspect_ratio_attr = node->first_attribute("preserveAspectRatio");
            if (aspect_ratio_attr)
            {
                preserve_aspect_ratio = parse_preserve_aspect_ratio(parser.err_handler(), aspect_ratio_attr->value());
            }

            double sx = width / vbox.width;
            double sy = height / vbox.height;
            double scale = preserve_aspect_ratio.second ? std::min(sx, sy) : std::max(sx, sy);
            switch (preserve_aspect_ratio.first)
            {
            case none:
                t = agg::trans_affine_scaling(sx, sy) * t;
                break;
            case xMinYMin:
                t = agg::trans_affine_scaling(scale, scale) * t;
                break;
            case xMinYMid:
                t = agg::trans_affine_scaling(scale, scale) * t;
                t = agg::trans_affine_translation(0, -0.5 * (vbox.height - height / scale)) * t;
                break;
            case xMinYMax:
                t = agg::trans_affine_scaling(scale, scale) * t;
                t = agg::trans_affine_translation(0, -1.0 * (vbox.height - height / scale)) * t;
                break;
            case xMidYMin:
                t = agg::trans_affine_scaling(scale, scale) * t;
                t = agg::trans_affine_translation(-0.5 * (vbox.width - width / scale), 0.0) * t;
                break;
            case xMidYMid: // (the default)
                t = agg::trans_affine_scaling(scale, scale) * t;
                t = agg::trans_affine_translation(-0.5 * (vbox.width - width / scale),
                                                  -0.5 * (vbox.height - height / scale)) * t;
                break;
            case xMidYMax:
                t = agg::trans_affine_scaling(scale, scale) * t;
                t = agg::trans_affine_translation(-0.5 * (vbox.width - width / scale),
                                                  -1.0 * (vbox.height - height / scale)) * t;
                break;
            case xMaxYMin:
                t = agg::trans_affine_scaling(scale, scale) * t;
                t = agg::trans_affine_translation(-1.0 * (vbox.width - width / scale), 0.0) * t;
                break;
            case xMaxYMid:
                t = agg::trans_affine_scaling(scale, scale) * t;
                t = agg::trans_affine_translation(-1.0 * (vbox.width - width / scale),
                                                  -0.5 * (vbox.height - height / scale)) * t;
                break;
            case xMaxYMax:
                t = agg::trans_affine_scaling(scale, scale) * t;
                t = agg::trans_affine_translation(-1.0 * (vbox.width - width / scale),
                                                  -1.0 * (vbox.height - height / scale)) * t;
                break;
            };

            t = agg::trans_affine_translation(-vbox.x0, -vbox.y0) * t;
            parser.viewbox_tr_ = t;
        }


    }
    if (has_percent_width && !has_percent_height && has_viewbox)
    {
        aspect_ratio = vbox.width / vbox.height;
        width = aspect_ratio * height;
    }
    else if (!has_percent_width && has_percent_height && has_viewbox)
    {
        aspect_ratio = vbox.width/vbox.height;
        height = height / aspect_ratio;
    }
    else if (has_percent_width && has_percent_height && has_viewbox)
    {
        width = vbox.width;
        height = vbox.height;
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
                auto const* id_attr = parse_id(parser, node);
                if (id_attr)
                {
                    parser.err_handler().on_error(std::string("SVG parse error: failed to parse <path> with <id> \"")
                                                  + id_attr->value() + "\"");
                }
                else
                {
                    parser.err_handler().on_error(std::string("SVG parse error: failed to parse <path>"));
                }
            }
            parser.path_.end_path();
        }
    }
}

void parse_use(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    auto * attr = parse_href(node);
    if (attr)
    {
        auto const* value = attr->value();
        if (std::strlen(value) > 1 && value[0] == '#')
        {
            std::string id(&value[1]);
            if (parser.node_cache_.count(id) > 0)
            {
                auto const* base_node = parser.node_cache_[id];
                double x = 0.0;
                double y = 0.0;
                double w = 0.0;
                double h = 0.0;
                bool percent = false;
                attr = node->first_attribute("x");
                if (attr != nullptr)
                {
                    x = parse_svg_value(parser.err_handler(), attr->value(), percent);
                }

                attr = node->first_attribute("y");
                if (attr != nullptr)
                {
                    y = parse_svg_value(parser.err_handler(), attr->value(), percent);
                }

                attr = node->first_attribute("width");
                if (attr != nullptr)
                {
                    w = parse_svg_value(parser.err_handler(), attr->value(), percent);
                    if (percent) w *= parser.path_.width();
                }
                attr = node->first_attribute("height");
                if (attr)
                {
                    h = parse_svg_value(parser.err_handler(), attr->value(), percent);
                    if (percent) h *= parser.path_.height();
                }
                if (w < 0.0)
                {
                    std::stringstream ss;
                    ss << "SVG validation error: invalid <use> width \"" << w <<  "\"";
                    parser.err_handler().on_error(ss.str());
                }
                else if (h < 0.0)
                {
                    std::stringstream ss;
                    ss << "SVG validation error: invalid <use> height \"" << w <<  "\"";
                    parser.err_handler().on_error(ss.str());
                }
                agg::trans_affine t{};
                if (!node->first_attribute("transform") && w != 0.0 && h != 0.0)
                {
                    // FIXME
                    double scale = std::min(double(w / parser.path_.width()), double(h / parser.path_.height()));
                    t *= agg::trans_affine_scaling(scale);
                }
                t *= agg::trans_affine_translation(x, y);
                parser.path_.transform().premultiply(t);
                traverse_tree(parser, base_node);
            }
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
            parser.err_handler().on_error(std::string("SVG parse error: failed to parse <polygon> points"));
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
            parser.err_handler().on_error(std::string("SVG parse error: failed to parse <polyline> points"));
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
    if (x1_attr) x1 = parse_svg_value(parser.err_handler(), x1_attr->value(), percent);

    auto const* y1_attr = node->first_attribute("y1");
    if (y1_attr) y1 = parse_svg_value(parser.err_handler(), y1_attr->value(), percent);

    auto const* x2_attr = node->first_attribute("x2");
    if (x2_attr) x2 = parse_svg_value(parser.err_handler(), x2_attr->value(), percent);

    auto const* y2_attr = node->first_attribute("y2");
    if (y2_attr) y2 = parse_svg_value(parser.err_handler(), y2_attr->value(), percent);

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
        cx = parse_svg_value(parser.err_handler(), attr->value(), percent);
    }

    attr = node->first_attribute("cy");
    if (attr != nullptr)
    {
        cy = parse_svg_value(parser.err_handler(), attr->value(), percent);
    }

    attr = node->first_attribute("r");
    if (attr != nullptr)
    {
        r = parse_svg_value(parser.err_handler(), attr->value(), percent);
    }

    parser.path_.begin_path();
    if(r != 0.0)
    {
        if (r < 0.0)
        {
            std::stringstream ss;
            ss << "SVG validation error: invalid <circle> radius \"" << r << "\"";
            parser.err_handler().on_error(ss.str());
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
        cx = parse_svg_value(parser.err_handler(), attr->value(), percent);
    }

    attr = node->first_attribute("cy");
    if (attr)
    {
        cy = parse_svg_value(parser.err_handler(), attr->value(), percent);
    }

    attr = node->first_attribute("rx");
    if (attr != nullptr)
    {
        rx = parse_svg_value(parser.err_handler(), attr->value(), percent);
    }

    attr = node->first_attribute("ry");
    if (attr != nullptr)
    {
        ry = parse_svg_value(parser.err_handler(), attr->value(), percent);
    }

    if (rx != 0.0 && ry != 0.0)
    {
        if (rx < 0.0)
        {
            std::stringstream ss;
            ss << "SVG validation error: invalid <ellipse> rx \"" << rx << "\"";
            parser.err_handler().on_error(ss.str());
        }
        else if (ry < 0.0)
        {
            std::stringstream ss;
            ss << "SVG validation error: invalid <ellipse> ry \"" << ry << "\"";
            parser.err_handler().on_error(ss.str());
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
        x = parse_svg_value(parser.err_handler(), attr->value(), percent);
    }

    attr = node->first_attribute("y");
    if (attr != nullptr)
    {
        y = parse_svg_value(parser.err_handler(), attr->value(), percent);
    }

    attr = node->first_attribute("width");
    if (attr != nullptr)
    {
        w = parse_svg_value(parser.err_handler(), attr->value(), percent);
    }
    attr = node->first_attribute("height");
    if (attr)
    {
        h = parse_svg_value(parser.err_handler(), attr->value(), percent);
    }

    bool rounded = true;
    attr = node->first_attribute("rx");
    if (attr != nullptr)
    {
        rx = parse_svg_value(parser.err_handler(), attr->value(), percent);
        if ( rx > 0.5 * w ) rx = 0.5 * w;
    }
    else rounded = false;

    attr = node->first_attribute("ry");
    if (attr != nullptr)
    {
        ry = parse_svg_value(parser.err_handler(), attr->value(), percent);
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
        if (w < 0.0)
        {
            std::stringstream ss;
            ss << "SVG validation error: invalid <rect> width \"" << w << "\"";
            parser.err_handler().on_error(ss.str());
        }
        else if (h < 0.0)
        {
            std::stringstream ss;
            ss << "SVG validation error: invalid <rect> height \"" << h << "\"";
            parser.err_handler().on_error(ss.str());
        }
        else if (rx < 0.0)
        {
            std::stringstream ss;
            ss << "SVG validation error: invalid <rect> rx \"" << rx << "\"";
            parser.err_handler().on_error(ss.str());
        }
        else if (ry < 0.0)
        {
            std::stringstream ss;
            ss << "SVG validation error: invalid <rect> ry \"" << ry << "\"";
            parser.err_handler().on_error(ss.str());
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

void parse_gradient_stop(svg_parser & parser, mapnik::gradient& gr, rapidxml::xml_node<char> const* node)
{
    double offset = 0.0;
    mapnik::color stop_color;
    double opacity = 1.0;

    auto * attr = node->first_attribute("offset");
    if (attr != nullptr)
    {
        bool percent = false;
        offset = parse_svg_value(parser.err_handler(),attr->value(), percent);
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
                stop_color = parse_color(parser.err_handler(), kv.second.c_str());
            }
            else if (kv.first == "stop-opacity")
            {
                opacity = parse_double(parser.err_handler(),kv.second.c_str());
            }
        }
    }

    attr = node->first_attribute("stop-color");
    if (attr != nullptr)
    {
        stop_color = parse_color(parser.err_handler(), attr->value());
    }

    attr = node->first_attribute("stop-opacity");
    if (attr != nullptr)
    {
        opacity = parse_double(parser.err_handler(), attr->value());
    }

    stop_color.set_alpha(static_cast<uint8_t>(opacity * 255));
    gr.add_stop(offset, stop_color);
}

bool parse_common_gradient(svg_parser & parser, std::string const& id, mapnik::gradient& gr, rapidxml::xml_node<char> const* node)
{
    // check if we should inherit from another tag
    auto * attr = parse_href(node);
    if (attr != nullptr)
    {
        auto const* value = attr->value();
        if (std::strlen(value) > 1 && value[0] == '#')
        {
            std::string linkid(&value[1]); // FIXME !!!
            if (parser.gradient_map_.count(linkid))
            {
                gr = parser.gradient_map_[linkid];
            }
            else
            {
                // save node for later
                parser.node_cache_.emplace(id, node);
                return false;
            }
        }
    }

    attr = node->first_attribute("gradientUnits");
    if (attr != nullptr)
    {
        if (std::strcmp(attr->value(), "userSpaceOnUse") == 0)
        {
            gr.set_units(USER_SPACE_ON_USE);
        }
        else
        {
            gr.set_units(OBJECT_BOUNDING_BOX);
        }
    }

    attr = node->first_attribute("gradientTransform");
    if (attr != nullptr)
    {
        agg::trans_affine tr;
        mapnik::svg::parse_svg_transform(attr->value(),tr);
        gr.set_transform(tr);
    }
    return true;
}

void parse_radial_gradient(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    auto * attr = parse_id(parser, node);
    if (attr == nullptr) return;
    std::string id = attr->value();

    mapnik::gradient gr;
    if (!parse_common_gradient(parser, id, gr, node)) return;
    double cx = 0.5;
    double cy = 0.5;
    double fx = 0.0;
    double fy = 0.0;
    double r = 0.5;
    bool has_percent=true;

    attr = node->first_attribute("cx");
    if (attr != nullptr)
    {
        cx = parse_svg_value(parser.err_handler(), attr->value(), has_percent);
    }

    attr = node->first_attribute("cy");
    if (attr != nullptr)
    {
        cy = parse_svg_value(parser.err_handler(), attr->value(), has_percent);
    }

    attr = node->first_attribute("fx");
    if (attr != nullptr)
    {
        fx = parse_svg_value(parser.err_handler(),attr->value(), has_percent);
    }
    else
        fx = cx;

    attr = node->first_attribute("fy");
    if (attr != nullptr)
    {
        fy = parse_svg_value(parser.err_handler(), attr->value(), has_percent);
    }
    else fy = cy;

    attr = node->first_attribute("r");
    if (attr != nullptr)
    {
        r = parse_svg_value(parser.err_handler(), attr->value(), has_percent);
    }
    // this logic for detecting %'s will not support mixed coordinates.
    if (has_percent && gr.get_units() == USER_SPACE_ON_USE)
    {
        gr.set_units(USER_SPACE_ON_USE_BOUNDING_BOX);
    }

    gr.set_gradient_type(RADIAL);
    gr.set_control_points(fx, fy, cx, cy, r);

    // parse stops
    for (auto const* child = node->first_node();
         child; child = child->next_sibling())
    {
        if (std::strcmp(child->name(), "stop") == 0)
        {
            parse_gradient_stop(parser, gr, child);
        }
    }
    parser.gradient_map_[id] = gr;
    //MAPNIK_LOG_DEBUG(svg_parser) << "Found Radial Gradient: " << " " << cx << " " << cy << " " << fx << " " << fy << " " << r;
}

void parse_linear_gradient(svg_parser & parser, rapidxml::xml_node<char> const* node)
{
    auto const* attr = parse_id(parser, node);
    if (attr == nullptr) return;

    std::string id = attr->value();
    mapnik::gradient gr;
    if (!parse_common_gradient(parser, id, gr, node)) return;

    double x1 = 0.0;
    double x2 = 1.0;
    double y1 = 0.0;
    double y2 = 0.0;

    bool has_percent=true;
    attr = node->first_attribute("x1");
    if (attr != nullptr)
    {
        x1 = parse_svg_value(parser.err_handler(), attr->value(), has_percent);
    }

    attr = node->first_attribute("x2");
    if (attr != nullptr)
    {
        x2 = parse_svg_value(parser.err_handler(), attr->value(), has_percent);
    }

    attr = node->first_attribute("y1");
    if (attr != nullptr)
    {
        y1 = parse_svg_value(parser.err_handler(), attr->value(), has_percent);
    }

    attr = node->first_attribute("y2");
    if (attr != nullptr)
    {
        y2 = parse_svg_value(parser.err_handler(), attr->value(), has_percent);
    }
    // this logic for detecting %'s will not support mixed coordinates.
    if (has_percent && gr.get_units() == USER_SPACE_ON_USE)
    {
        gr.set_units(USER_SPACE_ON_USE_BOUNDING_BOX);
    }

    gr.set_gradient_type(LINEAR);
    gr.set_control_points(x1, y1, x2, y2);

    // parse stops
    for (auto const* child = node->first_node();
         child; child = child->next_sibling())
    {
        if (std::strcmp(child->name(), "stop") == 0)
        {
            parse_gradient_stop(parser, gr, child);
        }
    }
    parser.gradient_map_[id] = gr;
}

svg_parser::svg_parser(svg_converter<svg_path_adapter,
                       agg::pod_bvector<mapnik::svg::path_attributes> > & path, bool strict)
    : path_(path),
      is_defs_(false),
      err_handler_(strict) {}

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
        ss << "SVG error: unable to open \"" << filename << "\"";
        throw std::runtime_error(ss.str());
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
        ss << "SVG error: unable to parse \"" << filename << "\"";
        throw std::runtime_error(ss.str());
    }

    for (rapidxml::xml_node<char> const* child = doc.first_node();
         child; child = child->next_sibling())
    {
        traverse_tree(*this, child);
    }
    return !err_handler_.error_messages().empty();
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
        std::string str = (svg.length() > 1024) ? svg.substr(0, 1024) + "..." : svg;
        ss << "SVG error: unable to parse \"" << str << "\"";
        throw std::runtime_error(ss.str());
    }
    for (rapidxml::xml_node<char> const* child = doc.first_node();
         child; child = child->next_sibling())
    {
        traverse_tree(*this, child);
    }
    return !err_handler_.error_messages().empty();
}

svg_parser::error_handler & svg_parser::err_handler()
{
    return err_handler_;
}

}}
