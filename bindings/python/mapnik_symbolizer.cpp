/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/symbolizer.hpp>
#include <mapnik/symbolizer_hash.hpp>
#include <mapnik/symbolizer_utils.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/path_expression.hpp>
#include "mapnik_enumeration.hpp"
#include "mapnik_svg.hpp"
#include <mapnik/graphics.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/value_error.hpp>
#include <mapnik/marker_cache.hpp> // for known_svg_prefix_
#include <mapnik/group/group_layout.hpp>
#include <mapnik/group/group_rule.hpp>
#include <mapnik/group/group_symbolizer_properties.hpp>
#include <mapnik/util/variant.hpp>

// stl
#include <sstream>

using mapnik::symbolizer;
using mapnik::point_symbolizer;
using mapnik::line_symbolizer;
using mapnik::line_pattern_symbolizer;
using mapnik::polygon_symbolizer;
using mapnik::polygon_pattern_symbolizer;
using mapnik::raster_symbolizer;
using mapnik::shield_symbolizer;
using mapnik::text_symbolizer;
using mapnik::building_symbolizer;
using mapnik::markers_symbolizer;
using mapnik::debug_symbolizer;
using mapnik::group_symbolizer;
using mapnik::symbolizer_base;
using mapnik::color;
using mapnik::path_processor_type;
using mapnik::path_expression_ptr;
using mapnik::guess_type;
using mapnik::expression_ptr;
using mapnik::parse_path;


namespace {
using namespace boost::python;
void __setitem__(mapnik::symbolizer_base & sym, std::string const& name, mapnik::symbolizer_base::value_type const& val)
{
    put(sym, mapnik::get_key(name), val);
}

std::shared_ptr<mapnik::symbolizer_base::value_type> numeric_wrapper(const object& arg)
{
    std::shared_ptr<mapnik::symbolizer_base::value_type> result;
    if (PyBool_Check(arg.ptr()))
    {
        mapnik::value_bool val = extract<mapnik::value_bool>(arg);
        result.reset(new mapnik::symbolizer_base::value_type(val));
    }
    else if (PyFloat_Check(arg.ptr()))
    {
        mapnik::value_double val = extract<mapnik::value_double>(arg);
        result.reset(new mapnik::symbolizer_base::value_type(val));
    }
    else
    {
        mapnik::value_integer val = extract<mapnik::value_integer>(arg);
        result.reset(new mapnik::symbolizer_base::value_type(val));
    }
    return result;
}

struct extract_python_object : public mapnik::util::static_visitor<boost::python::object>
{
    using result_type = boost::python::object;

    template <typename T>
    auto operator() (T const& val) const -> result_type
    {
        return result_type(val); // wrap into python object
    }
};

boost::python::object __getitem__(mapnik::symbolizer_base const& sym, std::string const& name)
{
    using const_iterator = symbolizer_base::cont_type::const_iterator;
    mapnik::keys key = mapnik::get_key(name);
    const_iterator itr = sym.properties.find(key);
    if (itr != sym.properties.end())
    {
        return mapnik::util::apply_visitor(extract_python_object(), itr->second);
    }
    //mapnik::property_meta_type const& meta = mapnik::get_meta(key);
    //return mapnik::util::apply_visitor(extract_python_object(), std::get<1>(meta));
    return boost::python::object();
}

/*
std::string __str__(mapnik::symbolizer const& sym)
{
    return mapnik::util::apply_visitor(mapnik::symbolizer_to_json(), sym);
}
*/

std::string get_symbolizer_type(symbolizer const& sym)
{
    return mapnik::symbolizer_name(sym); // FIXME - do we need this ?
}

std::size_t hash_impl(symbolizer const& sym)
{
    return mapnik::util::apply_visitor(mapnik::symbolizer_hash_visitor(), sym);
}

template <typename T>
std::size_t hash_impl_2(T const& sym)
{
    return mapnik::symbolizer_hash::value<T>(sym);
}

struct extract_underlying_type_visitor : mapnik::util::static_visitor<boost::python::object>
{
    template <typename T>
    boost::python::object operator() (T const& sym) const
    {
        return boost::python::object(sym);
    }
};

boost::python::object extract_underlying_type(symbolizer const& sym)
{
    return mapnik::util::apply_visitor(extract_underlying_type_visitor(), sym);
}

}

