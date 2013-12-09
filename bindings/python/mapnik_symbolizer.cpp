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

#include "boost_std_shared_shim.hpp" // FIXME - do we need it?
// The functions in this file produce deprecation warnings.
// But as shield symbolizer doesn't fully support more than one
// placement from python yet these functions are actually the
// correct ones.

#define NO_DEPRECATION_WARNINGS

// boost
#include <boost/python.hpp>
#include <boost/variant.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
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

struct extract_python_object : public boost::static_visitor<boost::python::object>
{
    typedef boost::python::object result_type;

    template <typename T>
    auto operator() (T const& val) const -> result_type
    {
        return result_type(val); // wrap into python object
    }
};

boost::python::object __getitem__(mapnik::symbolizer_base const& sym, std::string const& name)
{
    typedef symbolizer_base::cont_type::const_iterator const_iterator;
    mapnik::keys key = mapnik::get_key(name);
    const_iterator itr = sym.properties.find(key);
    if (itr != sym.properties.end())
    {
        return boost::apply_visitor(extract_python_object(), itr->second);
    }
    //mapnik::property_meta_type const& meta = mapnik::get_meta(key);
    //return boost::apply_visitor(extract_python_object(), std::get<1>(meta));
    return boost::python::object();
}

struct symbolizer_to_json : public boost::static_visitor<std::string>
{
    typedef std::string result_type;

    template <typename T>
    auto operator() (T const& sym) const -> result_type
    {
        std::stringstream ss;
        ss << "{\"type\":\"" << mapnik::symbolizer_traits<T>::name() << "\",";
        ss << "\"properties\":{";
        bool first = true;
        for (auto const& prop : sym.properties)
        {
            auto const& meta = mapnik::get_meta(prop.first);
            if (first) first = false;
            else ss << ",";
            ss << "\"" <<  std::get<0>(meta) << "\":";
            ss << boost::apply_visitor(mapnik::symbolizer_property_value_string<mapnik::property_meta_type>(meta),prop.second);
        }
        ss << "}}";
        return ss.str();
    }
};

std::string __str__(mapnik::symbolizer const& sym)
{
    return boost::apply_visitor(symbolizer_to_json(), sym);
}

std::string get_symbolizer_type(symbolizer const& sym)
{
    return mapnik::symbolizer_name(sym); // FIXME - do we need this ?
}

const point_symbolizer& point_(symbolizer const& sym )
{
    return boost::get<point_symbolizer>(sym);
}

const line_symbolizer& line_( const symbolizer& sym )
{
    return boost::get<line_symbolizer>(sym);
}

const polygon_symbolizer& polygon_( const symbolizer& sym )
{
    return boost::get<polygon_symbolizer>(sym);
}

const raster_symbolizer& raster_( const symbolizer& sym )
{
    return boost::get<raster_symbolizer>(sym);
}

const text_symbolizer& text_( const symbolizer& sym )
{
    return boost::get<text_symbolizer>(sym);
}

const shield_symbolizer& shield_( const symbolizer& sym )
{
    return boost::get<shield_symbolizer>(sym);
}

const line_pattern_symbolizer& line_pattern_( const symbolizer& sym )
{
    return boost::get<line_pattern_symbolizer>(sym);
}

const polygon_pattern_symbolizer& polygon_pattern_( const symbolizer& sym )
{
    return boost::get<polygon_pattern_symbolizer>(sym);
}

const building_symbolizer& building_( const symbolizer& sym )
{
    return boost::get<building_symbolizer>(sym);
}

const markers_symbolizer& markers_( const symbolizer& sym )
{
    return boost::get<markers_symbolizer>(sym);
}

struct symbolizer_hash_visitor : public boost::static_visitor<std::size_t>
{
    template <typename T>
    std::size_t operator() (T const& sym) const
    {
        return mapnik::symbolizer_hash::value(sym);
    }
};

std::size_t hash_impl(symbolizer const& sym)
{
    return boost::apply_visitor(symbolizer_hash_visitor(), sym);
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


    enum_<mapnik::keys>("keys")
        .value("gamma", mapnik::keys::gamma)
        .value("gamma_method",mapnik::keys::gamma_method)
        ;

    class_<symbolizer>("Symbolizer",no_init)
        .def("type",get_symbolizer_type)
        .def("__hash__",hash_impl)
        .def("point",point_,
             return_value_policy<copy_const_reference>())
        .def("line",line_,
             return_value_policy<copy_const_reference>())
        .def("line_pattern",line_pattern_,
             return_value_policy<copy_const_reference>())
        .def("polygon",polygon_,
             return_value_policy<copy_const_reference>())
        .def("polygon_pattern",polygon_pattern_,
             return_value_policy<copy_const_reference>())
        .def("raster",raster_,
             return_value_policy<copy_const_reference>())
        .def("shield",shield_,
             return_value_policy<copy_const_reference>())
        .def("text",text_,
             return_value_policy<copy_const_reference>())
        .def("building",building_,
             return_value_policy<copy_const_reference>())
        .def("markers",markers_,
             return_value_policy<copy_const_reference>())
        ;

    class_<symbolizer_base::value_type>("NumericWrapper")
        .def("__init__", make_constructor(numeric_wrapper))
        ;

    class_<symbolizer_base>("SymbolizerBase",no_init)
        .def("__setitem__",&__setitem__)
        .def("__setattr__",&__setitem__)
        .def("__getitem__",&__getitem__)
        .def("__getattr__",&__getitem__)
        .def("__str__", &__str__)
        ;
}


void export_shield_symbolizer()
{
    using namespace boost::python;
    class_< shield_symbolizer, bases<text_symbolizer> >("ShieldSymbolizer",
                                                        init<>("Default ctor"))
        ;

}

void export_polygon_symbolizer()
{
    using namespace boost::python;

    class_<polygon_symbolizer, bases<symbolizer_base> >("PolygonSymbolizer",
                                                        init<>("Default ctor"))
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
        ;
}


void export_line_symbolizer()
{
    using namespace boost::python;
    mapnik::enumeration_<mapnik::line_rasterizer_e>("line_rasterizer")
        .value("FULL",mapnik::RASTERIZER_FULL)
        .value("FAST",mapnik::RASTERIZER_FAST)
        ;

    class_<line_symbolizer, bases<symbolizer_base> >("LineSymbolizer",
                            init<>("Default LineSymbolizer - 1px solid black"))
        ;
}

void export_line_pattern_symbolizer()
{
    using namespace boost::python;

    class_<line_pattern_symbolizer, bases<symbolizer_base> >("LinePatternSymbolizer",
                                    init<> ("Default LinePatternSymbolizer"))
        ;
}

void export_debug_symbolizer()
{
    using namespace boost::python;

    mapnik::enumeration_<mapnik::debug_symbolizer_mode_e>("debug_symbolizer_mode")
        .value("COLLISION",mapnik::DEBUG_SYM_MODE_COLLISION)
        .value("VERTEX",mapnik::DEBUG_SYM_MODE_VERTEX)
        ;

    class_<mapnik::debug_symbolizer, bases<symbolizer_base> >("DebugSymbolizer",
                             init<>("Default debug Symbolizer"))
        ;
}

void export_building_symbolizer()
{
    using namespace boost::python;

    class_<building_symbolizer, bases<symbolizer_base> >("BuildingSymbolizer",
                               init<>("Default BuildingSymbolizer"))
        ;

}
