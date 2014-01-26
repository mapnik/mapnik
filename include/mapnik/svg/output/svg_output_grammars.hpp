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

#ifndef SVG_OUTPUT_GRAMMARS_HPP
#define SVG_OUTPUT_GRAMMARS_HPP

// mapnik
#include <mapnik/stroke.hpp>

// fwd declare
namespace mapnik { namespace svg {
  struct path_output_attributes;
  struct rect_output_attributes;
  struct root_output_attributes;
} }

// boost
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_omit.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/repository/include/karma_confix.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/struct.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>

/*!
 * mapnik::svg::path_output_attributes is adapted as a fusion sequence
 * in order to be used directly by the svg_path_attributes_grammar (below).
 *
 * This adaptation is the primary reason why the attributes are stored in
 * this structure before being passed to the generate_path method.
 */
BOOST_FUSION_ADAPT_STRUCT(
    mapnik::svg::path_output_attributes,
    (std::string, fill_color_)
    (double, fill_opacity_)
    (std::string, stroke_color_)
    (double, stroke_opacity_)
    (double, stroke_width_)
    (std::string, stroke_linecap_)
    (std::string, stroke_linejoin_)
    (double, stroke_dashoffset_)
    )

/*!
 * mapnik::svg::rect_output_attributes is adapted as a fusion sequence
 * in order to be used directly by the svg_rect_attributes_grammar (below).
 */
BOOST_FUSION_ADAPT_STRUCT(
    mapnik::svg::rect_output_attributes,
    (int, x_)
    (int, y_)
    (unsigned, width_)
    (unsigned, height_)
    (std::string, fill_color_)
    )

/*!
 * mapnik::svg::root_output_attributes is adapted as a fusion sequence
 * in order to be used directly by the svg_root_attributes_grammar (below).
 */
BOOST_FUSION_ADAPT_STRUCT(
    mapnik::svg::root_output_attributes,
    (unsigned, width_)
    (unsigned, height_)
    (double, svg_version_)
    (std::string, svg_namespace_url_)
    )

namespace mapnik { namespace svg {

    using namespace boost::spirit;
    using namespace boost::phoenix;


template <typename OutputIterator>
struct svg_path_attributes_grammar : karma::grammar<OutputIterator, mapnik::svg::path_output_attributes()>
{
    explicit svg_path_attributes_grammar()
        : svg_path_attributes_grammar::base_type(svg_path_attributes)
    {
        karma::lit_type lit;
        karma::double_type double_;
        karma::string_type kstring;
        repository::confix_type confix;

        svg_path_attributes =
            lit("fill=") << confix('"', '"')[kstring]
                         << lit(" fill-opacity=") << confix('"', '"')[double_]
                         << lit(" stroke=") << confix('"', '"')[kstring]
                         << lit(" stroke-opacity=") << confix('"', '"')[double_]
                         << lit(" stroke-width=") << confix('"', '"')[double_ << lit("px")]
                         << lit(" stroke-linecap=") << confix('"', '"')[kstring]
                         << lit(" stroke-linejoin=") << confix('"', '"')[kstring]
                         << lit(" stroke-dashoffset=") << confix('"', '"')[double_ << lit("px")];
    }

    karma::rule<OutputIterator, mapnik::svg::path_output_attributes()> svg_path_attributes;
};

template <typename OutputIterator>
struct svg_path_dash_array_grammar : karma::grammar<OutputIterator, mapnik::dash_array()>
{
    explicit svg_path_dash_array_grammar()
        : svg_path_dash_array_grammar::base_type(svg_path_dash_array)
    {
        karma::double_type double_;
        karma::lit_type lit;
        repository::confix_type confix;

        svg_path_dash_array =
            lit("stroke-dasharray=")
            << confix('"', '"')[
                -((double_ << lit(',') << double_) % lit(','))];
    }

    karma::rule<OutputIterator, mapnik::dash_array()> svg_path_dash_array;
};

template <typename OutputIterator>
struct svg_rect_attributes_grammar : karma::grammar<OutputIterator, mapnik::svg::rect_output_attributes()>
{
    explicit svg_rect_attributes_grammar()
        : svg_rect_attributes_grammar::base_type(svg_rect_attributes)
    {
        karma::lit_type lit;
        karma::int_type int_;
        karma::string_type kstring;
        repository::confix_type confix;

        svg_rect_attributes =
            lit("x=") << confix('"', '"')[int_]
                      << lit(" y=") << confix('"', '"')[int_]
                      << lit(" width=") << confix('"', '"')[int_ << lit("px")]
                      << lit(" height=") << confix('"', '"')[int_ << lit("px")]
                      << lit(" fill=") << confix('"', '"')[kstring];
    }

    karma::rule<OutputIterator, mapnik::svg::rect_output_attributes()> svg_rect_attributes;
};

template <typename OutputIterator>
struct svg_root_attributes_grammar : karma::grammar<OutputIterator, mapnik::svg::root_output_attributes()>
{
    explicit svg_root_attributes_grammar()
        : svg_root_attributes_grammar::base_type(svg_root_attributes)
    {
        karma::lit_type lit;
        karma::int_type int_;
        karma::string_type kstring;
        karma::double_type double_;
        repository::confix_type confix;

        svg_root_attributes =
            lit("width=") << confix('"', '"')[int_ << lit("px")]
                          << lit(" height=") << confix('"', '"')[int_ << lit("px")]
                          << " version=" << confix('"', '"')[double_]
                          << " xmlns=" << confix('"', '"')[kstring]
                          << lit(" xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"");
    }

    karma::rule<OutputIterator, mapnik::svg::root_output_attributes()> svg_root_attributes;
};
}
}

#endif // SVG_OUTPUT_GRAMMARS_HPP
