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

// NOTE: This is an implementation header file and is only meant to be included
//    from implementation files. It therefore doesn't have an include guard.

// mapnik
#include <mapnik/params.hpp>
#include <mapnik/value_types.hpp>

// boost
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp> // keep gcc happy
#include <boost/variant/variant.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

// stl
#include <string>

namespace mapnik
{

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

namespace params_detail {

template <typename T>
struct converter
{
    typedef boost::optional<T> return_type;
    static return_type extract(parameters const& params,
                               std::string const& name,
                               boost::optional<T> const& default_opt_value)
    {
        boost::optional<T> result(default_opt_value);
        parameters::const_iterator itr = params.find(name);
        if (itr != params.end())
        {
            boost::apply_visitor(value_extractor_visitor<T>(result),itr->second);
        }
        return result;
    }
};

} // end namespace params_detail


template <typename T>
boost::optional<T> parameters::get(std::string const& key) const
{
    return params_detail::converter<T>::extract(*this,key, boost::none);
}

template <typename T>
boost::optional<T> parameters::get(std::string const& key, T const& default_opt_value) const
{
    return params_detail::converter<T>::extract(*this,key,boost::optional<T>(default_opt_value));
}


}

