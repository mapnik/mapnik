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

#ifndef MAPNIK_SYMBOLIZER_UTILS_HPP
#define MAPNIK_SYMBOLIZER_UTILS_HPP

// mapnik
#include <mapnik/boolean.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/path_expression.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/color.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/xml_tree.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/evaluate_global_attributes.hpp>
#include <mapnik/transform/transform_processor.hpp>
#include <mapnik/transform/parse_transform.hpp>
#include <mapnik/util/dasharray_parser.hpp>
#include <mapnik/util/variant.hpp>

namespace mapnik {

template<typename Symbolizer>
struct symbolizer_traits
{
    static char const* name() { return "Unknown"; }
};

template<>
struct symbolizer_traits<point_symbolizer>
{
    static char const* name() { return "PointSymbolizer"; }
};

template<>
struct symbolizer_traits<line_symbolizer>
{
    static char const* name() { return "LineSymbolizer"; }
};

template<>
struct symbolizer_traits<polygon_symbolizer>
{
    static char const* name() { return "PolygonSymbolizer"; }
};

template<>
struct symbolizer_traits<text_symbolizer>
{
    static char const* name() { return "TextSymbolizer"; }
};

template<>
struct symbolizer_traits<line_pattern_symbolizer>
{
    static char const* name() { return "LinePatternSymbolizer"; }
};

template<>
struct symbolizer_traits<polygon_pattern_symbolizer>
{
    static char const* name() { return "PolygonPatternSymbolizer"; }
};

template<>
struct symbolizer_traits<markers_symbolizer>
{
    static char const* name() { return "MarkersSymbolizer"; }
};

template<>
struct symbolizer_traits<shield_symbolizer>
{
    static char const* name() { return "ShieldSymbolizer"; }
};

template<>
struct symbolizer_traits<raster_symbolizer>
{
    static char const* name() { return "RasterSymbolizer"; }
};

template<>
struct symbolizer_traits<building_symbolizer>
{
    static char const* name() { return "BuildingSymbolizer"; }
};

template<>
struct symbolizer_traits<group_symbolizer>
{
    static char const* name() { return "GroupSymbolizer"; }
};

template<>
struct symbolizer_traits<debug_symbolizer>
{
    static char const* name() { return "DebugSymbolizer"; }
};

template<>
struct symbolizer_traits<dot_symbolizer>
{
    static char const* name() { return "DotSymbolizer"; }
};

// symbolizer name impl
namespace detail {

struct symbolizer_name_impl
{
  public:
    template<typename Symbolizer>
    std::string operator()(Symbolizer const&) const
    {
        return symbolizer_traits<Symbolizer>::name();
    }
};
} // namespace detail

inline std::string symbolizer_name(symbolizer const& sym)
{
    std::string type = util::apply_visitor(detail::symbolizer_name_impl(), sym);
    return type;
}

template<typename Meta>
class symbolizer_property_value_string
{
  public:
    symbolizer_property_value_string(Meta const& meta)
        : meta_(meta)
    {}

    std::string operator()(mapnik::enumeration_wrapper const& e) const
    {
        std::stringstream ss;
        auto const& convert_fun_ptr(std::get<1>(meta_));
        if (convert_fun_ptr)
        {
            ss << '\"' << convert_fun_ptr(e) << '\"';
        }
        return ss.str();
    }

    std::string operator()(path_expression_ptr const& expr) const
    {
        std::ostringstream ss;
        if (expr)
        {
            ss << '\"' << path_processor::to_string(*expr) << '\"';
        }
        return ss.str();
    }

    std::string operator()(text_placements_ptr const& expr) const
    {
        return std::string("\"<fixme-text-placement-ptr>\"");
    }

    std::string operator()(raster_colorizer_ptr const& expr) const
    {
        return std::string("\"<fixme-raster-colorizer-ptr>\"");
    }

    std::string operator()(transform_type const& expr) const
    {
        std::ostringstream ss;
        if (expr)
        {
            ss << '\"' << transform_processor_type::to_string(*expr) << '\"';
        }
        return ss.str();
    }

    std::string operator()(expression_ptr const& expr) const
    {
        std::ostringstream ss;
        if (expr)
        {
            ss << '\"' << mapnik::to_expression_string(*expr) << '\"';
        }
        return ss.str();
    }

