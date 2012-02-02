/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_PARAMS_HPP
#define MAPNIK_PARAMS_HPP

// boost
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/none.hpp>
#include <boost/lexical_cast.hpp>

// mapnik
#include <mapnik/value.hpp>

// stl
#include <string>
#include <map>

namespace mapnik
{
typedef boost::variant<value_null,int,double,std::string> value_holder;
typedef std::pair<std::string, value_holder> parameter;
typedef std::map<std::string, value_holder> param_map;

template <typename T>
struct value_extractor_visitor : public boost::static_visitor<>
{
    value_extractor_visitor(boost::optional<T> & var)
        :var_(var) {}

    void operator () (T val) const
    {
        var_ = val;
    }

    template <typename T1>
    void operator () (T1 val) const
    {
        try
        {
            var_ = boost::lexical_cast<T>(val);
        }
        catch (boost::bad_lexical_cast & ) {}
    }

    boost::optional<T> & var_;
};


class parameters : public param_map
{
    template <typename T>
    struct converter
    {
        typedef boost::optional<T> return_type;
        static return_type extract(parameters const& params,
                                   std::string const& name,
                                   boost::optional<T> const& default_value)
        {
            boost::optional<T> result(default_value);
            parameters::const_iterator itr = params.find(name);
            if (itr != params.end())
            {
                boost::apply_visitor(value_extractor_visitor<T>(result),itr->second);
            }
            return result;
        }
    };

public:

    parameters() {}

    template <typename T>
    boost::optional<T> get(std::string const& key) const
    {
        return converter<T>::extract(*this,key, boost::none);
    }

    template <typename T>
    boost::optional<T> get(std::string const& key, T const& default_value) const
    {
        return converter<T>::extract(*this,key,boost::optional<T>(default_value));
    }
};
}

#endif // MAPNIK_PARAMS_HPP
