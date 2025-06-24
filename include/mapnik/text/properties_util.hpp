/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_PROPERTIES_UTIL_HPP
#define MAPNIK_PROPERTIES_UTIL_HPP

#include <mapnik/symbolizer_base.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/config_error.hpp>

#include <string>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {
namespace detail {

template<typename T, class Enable = void>
struct is_mapnik_enumeration
{
    static constexpr bool value = false;
};

template<typename T>
struct is_mapnik_enumeration<T, typename std::enable_if_t<std::is_enum<typename T::native_type>::value>>
{
    static constexpr bool value = true;
};

template<typename T0, bool is_mapnik_enumeration = false>
struct set_property_from_xml_impl
{
    using target_type = T0;
    template<typename T1>
    static void apply(T1& val, char const* name, xml_node const& node)
    {
        try
        {
            const auto val_ = node.get_opt_attr<target_type>(name);
            if (val_)
                val = *val_;
        }
        catch (config_error const& ex)
        {
            const auto val_ = node.get_opt_attr<expression_ptr>(name);
            if (val_)
                val = *val_;
            else
            {
                ex.append_context(std::string("set_property_from_xml'" + std::string(name) + "'"), node);
            }
        }
    }
};

template<>
struct set_property_from_xml_impl<std::string, false>
{
    using target_type = std::string;
    template<typename T1>
    static void apply(T1& val, char const* name, xml_node const& node)
    {
        try
        {
            const auto val_ = node.get_opt_attr<expression_ptr>(name);
            if (!val_)
                throw config_error("Failed to extract property");
            val = *val_;
        }
        catch (config_error const& ex)
        {
            const auto val_ = node.get_opt_attr<target_type>(name);
            if (val_)
            {
                val = *val_;
            }
            else
            {
                ex.append_context(std::string("set_property_from_xml'" + std::string(name) + "'"), node);
            }
        }
    }
};

template<typename T0>
struct set_property_from_xml_impl<T0, true>
{
    using target_enum_type = T0;
    template<typename T1>
    static void apply(T1& val, char const* name, xml_node const& node)
    {
        try
        {
            const auto enum_str = node.get_opt_attr<std::string>(name);
            if (enum_str)
            {
                target_enum_type e;
                e.from_string(*enum_str);
                val = enumeration_wrapper(e.value_);
            }
        }
        catch (...)
        {
            const auto expr = node.get_opt_attr<expression_ptr>(name);
            if (expr)
                val = *expr;
            else
            {
                throw config_error(std::string("set_property_from_xml'" + std::string(name)));
            }
        }
    }
};

} // namespace detail

template<typename T0, typename T1>
void set_property_from_xml(T1& val, char const* name, xml_node const& node)
{
    detail::set_property_from_xml_impl<T0, detail::is_mapnik_enumeration<T0>::value>::apply(val, name, node);
}

void serialize_property(std::string const& name,
                        symbolizer_base::value_type const& val,
                        boost::property_tree::ptree& node);

} // namespace mapnik

#endif // MAPNIK_PROPERTIES_UTIL_HPP
