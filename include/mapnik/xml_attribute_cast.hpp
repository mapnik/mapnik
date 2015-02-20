/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_XML_ATTRIBUTE_CAST_HPP
#define MAPNIK_XML_ATTRIBUTE_CAST_HPP

//mapnik
#include <mapnik/debug.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/xml_tree.hpp>
#include <mapnik/enumeration.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/color.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/text/font_feature_settings.hpp>

// boost
#include <boost/optional.hpp>

// stl
#include <string>
#include <map>
#include <typeinfo>
#include <utility>
#include <stdexcept>

namespace mapnik { namespace detail {

template <typename T>
struct do_xml_attribute_cast
{
    static inline boost::optional<T> xml_attribute_cast_impl(xml_tree const& /*tree*/, std::string const& /*source*/)
    {
        std::string err_msg("No conversion from std::string to");
        err_msg += std::string(typeid(T).name());
        throw std::runtime_error(err_msg);
    }
};

// specialization for mapnik::boolean_type
template <>
struct do_xml_attribute_cast<mapnik::boolean_type>
{
    static inline boost::optional<mapnik::boolean_type> xml_attribute_cast_impl(xml_tree const& /*tree*/, std::string const& source)
    {
        bool result;
        if (mapnik::util::string2bool(source, result))
            return boost::optional<mapnik::boolean_type>(result);
        return boost::optional<mapnik::boolean_type>();
    }
};

// specialization for int
template <>
struct do_xml_attribute_cast<int>
{
    static inline boost::optional<int> xml_attribute_cast_impl(xml_tree const& /*tree*/, std::string const& source)
    {
        int result;
        if (mapnik::util::string2int(source, result))
            return boost::optional<int>(result);
        return boost::optional<int>();
    }
};

#ifdef BIGINT
// specialization for long long
template <>
struct do_xml_attribute_cast<mapnik::value_integer>
{
    static inline boost::optional<mapnik::value_integer> xml_attribute_cast_impl(xml_tree const& /*tree*/, std::string const& source)
    {
        int result;
        if (mapnik::util::string2int(source, result))
            return boost::optional<mapnik::value_integer>(result);
        return boost::optional<mapnik::value_integer>();
    }
};

#endif

// specialization for unsigned

template <>
struct do_xml_attribute_cast<unsigned>
{
    static inline boost::optional<unsigned> xml_attribute_cast_impl(xml_tree const& /*tree*/, std::string const& source)
    {
        int result;
        if (mapnik::util::string2int(source, result))
            return boost::optional<unsigned>(result);
        return boost::optional<unsigned>();
    }
};

// specialization for float
template <>
struct do_xml_attribute_cast<float>
{
    static inline boost::optional<float> xml_attribute_cast_impl(xml_tree const& /*tree*/, std::string const& source)
    {
        float result;
        if (mapnik::util::string2float(source, result))
            return boost::optional<float>(result);
        return boost::optional<float>();
    }
};

// specialization for double
template <>
struct do_xml_attribute_cast<double>
{
    static inline boost::optional<double> xml_attribute_cast_impl(xml_tree const& /*tree*/, std::string const& source)
    {
        double result;
        if (mapnik::util::string2double(source, result))
            return boost::optional<double>(result);
        return boost::optional<double>();
    }
};

// specialization for mapnik::enumeration<T,int>
template <typename T, int MAX>
struct do_xml_attribute_cast<mapnik::enumeration<T,MAX> >
{
    static inline boost::optional<mapnik::enumeration<T,MAX> > xml_attribute_cast_impl(xml_tree const& /*tree*/, std::string const& source)
    {
        using result_type = typename boost::optional<mapnik::enumeration<T,MAX> >;
        try
        {
            mapnik::enumeration<T,MAX> e;
            e.from_string(source);
            return result_type(e);
        }
        catch (illegal_enum_value const& ex)
        {
            MAPNIK_LOG_ERROR(do_xml_attribute_cast) << ex.what();
            return result_type();
        }
    }
};

// specialization for mapnik::color
template <>
struct do_xml_attribute_cast<mapnik::color>
{
    static inline boost::optional<mapnik::color> xml_attribute_cast_impl(xml_tree const& tree, std::string const& source)
    {
        return parse_color(source);
    }
};

// specialization for std::string
template <>
struct do_xml_attribute_cast<std::string>
{
    static inline boost::optional<std::string> xml_attribute_cast_impl(xml_tree const& /*tree*/, std::string const& source)
    {
        return boost::optional<std::string>(source);
    }
};

// specialization for mapnik::expression_ptr
template <>
struct do_xml_attribute_cast<mapnik::expression_ptr>
{
    static inline boost::optional<mapnik::expression_ptr> xml_attribute_cast_impl(xml_tree const& tree, std::string const& source)
    {
        std::map<std::string,mapnik::expression_ptr>::const_iterator itr = tree.expr_cache_.find(source);
        if (itr != tree.expr_cache_.end())
        {
            return itr->second;
        }
        else
        {
            mapnik::expression_ptr expr = parse_expression(source);
            tree.expr_cache_.emplace(source,expr);
            return expr;
        }
    }
};

// specialization for mapnik::font_feature_settings
template <>
struct do_xml_attribute_cast<mapnik::font_feature_settings>
{
    static inline boost::optional<mapnik::font_feature_settings> xml_attribute_cast_impl(xml_tree const& tree, std::string const& source)
    {
        return mapnik::font_feature_settings(source);
    }
};

} // end namespace detail

template <typename T>
boost::optional<T> xml_attribute_cast(xml_tree const& tree, std::string const& source)
{
    return detail::do_xml_attribute_cast<T>::xml_attribute_cast_impl(tree, source);
}

} // end namespace mapnik

#endif // MAPNIK_XML_ATTRIBUTE_CAST_HPP