    std::string operator()(color const& c) const
    {
        std::ostringstream ss;
        ss << '\"' << c << '\"';
        return ss.str();
    }

    std::string operator()(dash_array const& dash) const
    {
        std::ostringstream ss;
        ss << '[';
        for (std::size_t i = 0; i < dash.size(); ++i)
        {
            ss << dash[i].first << "," << dash[i].second;
            if (i + 1 < dash.size())
                ss << ',';
        }
        ss << ']';
        return ss.str();
    }

    std::string operator()(mapnik::value_double val) const
    {
        std::ostringstream ss;
        ss  << val;
        return ss.str();
    }

    std::string operator()(mapnik::value_integer val) const
    {
        std::ostringstream ss;
        ss  << val;
        return ss.str();
    }

    std::string operator()(mapnik::value_bool val) const
    {
        std::ostringstream ss;
        ss  << mapnik::boolean_type(val);
        return ss.str();
    }

    template<typename T>
    std::string operator()(T const& val) const
    {
        std::ostringstream ss;
        ss << '\"' << val << '\"';
        return ss.str();
    }

  private:
    Meta const& meta_;
};

struct symbolizer_to_json
{
    using result_type = std::string;

    template<typename T>
    auto operator()(T const& sym) const -> result_type
    {
        std::stringstream ss;
        ss << "{\"type\":\"" << mapnik::symbolizer_traits<T>::name() << "\",";
        ss << "\"properties\":{";
        bool first = true;
        for (auto const& prop : sym.properties)
        {
            auto const& meta = mapnik::get_meta(prop.first);
            if (first)
                first = false;
            else
                ss << ",";
            ss << "\"" << std::get<0>(meta) << "\":";
            ss << util::apply_visitor(symbolizer_property_value_string<property_meta_type>(meta), prop.second);
        }
        ss << "}}";
        return ss.str();
    }
};

namespace {

template<typename Symbolizer, typename T>
struct set_property_impl
{
    static void apply(Symbolizer&, mapnik::keys, std::string const&) { std::cerr << "do nothing" << std::endl; }
};

template<typename Symbolizer>
struct set_property_impl<Symbolizer, std::integral_constant<property_types, property_types::target_color>>
{
    static void apply(Symbolizer& sym, mapnik::keys key, std::string const& val)
    {
        put(sym, key, mapnik::parse_color(val));
    }
};

template<typename Symbolizer>
struct set_property_impl<Symbolizer, std::integral_constant<property_types, property_types::target_double>>
{
    static void apply(Symbolizer&, mapnik::keys, std::string const&) { std::cerr << " expects double" << std::endl; }
};

template<typename Symbolizer>
struct set_property_impl<Symbolizer, std::integral_constant<property_types, property_types::target_bool>>
{
    static void apply(Symbolizer&, mapnik::keys, std::string const&) { std::cerr << " expects bool" << std::endl; }
};

} // namespace

template<typename Symbolizer, typename T>
inline void set_property(Symbolizer& sym, mapnik::keys key, T const& val)
{
    switch (std::get<2>(get_meta(key)))
    {
        case property_types::target_bool:
            set_property_impl<Symbolizer, std::integral_constant<property_types, property_types::target_bool>>::apply(
              sym,
              key,
              val);
            break;
        case property_types::target_integer:
            set_property_impl<Symbolizer,
                              std::integral_constant<property_types, property_types::target_integer>>::apply(sym,
                                                                                                             key,
                                                                                                             val);
            break;
        case property_types::target_double:
            set_property_impl<Symbolizer, std::integral_constant<property_types, property_types::target_double>>::apply(
              sym,
              key,
              val);
            break;
        case property_types::target_color:
            set_property_impl<Symbolizer, std::integral_constant<property_types, property_types::target_color>>::apply(
              sym,
              key,
              val);
            break;
        default:
            break;
    }
}

template<typename Symbolizer, typename T>
inline void set_property_from_value(Symbolizer& sym, mapnik::keys key, T const& val)
{
    switch (std::get<2>(get_meta(key)))
    {
        case property_types::target_bool:
            put(sym, key, val.to_bool());
            break;
        case property_types::target_integer:
            put(sym, key, val.to_int());
            break;
        case property_types::target_double:
            put(sym, key, val.to_double());
            break;
        case property_types::target_color:
            put(sym, key, mapnik::parse_color(val.to_string()));
            break;
        default:
            break;
    }
}

namespace detail {
// helpers
template<typename Symbolizer, typename T, bool is_enum = false>
struct set_symbolizer_property_impl
{
    static void apply(Symbolizer& sym, keys key, std::string const& name, xml_node const& node)
    {
        using value_type = T;
        try
        {
            boost::optional<value_type> val = node.get_opt_attr<value_type>(name);
            if (val)
                put(sym, key, *val);
        }
        catch (config_error const& ex)
        {
            // try parsing as an expression
            boost::optional<expression_ptr> val = node.get_opt_attr<expression_ptr>(name);
            if (val)
            {
                // first try pre-evaluate expressions which don't have dynamic properties
                auto result = pre_evaluate_expression<mapnik::value>(*val);
                if (std::get<1>(result))
                {
                    set_property_from_value(sym, key, std::get<0>(result));
                }
                else
                {
                    // expression_ptr
                    put(sym, key, *val);
                }
            }
            else
            {
                throw;
            }
        }
    }
};

template<typename Symbolizer>
struct set_symbolizer_property_impl<Symbolizer, transform_type, false>
{
    static void apply(Symbolizer& sym, keys key, std::string const& name, xml_node const& node)
    {
        boost::optional<std::string> transform = node.get_opt_attr<std::string>(name);
        if (transform)
            put(sym, key, mapnik::parse_transform(*transform));
    }
};

template<typename Symbolizer>
struct set_symbolizer_property_impl<Symbolizer, dash_array, false>
{
    static void apply(Symbolizer& sym, keys key, std::string const& name, xml_node const& node)
    {
        boost::optional<std::string> str = node.get_opt_attr<std::string>(name);
        if (str)
        {
            dash_array dash;
            if (util::parse_dasharray(*str, dash))
            {
                put(sym, key, dash);
            }
            else
            {
                boost::optional<expression_ptr> val = node.get_opt_attr<expression_ptr>(name);
                if (val)
                {
                    // first try pre-evaluate expressions which don't have dynamic properties
                    auto result = pre_evaluate_expression<mapnik::value>(*val);
                    if (std::get<1>(result))
                    {
                        set_property_from_value(sym, key, std::get<0>(result));
                    }
                    else
                    {
                        // expression_ptr
                        put(sym, key, *val);
                    }
                }
                else
                {
                    throw config_error(std::string("Failed to parse dasharray ") + "'. Expected a " +
                                       "list of floats or 'none' but got '" + (*str) + "'");
                }
            }
        }
    }
};

template<typename Symbolizer, typename T>
struct set_symbolizer_property_impl<Symbolizer, T, true>
{
    static void apply(Symbolizer& sym, keys key, std::string const& name, xml_node const& node)
    {
        using value_type = T;
        boost::optional<std::string> enum_str = node.get_opt_attr<std::string>(name);
        if (enum_str)
        {
            boost::optional<value_type> enum_val = detail::enum_traits<value_type>::from_string(*enum_str);
            if (enum_val)
            {
                put(sym, key, *enum_val);
            }
            else
            {
                boost::optional<expression_ptr> val = node.get_opt_attr<expression_ptr>(name);
                if (val)
                {
                    // first try pre-evaluating expression
                    auto result = pre_evaluate_expression<value>(*val);
                    if (std::get<1>(result))
                    {
                        boost::optional<value_type> enum_val2 =
                          detail::enum_traits<value_type>::from_string(std::get<0>(result).to_string());
                        if (enum_val2)
                        {
                            put(sym, key, *enum_val);
                        }
                        else
                        {
                            // can't evaluate
                            throw config_error("failed to parse '" + name + "'");
                        }
                    }
                    else
                    {
                        // put expression_ptr
                        put(sym, key, *val);
                    }
                }
                else
                {
                    throw config_error("failed to parse '" + name + "'");
                }
            }
        }
    }
};

} // namespace detail

template<typename Symbolizer, typename T>
void set_symbolizer_property(Symbolizer& sym, keys key, xml_node const& node)
{
    std::string const& name = std::get<0>(get_meta(key));
    if (node.has_attribute(name))
    {
        detail::set_symbolizer_property_impl<Symbolizer, T, std::is_enum<T>::value>::apply(sym, key, name, node);
    }
}

} // namespace mapnik

#endif // MAPNIK_SYMBOLIZER_UTILS_HPP
