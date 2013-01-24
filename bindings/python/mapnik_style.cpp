/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon
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

// boost
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

// mapnik
#include <mapnik/value_error.hpp>
#include "mapnik_enumeration.hpp"
#include <mapnik/feature_type_style.hpp>
#include <mapnik/image_filter_grammar.hpp> // image_filter_grammar
#include <mapnik/image_filter_types.hpp> // generate_image_filters

using mapnik::feature_type_style;
using mapnik::rules;
using mapnik::rule;

std::string get_image_filters(feature_type_style & style)
{
    std::string filters_str;
    std::back_insert_iterator<std::string> sink(filters_str);
    generate_image_filters(sink, style.image_filters());
    return filters_str;
}

void set_image_filters(feature_type_style & style, std::string const& filters)
{
    std::string::const_iterator itr = filters.begin();
    std::string::const_iterator end = filters.end();
    mapnik::image_filter_grammar<std::string::const_iterator,
                                 std::vector<mapnik::filter::filter_type> > filter_grammar;
    std::vector<mapnik::filter::filter_type> new_filters;
    bool result = boost::spirit::qi::phrase_parse(itr,end,
                                                  filter_grammar,
                                                  boost::spirit::qi::ascii::space,
                                                  new_filters);
    if (!result || itr!=end)
    {
        throw mapnik::value_error("failed to parse image-filters: '" + std::string(itr,end) + "'");
    }
    style.image_filters().swap(new_filters);
}

void export_style()
{
    using namespace boost::python;

    mapnik::enumeration_<mapnik::filter_mode_e>("filter_mode")
        .value("ALL",mapnik::FILTER_ALL)
        .value("FIRST",mapnik::FILTER_FIRST)
        ;

    class_<rules>("Rules",init<>("default ctor"))
        .def(vector_indexing_suite<rules>())
        ;
    class_<feature_type_style>("Style",init<>("default style constructor"))

        .add_property("rules",make_function
                      (&feature_type_style::get_rules,
                       return_value_policy<reference_existing_object>()),
                      "List of rules belonging to a style as rule objects.\n"
                      "\n"
                      "Usage:\n"
                      ">>> for r in m.find_style('style 1').rules:\n"
                      ">>>    print r\n"
                      "<mapnik._mapnik.Rule object at 0x100549910>\n"
                      "<mapnik._mapnik.Rule object at 0x100549980>\n"
            )
        .add_property("filter_mode",
                      &feature_type_style::get_filter_mode,
                      &feature_type_style::set_filter_mode,
                      "Set/get the filter mode of the style")
        .add_property("opacity",
                      &feature_type_style::get_opacity,
                      &feature_type_style::set_opacity,
                      "Set/get the opacity of the style")
        .add_property("comp_op",
                      &feature_type_style::comp_op,
                      &feature_type_style::set_comp_op,
                      "Set/get the comp-op (composite operation) of the style")
        .add_property("image_filters",
                      get_image_filters,
                      set_image_filters,
                      "Set/get the comp-op (composite operation) of the style")
        ;

}