void export_symbolizer()
{
    using namespace boost::python;

    //implicitly_convertible<mapnik::value_bool, mapnik::symbolizer_base::value_type>();
    implicitly_convertible<mapnik::value_integer, mapnik::symbolizer_base::value_type>();
    implicitly_convertible<mapnik::value_double, mapnik::symbolizer_base::value_type>();
    implicitly_convertible<std::string, mapnik::symbolizer_base::value_type>();
    implicitly_convertible<mapnik::color, mapnik::symbolizer_base::value_type>();
    implicitly_convertible<mapnik::expression_ptr, mapnik::symbolizer_base::value_type>();
    implicitly_convertible<mapnik::enumeration_wrapper, mapnik::symbolizer_base::value_type>();
    implicitly_convertible<std::shared_ptr<mapnik::group_symbolizer_properties>, mapnik::symbolizer_base::value_type>();

    enum_<mapnik::keys>("keys")
        .value("gamma", mapnik::keys::gamma)
        .value("gamma_method",mapnik::keys::gamma_method)
        ;

    class_<symbolizer>("Symbolizer",no_init)
        .def("type",get_symbolizer_type)
        .def("__hash__",hash_impl)
        .def("extract", extract_underlying_type)
        ;

    class_<symbolizer_base::value_type>("NumericWrapper")
        .def("__init__", make_constructor(numeric_wrapper))
        ;

    class_<symbolizer_base>("SymbolizerBase",no_init)
        .def("__setitem__",&__setitem__)
        .def("__setattr__",&__setitem__)
        .def("__getitem__",&__getitem__)
        .def("__getattr__",&__getitem__)
        //.def("__str__", &__str__)
        .def(self == self) // __eq__
        ;
}


void export_shield_symbolizer()
{
    using namespace boost::python;
    class_< shield_symbolizer, bases<text_symbolizer> >("ShieldSymbolizer",
                                                        init<>("Default ctor"))
        .def("__hash__",hash_impl_2<shield_symbolizer>)
        ;

}

void export_polygon_symbolizer()
{
    using namespace boost::python;

    class_<polygon_symbolizer, bases<symbolizer_base> >("PolygonSymbolizer",
                                                        init<>("Default ctor"))
        .def("__hash__",hash_impl_2<polygon_symbolizer>)
        ;

}

void export_polygon_pattern_symbolizer()
{
    using namespace boost::python;

    mapnik::enumeration_<mapnik::pattern_alignment_e>("pattern_alignment")
        .value("LOCAL",mapnik::LOCAL_ALIGNMENT)
        .value("GLOBAL",mapnik::GLOBAL_ALIGNMENT)
        ;

    class_<polygon_pattern_symbolizer>("PolygonPatternSymbolizer",
                                       init<>("Default ctor"))
        .def("__hash__",hash_impl_2<polygon_pattern_symbolizer>)
        ;
}

void export_raster_symbolizer()
{
    using namespace boost::python;

    class_<raster_symbolizer, bases<symbolizer_base> >("RasterSymbolizer",
                              init<>("Default ctor"))
        ;
}

void export_point_symbolizer()
{
    using namespace boost::python;

    mapnik::enumeration_<mapnik::point_placement_e>("point_placement")
        .value("CENTROID",mapnik::CENTROID_POINT_PLACEMENT)
        .value("INTERIOR",mapnik::INTERIOR_POINT_PLACEMENT)
        ;

    class_<point_symbolizer, bases<symbolizer_base> >("PointSymbolizer",
                             init<>("Default Point Symbolizer - 4x4 black square"))
        .def("__hash__",hash_impl_2<point_symbolizer>)
        ;
}

void export_markers_symbolizer()
{
    using namespace boost::python;

    mapnik::enumeration_<mapnik::marker_placement_e>("marker_placement")
        .value("POINT_PLACEMENT",mapnik::MARKER_POINT_PLACEMENT)
        .value("INTERIOR_PLACEMENT",mapnik::MARKER_INTERIOR_PLACEMENT)
        .value("LINE_PLACEMENT",mapnik::MARKER_LINE_PLACEMENT)
        ;

    mapnik::enumeration_<mapnik::marker_multi_policy_e>("marker_multi_policy")
        .value("EACH",mapnik::MARKER_EACH_MULTI)
        .value("WHOLE",mapnik::MARKER_WHOLE_MULTI)
        .value("LARGEST",mapnik::MARKER_LARGEST_MULTI)
        ;

    class_<markers_symbolizer, bases<symbolizer_base> >("MarkersSymbolizer",
                               init<>("Default Markers Symbolizer - circle"))
        .def("__hash__",hash_impl_2<markers_symbolizer>)
        ;
}


