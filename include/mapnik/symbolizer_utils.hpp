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

#ifndef MAPNIK_SYMBOLIZER_UTILS_HPP
#define MAPNIK_SYMBOLIZER_UTILS_HPP

// mapnik
#include <mapnik/expression_string.hpp>
#include <mapnik/transform_processor.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/symbolizer.hpp>

// boost
#include <boost/variant/apply_visitor.hpp>

namespace mapnik {

template <typename Symbolizer>
struct symbolizer_traits
{
    static char const* name() { return "Unknown";}
};

template<>
struct symbolizer_traits<point_symbolizer>
{
    static char const* name() { return "PointSymbolizer";}
};

template<>
struct symbolizer_traits<line_symbolizer>
{
    static char const* name() { return "LineSymbolizer";}
};

template<>
struct symbolizer_traits<polygon_symbolizer>
{
    static char const* name() { return "PolygonSymbolizer";}
};

template<>
struct symbolizer_traits<text_symbolizer>
{
    static char const* name() { return "TextSymbolizer";}
};

template<>
struct symbolizer_traits<line_pattern_symbolizer>
{
    static char const* name() { return "LinePatternSymbolizer";}
};

template<>
struct symbolizer_traits<polygon_pattern_symbolizer>
{
    static char const* name() { return "PolygonPatternSymbolizer";}
};

template<>
struct symbolizer_traits<markers_symbolizer>
{
    static char const* name() { return "MarkersSymbolizer";}
};

template<>
struct symbolizer_traits<shield_symbolizer>
{
    static char const* name() { return "ShieldSymbolizer";}
};

template<>
struct symbolizer_traits<raster_symbolizer>
{
    static char const* name() { return "RasterSymbolizer";}
};

template<>
struct symbolizer_traits<building_symbolizer>
{
    static char const* name() { return "BuildingSymbolizer";}
};

template<>
struct symbolizer_traits<group_symbolizer>
{
    static char const* name() { return "GroupSymbolizer";}
};

template<>
struct symbolizer_traits<debug_symbolizer>
{
    static char const* name() { return "DebugSymbolizer";}
};

// symbolizer name impl
namespace detail {

struct symbolizer_name_impl : public boost::static_visitor<std::string>
{
public:
    template <typename Symbolizer>
    std::string operator () (Symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return symbolizer_traits<Symbolizer>::name();
    }
};
}

std::string symbolizer_name(symbolizer const& sym)
{
    std::string type = boost::apply_visitor( detail::symbolizer_name_impl(), sym);
    return type;
}


template <typename Meta>
class symbolizer_property_value_string : public boost::static_visitor<std::string>
{
public:
    symbolizer_property_value_string (Meta const& meta)
        : meta_(meta) {}

    std::string operator() ( mapnik::enumeration_wrapper const& e) const
    {
        std::stringstream ss;
        auto const& convert_fun_ptr(std::get<2>(meta_));
        if ( convert_fun_ptr )
        {
            ss << convert_fun_ptr(e);
        }
        return ss.str();
    }

    std::string operator () ( path_expression_ptr const& expr) const
    {
        std::ostringstream ss;
        if (expr)
        {
            ss << '\"' << path_processor::to_string(*expr) << '\"';
        }
        return ss.str();
    }

    std::string operator () (text_placements_ptr const& expr) const
    {
        return std::string("\"<fixme-text-placement-ptr>\"");
    }

    std::string operator () (raster_colorizer_ptr const& expr) const
    {
        return std::string("\"<fixme-raster-colorizer-ptr>\"");
    }

    std::string operator () (transform_type const& expr) const
    {
        std::ostringstream ss;
        if (expr)
        {
            ss << '\"' << transform_processor_type::to_string(*expr) << '\"';
        }
        return ss.str();
    }

    std::string operator () (expression_ptr const& expr) const
    {
        std::ostringstream ss;
        if (expr)
        {
            ss << '\"' << "FIXME" /*mapnik::to_expression_string(*expr)*/ <<  '\"';
        }
        return ss.str();
    }

    std::string operator () (color const& c) const
    {
        std::ostringstream ss;
        ss << '\"' << c << '\"';
        return ss.str();
    }

    std::string operator () (dash_array const& dash) const
    {
        std::ostringstream ss;
        for (std::size_t i = 0; i < dash.size(); ++i)
        {
            ss << dash[i].first << ", " << dash[i].second;
            if ( i + 1 < dash.size() ) ss << ',';
        }
        return ss.str();
    }

    template <typename T>
    std::string operator () ( T const& val ) const
    {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }

private:
    Meta const& meta_;
};

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
            ss << boost::apply_visitor(symbolizer_property_value_string<property_meta_type>(meta),prop.second);
        }
        ss << "}}";
        return ss.str();
    }
};

namespace {

template <typename Symbolizer, typename T>
struct set_property_impl
{
    static void apply(Symbolizer & sym, mapnik::keys key, std::string const& val)
    {
        std::cerr << "do nothing" << std::endl;
    }
};

template <typename Symbolizer>
struct set_property_impl<Symbolizer, std::integral_constant<property_types, property_types::target_color> >
{
    static void apply(Symbolizer & sym, mapnik::keys key, std::string const& val)
    {
        put(sym, key, mapnik::parse_color(val));
    }
};

template <typename Symbolizer>
struct set_property_impl<Symbolizer, std::integral_constant<property_types, property_types::target_double> >
{
    static void apply(Symbolizer & sym, mapnik::keys key, std::string const& val)
    {
        std::cerr << " expects double" << std::endl;
    }
};

template <typename Symbolizer>
struct set_property_impl<Symbolizer, std::integral_constant<property_types, property_types::target_bool> >
{
    static void apply(Symbolizer & sym, mapnik::keys key, std::string const& val)
    {
        std::cerr << " expects bool" << std::endl;
    }
};

}

template <typename Symbolizer, typename T>
inline void set_property(Symbolizer & sym, mapnik::keys key, T const& val)
{
    switch (std::get<3>(get_meta(key)))
    {
    case property_types::target_bool:
        set_property_impl<Symbolizer, std::integral_constant<property_types, property_types::target_bool> >::apply(sym,key,val);
        break;
    case property_types::target_integer:
        set_property_impl<Symbolizer, std::integral_constant<property_types, property_types::target_integer> >::apply(sym,key,val);
        break;
    case property_types::target_double:
        set_property_impl<Symbolizer, std::integral_constant<property_types, property_types::target_double> >::apply(sym,key,val);
        break;
    case property_types::target_color:
        set_property_impl<Symbolizer, std::integral_constant<property_types, property_types::target_color> >::apply(sym,key,val);
        break;
    default:
        break;
    }
}

}

#endif // MAPNIK_SYMBOLIZER_UTILS_HPP
