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
#include <mapnik/svg/output/svg_generator.hpp>
#include <mapnik/geometry.hpp>

// boost
#include <boost/spirit/include/karma.hpp>

namespace mapnik { namespace svg {

    using namespace boost::spirit;

    template <typename OutputIterator>
    svg_generator<OutputIterator>::svg_generator(OutputIterator& output_iterator)
        : output_iterator_(output_iterator) {}

    template <typename OutputIterator>
    svg_generator<OutputIterator>::~svg_generator() {}

    template <typename OutputIterator>
    void svg_generator<OutputIterator>::generate_header()
    {
        karma::generate(output_iterator_, lit("<?xml version=\"1.0\" standalone=\"no\"?>\n"));
        karma::generate(output_iterator_, lit("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"));
    }

    template <typename OutputIterator>
    void svg_generator<OutputIterator>::generate_opening_root(root_output_attributes const& root_attributes)
    {
        root_attributes_grammar attributes_grammar;
        karma::generate(output_iterator_, lit("<svg ") << attributes_grammar << lit(">\n"), root_attributes);
    }

    template <typename OutputIterator>
    void svg_generator<OutputIterator>::generate_closing_root()
    {
        karma::generate(output_iterator_, lit("</svg>"));
    }

    template <typename OutputIterator>
    void svg_generator<OutputIterator>::generate_rect(rect_output_attributes const& rect_attributes)
    {
        rect_attributes_grammar attributes_grammar;
        karma::generate(output_iterator_, lit("<rect ") << attributes_grammar << lit("/>\n"), rect_attributes);
    }

    /*template <typename OutputIterator>
    void svg_generator<OutputIterator>::generate_path(path_type const& path, path_output_attributes const& path_attributes)
    {
        path_data_grammar data_grammar(path);
        path_attributes_grammar attributes_grammar;
        path_dash_array_grammar dash_array_grammar;

        karma::generate(output_iterator_, lit("<path ")  << data_grammar, path);
        karma::generate(output_iterator_, lit(" ") << dash_array_grammar, path_attributes.stroke_dasharray());
        karma::generate(output_iterator_, lit(" ") << attributes_grammar << lit("/>\n"), path_attributes);
    }*/

    template class svg_generator<std::ostream_iterator<char> >;
    }}