void export_line_symbolizer()
{
    using namespace boost::python;

    mapnik::enumeration_<mapnik::line_rasterizer_e>("line_rasterizer")
        .value("FULL",mapnik::RASTERIZER_FULL)
        .value("FAST",mapnik::RASTERIZER_FAST)
        ;

    mapnik::enumeration_<mapnik::line_cap_e>("stroke_linecap",
                             "The possible values for a line cap used when drawing\n"
                             "with a stroke.\n")
        .value("BUTT_CAP",mapnik::BUTT_CAP)
        .value("SQUARE_CAP",mapnik::SQUARE_CAP)
        .value("ROUND_CAP",mapnik::ROUND_CAP)
        ;

    mapnik::enumeration_<mapnik::line_join_e>("stroke_linejoin",
                                      "The possible values for the line joining mode\n"
                                      "when drawing with a stroke.\n")
        .value("MITER_JOIN",mapnik::MITER_JOIN)
        .value("MITER_REVERT_JOIN",mapnik::MITER_REVERT_JOIN)
        .value("ROUND_JOIN",mapnik::ROUND_JOIN)
        .value("BEVEL_JOIN",mapnik::BEVEL_JOIN)
        ;


    class_<line_symbolizer, bases<symbolizer_base> >("LineSymbolizer",
                            init<>("Default LineSymbolizer - 1px solid black"))
        .def("__hash__",hash_impl_2<line_symbolizer>)
        ;
}

void export_line_pattern_symbolizer()
{
    using namespace boost::python;

    class_<line_pattern_symbolizer, bases<symbolizer_base> >("LinePatternSymbolizer",
                                    init<> ("Default LinePatternSymbolizer"))
        .def("__hash__",hash_impl_2<line_pattern_symbolizer>)
        ;
}

void export_debug_symbolizer()
{
    using namespace boost::python;

    mapnik::enumeration_<mapnik::debug_symbolizer_mode_e>("debug_symbolizer_mode")
        .value("COLLISION",mapnik::DEBUG_SYM_MODE_COLLISION)
        .value("VERTEX",mapnik::DEBUG_SYM_MODE_VERTEX)
        ;

    class_<debug_symbolizer, bases<symbolizer_base> >("DebugSymbolizer",
                             init<>("Default debug Symbolizer"))
        .def("__hash__",hash_impl_2<debug_symbolizer>)
        ;
}

void export_building_symbolizer()
{
    using namespace boost::python;

    class_<building_symbolizer, bases<symbolizer_base> >("BuildingSymbolizer",
                               init<>("Default BuildingSymbolizer"))
        .def("__hash__",hash_impl_2<building_symbolizer>)
        ;

}

namespace {

void group_symbolizer_properties_set_layout_simple(mapnik::group_symbolizer_properties &p,
                                                   mapnik::simple_row_layout &s)
{
    p.set_layout(s);
}

void group_symbolizer_properties_set_layout_pair(mapnik::group_symbolizer_properties &p,
                                                 mapnik::pair_layout &s)
{
    p.set_layout(s);
}

std::shared_ptr<mapnik::group_rule> group_rule_construct1(mapnik::expression_ptr p)
{
    return std::make_shared<mapnik::group_rule>(p, mapnik::expression_ptr());
}

} // anonymous namespace

void export_group_symbolizer()
{
    using namespace boost::python;
    using mapnik::group_rule;
    using mapnik::simple_row_layout;
    using mapnik::pair_layout;
    using mapnik::group_symbolizer_properties;

    class_<group_rule, std::shared_ptr<group_rule> >("GroupRule",
                                                     init<expression_ptr, expression_ptr>())
        .def("__init__", boost::python::make_constructor(group_rule_construct1))
        .def("append", &group_rule::append)
        .def("set_filter", &group_rule::set_filter)
        .def("set_repeat_key", &group_rule::set_repeat_key)
        ;

    class_<simple_row_layout>("SimpleRowLayout")
        .def("item_margin", &simple_row_layout::get_item_margin)
        .def("set_item_margin", &simple_row_layout::set_item_margin)
        ;

    class_<pair_layout>("PairLayout")
        .def("item_margin", &simple_row_layout::get_item_margin)
        .def("set_item_margin", &simple_row_layout::set_item_margin)
        .def("max_difference", &pair_layout::get_max_difference)
        .def("set_max_difference", &pair_layout::set_max_difference)
        ;

    class_<group_symbolizer_properties, std::shared_ptr<group_symbolizer_properties> >("GroupSymbolizerProperties")
        .def("add_rule", &group_symbolizer_properties::add_rule)
        .def("set_layout", &group_symbolizer_properties_set_layout_simple)
        .def("set_layout", &group_symbolizer_properties_set_layout_pair)
        ;

    class_<group_symbolizer, bases<symbolizer_base> >("GroupSymbolizer",
                                                      init<>("Default GroupSymbolizer"))
        .def("__hash__",hash_impl_2<group_symbolizer>)
        ;

}
