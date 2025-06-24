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

#include <mapnik/text/properties_util.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/text/font_feature_settings.hpp>

namespace mapnik {
namespace detail {

struct property_serializer
{
    property_serializer(std::string const& name, boost::property_tree::ptree& node)
        : name_(name)
        , node_(node)
    {}

    void operator()(expression_ptr const& expr) const
    {
        if (expr)
            node_.put("<xmlattr>." + name_, to_expression_string(*expr));
    }

    void operator()(value_bool val) const { node_.put("<xmlattr>." + name_, val); }

    void operator()(value_integer val) const { node_.put("<xmlattr>." + name_, val); }

    void operator()(value_double val) const { node_.put("<xmlattr>." + name_, val); }

    void operator()(std::string const& val) const { node_.put("<xmlattr>." + name_, val); }

    void operator()(color const& val) const { node_.put("<xmlattr>." + name_, val); }

    void operator()(enumeration_wrapper const& val) const
    {
        std::string str = std::get<1>(get_meta(get_key(name_)))(val);
        node_.put("<xmlattr>." + name_, str);
    }

    void operator()(font_feature_settings const& val) const
    {
        std::string str = val.to_string();
        node_.put("<xmlattr>." + name_, str);
    }

    template<typename T>
    void operator()(T const& val) const
    {
        std::cerr << "NOOP" << std::endl;
    }

    std::string const& name_;
    boost::property_tree::ptree& node_;
};
} // namespace detail

void serialize_property(std::string const& name,
                        symbolizer_base::value_type const& val,
                        boost::property_tree::ptree& node)
{
    util::apply_visitor(detail::property_serializer(name, node), val);
}

} // namespace mapnik
