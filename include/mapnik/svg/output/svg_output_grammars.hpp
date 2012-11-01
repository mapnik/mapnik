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
#include <mapnik/vertex.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/svg/output/svg_path_iterator.hpp>
#include <mapnik/svg/output/svg_output_attributes.hpp>

// boost
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_omit.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/repository/include/karma_confix.hpp>
#include <boost/spirit/home/phoenix/bind/bind_member_function.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/struct.hpp>
#include <boost/fusion/include/boost_tuple.hpp>

// std
#include <iostream>

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

/*!
 * mapnik::geometry_type is adapted to conform to the concepts
 * required by Karma to be recognized as a container of
 * attributes for output generation.
 */
/*
namespace boost { namespace spirit { namespace traits {

        typedef mapnik::coord_transform<mapnik::CoordTransform, mapnik::geometry_type> path_type;

        template <>
        struct is_container<path_type const>
            : mpl::true_ {};

        template <>
        struct container_iterator<path_type const>
        {
            typedef mapnik::svg::path_iterator_type type;
        };

        template <>
        struct begin_container<path_type const>
        {
            static mapnik::svg::path_iterator_type
            call(path_type const& path)
            {
                return mapnik::svg::path_iterator_type(0, path);
            }
        };

        template <>
        struct end_container<path_type const>
        {
            static mapnik::svg::path_iterator_type
            call(path_type const& path)
            {
                return mapnik::svg::path_iterator_type(path);
            }
        };
}}}
*/

namespace mapnik { namespace svg {

    using namespace boost::spirit;
    using namespace boost::phoenix;

/*
    template <typename OutputIterator, typename PathType>
    struct svg_path_data_grammar : karma::grammar<OutputIterator, PathType&()>
    {
        typedef path_iterator_type::value_type vertex_type;

        explicit svg_path_data_grammar(PathType const& path_type)
            : svg_path_data_grammar::base_type(svg_path),
              path_type_(path_type)
        {
            using karma::int_;
            using karma::double_;
            using repository::confix;

            svg_path =
                lit("d=")
                << confix('"', '"')[
                    -(path_vertex % lit(' '))];

            path_vertex =
                path_vertex_command
                << double_
                << lit(' ')
                << double_;

            path_vertex_command = &int_(1) << lit('M') | lit('L');
        }

        karma::rule<OutputIterator, PathType&()> svg_path;
        karma::rule<OutputIterator, vertex_type()> path_vertex;
        karma::rule<OutputIterator, int()> path_vertex_command;

        PathType const& path_type_;
    };
*/

template <typename OutputIterator>
struct svg_path_attributes_grammar : karma::grammar<OutputIterator, mapnik::svg::path_output_attributes()>
{
    explicit svg_path_attributes_grammar()
        : svg_path_attributes_grammar::base_type(svg_path_attributes)
    {
        using karma::double_;
        using karma::string;
        using repository::confix;

        svg_path_attributes =
            lit("fill=") << confix('"', '"')[string]
                         << lit(" fill-opacity=") << confix('"', '"')[double_]
                         << lit(" stroke=") << confix('"', '"')[string]
                         << lit(" stroke-opacity=") << confix('"', '"')[double_]
                         << lit(" stroke-width=") << confix('"', '"')[double_ << lit("px")]
                         << lit(" stroke-linecap=") << confix('"', '"')[string]
                         << lit(" stroke-linejoin=") << confix('"', '"')[string]
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
        using karma::double_;
        using repository::confix;

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
        using karma::int_;
        using karma::string;
        using repository::confix;

        svg_rect_attributes =
            lit("x=") << confix('"', '"')[int_]
                      << lit(" y=") << confix('"', '"')[int_]
                      << lit(" width=") << confix('"', '"')[int_ << lit("px")]
                      << lit(" height=") << confix('"', '"')[int_ << lit("px")]
                      << lit(" fill=") << confix('"', '"')[string];
    }

    karma::rule<OutputIterator, mapnik::svg::rect_output_attributes()> svg_rect_attributes;
};

template <typename OutputIterator>
struct svg_root_attributes_grammar : karma::grammar<OutputIterator, mapnik::svg::root_output_attributes()>
{
    explicit svg_root_attributes_grammar()
        : svg_root_attributes_grammar::base_type(svg_root_attributes)
    {
        using karma::int_;
        using karma::string;
        using karma::double_;
        using repository::confix;

        svg_root_attributes =
            lit("width=") << confix('"', '"')[int_ << lit("px")]
                          << lit(" height=") << confix('"', '"')[int_ << lit("px")]
                          << " version=" << confix('"', '"')[double_]
                          << " xmlns=" << confix('"', '"')[string];
    }

    karma::rule<OutputIterator, mapnik::svg::root_output_attributes()> svg_root_attributes;
};
}
}

#endif // SVG_OUTPUT_GRAMMARS_HPP
