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

// NOTE: This is an implementation header file and is only meant to be included
//    from implementation files. It therefore doesn't have an include guard.

// mapnik
#include <mapnik/params.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/util/conversions.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/lexical_cast.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <string>
#include <sstream>
#include <stdexcept>
#include <optional>

namespace mapnik {
namespace detail {

template<typename T>
struct extract_value
{
    static inline std::optional<T> do_extract_from_string(std::string const& /*source*/)
    {
        std::ostringstream s;
        s << "No conversion from std::string to " << typeid(T).name();
        throw std::runtime_error(s.str());
    }
    static inline std::optional<T> do_extract_from_bool(value_bool const& /*source*/)
    {
        std::ostringstream s;
        s << "No conversion from boolean to " << typeid(T).name();
        throw std::runtime_error(s.str());
    }
};

template<>
struct extract_value<value_bool>
{
    static inline std::optional<value_bool> do_extract_from_string(std::string const& source)
    {
        bool result;
        if (mapnik::util::string2bool(source, result))
            return std::optional<value_bool>(result);
        return std::nullopt;
    }

    static inline std::optional<value_bool> do_extract_from_bool(value_bool const& source) { return source; }
};

template<>
struct extract_value<mapnik::boolean_type>
{
    static inline std::optional<mapnik::boolean_type> do_extract_from_string(std::string const& source)
    {
        bool result;
        if (mapnik::util::string2bool(source, result))
            return std::optional<mapnik::boolean_type>(result);
        return std::nullopt;
    }

    static inline std::optional<mapnik::boolean_type> do_extract_from_bool(value_bool const& source)
    {
        return std::optional<mapnik::boolean_type>(source);
    }
};

template<>
struct extract_value<mapnik::value_integer>
{
    static inline std::optional<mapnik::value_integer> do_extract_from_string(std::string const& source)
    {
        mapnik::value_integer result;
        if (mapnik::util::string2int(source, result))
            return std::optional<mapnik::value_integer>(result);
        return std::nullopt;
    }
    static inline std::optional<mapnik::value_integer> do_extract_from_bool(value_bool const& source)
    {
        return std::optional<mapnik::value_integer>(boost::lexical_cast<mapnik::value_integer>(source));
    }
};

template<>
struct extract_value<mapnik::value_double>
{
    static inline std::optional<mapnik::value_double> do_extract_from_string(std::string const& source)
    {
        mapnik::value_double result;
        if (mapnik::util::string2double(source, result))
            return std::optional<double>(result);
        return std::nullopt;
    }

    static inline std::optional<mapnik::value_double> do_extract_from_bool(value_bool const& source)
    {
        return std::optional<double>(boost::lexical_cast<double>(source));
    }
};

template<>
struct extract_value<mapnik::value_null>
{
    static inline std::optional<mapnik::value_null> do_extract_from_string(std::string const&)
    {
        return std::nullopt; // FIXME
    }

    static inline std::optional<mapnik::value_null> do_extract_from_bool(value_bool const&)
    {
        return std::nullopt; // FIXME
    }
};

template<>
struct extract_value<std::string>
{
    static inline std::optional<std::string> do_extract_from_string(std::string const& source)
    {
        return std::optional<std::string>(source);
    }

    static inline std::optional<std::string> do_extract_from_bool(value_bool const& source)
    {
        return source ? "true" : "false";
    }
};

template<typename T>
std::optional<T> param_cast(std::string const& source)
{
    return extract_value<T>::do_extract_from_string(source);
}

template<typename T>
std::optional<T> param_cast(value_bool const& source)
{
    return extract_value<T>::do_extract_from_bool(source);
}

} // end namespace detail

template<typename T>
struct value_extractor_visitor
{
    value_extractor_visitor(std::optional<T>& var)
        : var_(var)
    {}

    void operator()(std::string const& val) const { var_ = detail::param_cast<T>(val); }

    void operator()(value_bool const& val) const { var_ = detail::param_cast<T>(val); }

    template<typename T1>
    void operator()(T1 const& val) const
    {
        try
        {
            var_ = boost::lexical_cast<T>(val);
        }
        catch (boost::bad_lexical_cast const&)
        {
            std::ostringstream s;
            s << "Failed converting from " << typeid(T1).name() << " to " << typeid(T).name();
            throw std::runtime_error(s.str());
        }
    }

    std::optional<T>& var_;
};

namespace params_detail {

template<typename T>
struct converter
{
    using value_type = T;
    using return_type = std::optional<value_type>;
    static return_type
      extract(parameters const& params, std::string const& name, std::optional<T> const& default_opt_value)
    {
        std::optional<T> result(default_opt_value);
        parameters::const_iterator itr = params.find(name);
        if (itr != params.end())
        {
            util::apply_visitor(value_extractor_visitor<T>(result), itr->second);
        }
        return result;
    }
};

} // end namespace params_detail

template<typename T>
std::optional<T> parameters::get(std::string const& key) const
{
    return params_detail::converter<T>::extract(*this, key, std::nullopt);
}

template<typename T>
std::optional<T> parameters::get(std::string const& key, T const& default_opt_value) const
{
    return params_detail::converter<T>::extract(*this, key, std::optional<T>(default_opt_value));
}

} // namespace mapnik
