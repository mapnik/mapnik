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

// FIXME: workaround incompatibility of karma with -DBOOST_SPIRIT_NO_PREDEFINED_TERMINALS=1
/*
boost/spirit/repository/home/karma/directive/confix.hpp:49:23: error: no member named
      'confix' in namespace 'boost::spirit::repository'
    using repository::confix;
*/
#undef BOOST_SPIRIT_NO_PREDEFINED_TERMINALS

// mapnik
#include <mapnik/ctrans.hpp>
#include <mapnik/color.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/util/geometry_svg_generator.hpp>
#include <mapnik/svg/output/svg_output_grammars.hpp>
#include <mapnik/svg/output/svg_output_attributes.hpp>
#include <mapnik/noncopyable.hpp>
#include <mapnik/value_types.hpp>


namespace mapnik { namespace svg {

    /*!
     * @brief Performs the actual generation of output calling the underlying library.
     * A method to generate each kind of SVG tag is provided. The information needed
     * needed to generate the attributes of a tag is passed within a *_output_attributes
     * structure.
     */
    template <typename OutputIterator>
    class svg_generator : private mapnik::noncopyable
    {
        typedef svg::svg_root_attributes_grammar<OutputIterator> root_attributes_grammar;
        typedef svg::svg_rect_attributes_grammar<OutputIterator> rect_attributes_grammar;
        typedef svg::svg_path_attributes_grammar<OutputIterator> path_attributes_grammar;
        typedef svg::svg_path_dash_array_grammar<OutputIterator> path_dash_array_grammar;

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
        template <typename PathType>
        void generate_path(PathType const& path, path_output_attributes const& path_attributes)
        {
            karma::lit_type lit;
            util::svg_generator<OutputIterator,PathType> svg_path_grammer;
            karma::generate(output_iterator_, lit("<path ") << svg_path_grammer, path);
            path_attributes_grammar attributes_grammar;
            path_dash_array_grammar dash_array_grammar;
            karma::generate(output_iterator_, lit(" ") << dash_array_grammar, path_attributes.stroke_dasharray());
            karma::generate(output_iterator_, lit(" ") << attributes_grammar << lit("/>\n"), path_attributes);
        }

    private:
        OutputIterator& output_iterator_;
    };
    }}

#endif // MAPNIK_SVG_GENERATOR_HPP
