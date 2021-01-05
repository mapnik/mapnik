/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_SVG_GENERATOR_HPP
#define MAPNIK_SVG_GENERATOR_HPP

// mapnik
#include <mapnik/view_transform.hpp>
#include <mapnik/color.hpp>

#include <mapnik/svg/geometry_svg_generator.hpp>
#include <mapnik/svg/output/svg_output_attributes.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/value/types.hpp>


namespace mapnik { namespace svg {

    /*!
     * @brief Performs the actual generation of output calling the underlying library.
     * A method to generate each kind of SVG tag is provided. The information needed
     * needed to generate the attributes of a tag is passed within a *_output_attributes
     * structure.
     */
    template <typename OutputIterator>
    class svg_generator : private util::noncopyable
    {
    public:
        explicit svg_generator(OutputIterator& output_iterator);
        ~svg_generator();

        void generate_header();
        void generate_opening_root(root_output_attributes const& root_attributes);
        void generate_closing_root();
        void generate_rect(rect_output_attributes const& rect_attributes);
        void generate_opening_group(mapnik::value_integer val);
        void generate_opening_group(std::string const& val);
        void generate_closing_group();
        OutputIterator& output_iterator_;
    };
    }}

#endif // MAPNIK_SVG_GENERATOR_HPP
