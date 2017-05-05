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

#ifndef SVG_OUTPUT_GRAMMARS_HPP
#define SVG_OUTPUT_GRAMMARS_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/symbolizer_base.hpp>

#include <vector>

// fwd declare
namespace mapnik { namespace svg {
    struct path_output_attributes;
    struct rect_output_attributes;
    struct root_output_attributes;
}}

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/karma_nonterminal.hpp>
#include <boost/spirit/include/karma_rule.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace svg {

    using namespace boost::spirit;


template <typename OutputIterator>
struct svg_path_attributes_grammar : karma::grammar<OutputIterator, mapnik::svg::path_output_attributes()>
{
    explicit svg_path_attributes_grammar();
    karma::rule<OutputIterator, mapnik::svg::path_output_attributes()> svg_path_attributes;
};

template <typename OutputIterator>
struct svg_path_dash_array_grammar : karma::grammar<OutputIterator, mapnik::dash_array()>
{
    explicit svg_path_dash_array_grammar();
    karma::rule<OutputIterator, mapnik::dash_array()> svg_path_dash_array;
};

template <typename OutputIterator>
struct svg_rect_attributes_grammar : karma::grammar<OutputIterator, mapnik::svg::rect_output_attributes()>
{
    explicit svg_rect_attributes_grammar();
    karma::rule<OutputIterator, mapnik::svg::rect_output_attributes()> svg_rect_attributes;
};

template <typename OutputIterator>
struct svg_root_attributes_grammar : karma::grammar<OutputIterator, mapnik::svg::root_output_attributes()>
{
    explicit svg_root_attributes_grammar();
    karma::rule<OutputIterator, mapnik::svg::root_output_attributes()> svg_root_attributes;
};
}
}

#endif // SVG_OUTPUT_GRAMMARS_HPP
