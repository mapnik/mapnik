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
#include <boost/variant.hpp>

// mapnik
//symbolizer typdef here rather than mapnik/symbolizer.hpp
#include <mapnik/rule.hpp>
#include <mapnik/symbolizer_hash.hpp>

using mapnik::symbolizer;

using mapnik::rule;
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

struct get_symbolizer_type : public boost::static_visitor<std::string>
{
public:
    std::string operator () (point_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("point");
    }

    std::string operator () ( line_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("line");
    }

    std::string operator () (line_pattern_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("line_pattern");
    }

    std::string operator () (polygon_symbolizer const& sym ) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("polygon");
    }

    std::string operator () (polygon_pattern_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("polygon_pattern");
    }

    std::string operator () (raster_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("raster");
    }

    std::string operator () (shield_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("shield");
    }

    std::string operator () (text_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("text");
    }

    std::string operator () (building_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("building");
    }

    std::string operator () (markers_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("markers");
    }

    template <typename Symbolizer>
    std::string operator() ( Symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return std::string("unknown");
    }
};

std::string get_symbol_type(symbolizer const& symbol)
{
    std::string type = boost::apply_visitor( get_symbolizer_type(), symbol);
    return type;
}

const point_symbolizer& point_(symbolizer const& symbol )
{
    return boost::get<point_symbolizer>(symbol);
}

const line_symbolizer& line_( const symbolizer& symbol )
{
    return boost::get<line_symbolizer>(symbol);
}

const polygon_symbolizer& polygon_( const symbolizer& symbol )
{
    return boost::get<polygon_symbolizer>(symbol);
}

const raster_symbolizer& raster_( const symbolizer& symbol )
{
    return boost::get<raster_symbolizer>(symbol);
}

const text_symbolizer& text_( const symbolizer& symbol )
{
    return boost::get<text_symbolizer>(symbol);
}

const shield_symbolizer& shield_( const symbolizer& symbol )
{
    return boost::get<shield_symbolizer>(symbol);
}

const line_pattern_symbolizer& line_pattern_( const symbolizer& symbol )
{
    return boost::get<line_pattern_symbolizer>(symbol);
}

const polygon_pattern_symbolizer& polygon_pattern_( const symbolizer& symbol )
{
    return boost::get<polygon_pattern_symbolizer>(symbol);
}

const building_symbolizer& building_( const symbolizer& symbol )
{
    return boost::get<building_symbolizer>(symbol);
}

const markers_symbolizer& markers_( const symbolizer& symbol )
{
    return boost::get<markers_symbolizer>(symbol);
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

void export_symbolizer()
{
    using namespace boost::python;

    class_<symbolizer>("Symbolizer",no_init)

        .def("type",get_symbol_type)

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
}
