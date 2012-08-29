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

#ifndef MAPNIK_PARSE_PATH_HPP
#define MAPNIK_PARSE_PATH_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/value.hpp>
#include <mapnik/path_expression_grammar.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>
#include <boost/foreach.hpp>

// stl
#include <string>
#include <vector>

namespace mapnik {

typedef boost::shared_ptr<path_expression> path_expression_ptr;

MAPNIK_DECL path_expression_ptr parse_path(std::string const & str);
MAPNIK_DECL path_expression_ptr parse_path(std::string const & str,
                                           path_expression_grammar<std::string::const_iterator> const& g);

template <typename T>
struct path_processor
{
    typedef T feature_type;
    struct path_visitor_ : boost::static_visitor<void>
    {
        path_visitor_ (std::string & filename, feature_type const& f)
            : filename_(filename),
              feature_(f) {}

        void operator() (std::string const& token) const
        {
            filename_ += token;
        }

        void operator() (attribute const& attr) const
        {
            // convert mapnik::value to std::string
            value const& val = feature_.get(attr.name());
            filename_ += val.to_string();
        }

        std::string & filename_;
        feature_type const& feature_;
    };

    struct to_string_ : boost::static_visitor<void>
    {
        to_string_ (std::string & str)
            : str_(str) {}

        void operator() (std::string const& token) const
        {
            str_ += token;
        }

        void operator() (attribute const& attr) const
        {
            str_ += "[";
            str_ += attr.name();
            str_ += "]";
        }

        std::string & str_;
    };

    template <typename T1>
    struct collect_ : boost::static_visitor<void>
    {
        collect_ (T1 & cont)
            : cont_(cont) {}

        void operator() (std::string const& token) const
        {
            boost::ignore_unused_variable_warning(token);
        }

        void operator() (attribute const& attr) const
        {
            cont_.insert(attr.name());
        }

        T1 & cont_;
    };

    static std::string evaluate(path_expression const& path,feature_type const& f)
    {
        std::string out;
        path_visitor_ eval(out,f);
        BOOST_FOREACH( mapnik::path_component const& token, path)
            boost::apply_visitor(eval,token);
        return out;
    }

    static std::string to_string(path_expression const& path)
    {
        std::string str;
        to_string_ visitor(str);
        BOOST_FOREACH( mapnik::path_component const& token, path)
            boost::apply_visitor(visitor,token);
        return str;
    }

    template <typename T2>
    static void collect_attributes(path_expression const& path, T2 & names)
    {
        typedef T2 cont_type;
        collect_<cont_type> visitor(names);
        BOOST_FOREACH( mapnik::path_component const& token, path)
            boost::apply_visitor(visitor,token);
    }
};

typedef mapnik::path_processor<Feature> path_processor_type;

}

#endif // MAPNIK_PARSE_PATH_HPP
