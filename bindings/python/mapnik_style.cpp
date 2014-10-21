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

#include <mapnik/config.hpp>

// boost
#include "boost_std_shared_shim.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/value_error.hpp>
#include <mapnik/rule.hpp>
#include "mapnik_enumeration.hpp"
#include <mapnik/feature_type_style.hpp>
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
    std::vector<mapnik::filter::filter_type> new_filters;
    bool result = parse_image_filters(filters, new_filters);
    if (!result)
    {
        throw mapnik::value_error("failed to parse image-filters: '" + filters + "'");
    }
#ifdef _WINDOWS
    style.image_filters() = new_filters;
    // FIXME : https://svn.boost.org/trac/boost/ticket/2839
#else
    style.image_filters() = std::move(new_filters);
#endif
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
        .add_property("image_filters_inflate",
                      &feature_type_style::image_filters_inflate,
                      &feature_type_style::image_filters_inflate,
                      "Set/get the image_filters_inflate property of the style")
        .add_property("image_filters",
                      get_image_filters,
                      set_image_filters,
                      "Set/get the comp-op (composite operation) of the style")
        ;

}
