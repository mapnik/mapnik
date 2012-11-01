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

#ifndef MAPNIK_SVG_GENERATOR_HPP
#define MAPNIK_SVG_GENERATOR_HPP

// mapnik
#include <mapnik/ctrans.hpp>
#include <mapnik/color.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/svg/output/svg_output_grammars.hpp>
#include <mapnik/svg/output/svg_output_attributes.hpp>

// boost
#include <boost/utility.hpp>

namespace mapnik { namespace svg {

    /*!
     * @brief Performs the actual generation of output calling the underlying library.
     * A method to generate each kind of SVG tag is provided. The information needed
     * needed to generate the attributes of a tag is passed within a *_output_attributes
     * structure.
     */
    template <typename OutputIterator>
    class svg_generator : private boost::noncopyable
    {
        typedef coord_transform<CoordTransform, geometry_type> path_type;

        typedef svg::svg_root_attributes_grammar<OutputIterator> root_attributes_grammar;
        typedef svg::svg_rect_attributes_grammar<OutputIterator> rect_attributes_grammar;
        //typedef svg::svg_path_data_grammar<OutputIterator, path_type> path_data_grammar;
        typedef svg::svg_path_attributes_grammar<OutputIterator> path_attributes_grammar;
        typedef svg::svg_path_dash_array_grammar<OutputIterator> path_dash_array_grammar;

    public:
        explicit svg_generator(OutputIterator& output_iterator);
        ~svg_generator();

        void generate_header();
        void generate_opening_root(root_output_attributes const& root_attributes);
        void generate_closing_root();
        void generate_rect(rect_output_attributes const& rect_attributes);
        //void generate_path(path_type const& path, path_output_attributes const& path_attributes);

    private:
        OutputIterator& output_iterator_;
    };
    }}

#endif // MAPNIK_SVG_GENERATOR_HPP
