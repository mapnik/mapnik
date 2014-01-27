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

#include <mapnik/parse_path.hpp>
#include <mapnik/path_expression_grammar.hpp>

#include <mapnik/config.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/value.hpp>

// boost
#include <boost/variant.hpp>


// stl
#include <stdexcept>


namespace mapnik {

path_expression_ptr parse_path(std::string const& str)
{
    path_expression_grammar<std::string::const_iterator> g;
    return parse_path(str,g);
}

path_expression_ptr parse_path(std::string const& str,
                               path_expression_grammar<std::string::const_iterator> const& g)
{
    path_expression path;
    boost::spirit::standard_wide::space_type space;
    std::string::const_iterator itr = str.begin();
    std::string::const_iterator end = str.end();
    bool r = qi::phrase_parse(itr, end, g, space, path);
    if (r  && itr == end)
    {
        return std::make_shared<path_expression>(path); //path;
    }
    else
    {
        throw std::runtime_error("Failed to parse path expression: \"" + str + "\"");
    }
}

namespace path_processor_detail {
    struct path_visitor_ : boost::static_visitor<void>
    {
        path_visitor_ (std::string & filename, feature_impl const& f)
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
        feature_impl const& feature_;
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

    struct collect_ : boost::static_visitor<void>
    {
        collect_ (std::set<std::string> & cont)
            : cont_(cont) {}

        void operator() (std::string const& token) const
        {
            boost::ignore_unused_variable_warning(token);
        }

        void operator() (attribute const& attr) const
        {
            cont_.insert(attr.name());
        }

        std::set<std::string> & cont_;
    };
}

std::string path_processor::evaluate(path_expression const& path,feature_impl const& f)
{
    std::string out;
    path_processor_detail::path_visitor_ eval(out,f);
    for ( mapnik::path_component const& token : path)
        boost::apply_visitor(eval,token);
    return out;
}

std::string path_processor::to_string(path_expression const& path)
{
    std::string str;
    path_processor_detail::to_string_ visitor(str);
    for ( mapnik::path_component const& token : path)
        boost::apply_visitor(visitor,token);
    return str;
}

void path_processor::collect_attributes(path_expression const& path, std::set<std::string>& names)
{
    path_processor_detail::collect_ visitor(names);
    for ( mapnik::path_component const& token : path)
        boost::apply_visitor(visitor,token);
}


}
