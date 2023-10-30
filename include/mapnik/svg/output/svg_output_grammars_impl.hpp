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

// NOTE: This is an implementation header file and is only meant to be included
//    from implementation files. It therefore doesn't have an include guard.

// mapnik
#include <mapnik/svg/output/svg_output_grammars.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/adapted/struct.hpp>
MAPNIK_DISABLE_WARNING_POP

/*!
 * mapnik::svg::path_output_attributes is adapted as a fusion sequence
 * in order to be used directly by the svg_path_attributes_grammar (below).
 *
 * This adaptation is the primary reason why the attributes are stored in
 * this structure before being passed to the generate_path method.
 */
BOOST_FUSION_ADAPT_STRUCT(mapnik::svg::path_output_attributes,
                          (std::string, fill_color_)(double, fill_opacity_)(std::string,
                                                                            stroke_color_)(double, stroke_opacity_)(
                            double,
                            stroke_width_)(std::string, stroke_linecap_)(std::string,
                                                                         stroke_linejoin_)(double, stroke_dashoffset_));

/*!
 * mapnik::svg::rect_output_attributes is adapted as a fusion sequence
 * in order to be used directly by the svg_rect_attributes_grammar (below).
 */
BOOST_FUSION_ADAPT_STRUCT(mapnik::svg::rect_output_attributes,
                          (int, x_)(int, y_)(unsigned, width_)(unsigned, height_)(std::string, fill_color_));

/*!
 * mapnik::svg::root_output_attributes is adapted as a fusion sequence
 * in order to be used directly by the svg_root_attributes_grammar (below).
 */
BOOST_FUSION_ADAPT_STRUCT(mapnik::svg::root_output_attributes,
                          (unsigned, width_)(unsigned, height_)(double, svg_version_)(std::string, svg_namespace_url_));

namespace mapnik {
namespace svg {

using namespace boost::spirit;

template<typename OutputIterator>
svg_path_attributes_grammar<OutputIterator>::svg_path_attributes_grammar()
    : svg_path_attributes_grammar::base_type(svg_path_attributes)
{
    karma::lit_type lit;
    karma::double_type double_;
    karma::string_type kstring;

    svg_path_attributes =                                                    //
      lit("fill=\"")                                                         //
      << kstring                                                             //
      << lit("\" fill-opacity=\"") << double_                                //
      << lit("\" stroke=\"") << kstring                                      //
      << lit("\" stroke-opacity=\"") << double_                              //
      << lit("\" stroke-width=\"") << double_ << lit("px")                   //
      << lit("\" stroke-linecap=\"") << kstring                              //
      << lit("\" stroke-linejoin=\"") << kstring                             //
      << lit("\" stroke-dashoffset=\"") << double_ << lit("px") << lit('"'); //
}

template<typename OutputIterator>
svg_path_dash_array_grammar<OutputIterator>::svg_path_dash_array_grammar()
    : svg_path_dash_array_grammar::base_type(svg_path_dash_array)
{
    karma::double_type double_;
    karma::lit_type lit;

    svg_path_dash_array = lit("stroke-dasharray=\"") << -((double_ << lit(',') << double_) % lit(',')) << lit('"');
}

template<typename OutputIterator>
svg_rect_attributes_grammar<OutputIterator>::svg_rect_attributes_grammar()
    : svg_rect_attributes_grammar::base_type(svg_rect_attributes)
{
    karma::lit_type lit;
    karma::int_type int_;
    karma::string_type kstring;

    svg_rect_attributes =                          //
      lit("x=\"")                                  //
      << int_                                      //
      << lit("\" y=\"") << int_                    //
      << lit("\" width=\"") << int_ << lit("px")   //
      << lit("\" height=\"") << int_ << lit("px")  //
      << lit("\" fill=\"") << kstring << lit('"'); //
}

template<typename OutputIterator>
svg_root_attributes_grammar<OutputIterator>::svg_root_attributes_grammar()
    : svg_root_attributes_grammar::base_type(svg_root_attributes)
{
    karma::lit_type lit;
    karma::int_type int_;
    karma::string_type kstring;
    karma::double_type double_;

    svg_root_attributes =                                                          //
      lit("width=\"")                                                              //
      << int_                                                                      //
      << lit("px")                                                                 //
      << lit("\" height=\"") << int_ << lit("px")                                  //
      << lit("\" version=\"") << double_                                           //
      << lit("\" xmlns=\"") << kstring                                             //
      << lit("\" xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\""); //
}

} // namespace svg
} // namespace mapnik
