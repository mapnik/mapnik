/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2009 Artem Pavlenko
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

//$Id$

#ifndef MAPNIK_PATH_EXPRESSIONS_GRAMMAR_HPP
#define MAPNIK_PATH_EXPRESSIONS_GRAMMAR_HPP

// mapnik
#include <mapnik/unicode.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/feature.hpp>

// boost
#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/concept_check.hpp>
#include <boost/foreach.hpp>
//spirit2
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_action.hpp>
//fusion
#include <boost/fusion/include/adapt_struct.hpp>
//phoenix
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
// stl
#include <string>
#include <vector>

namespace mapnik
{

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace standard_wide =  boost::spirit::standard_wide;

//
using standard_wide::space_type;
using standard_wide::space;

typedef boost::variant<std::string, attribute> path_component;
typedef std::vector<path_component> path_expression;
typedef boost::shared_ptr<path_expression> path_expression_ptr;

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
            filename_ += attr.value<mapnik::value,feature_type>(feature_).to_string();
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

typedef mapnik::path_processor<mapnik::Feature> path_processor_type;

template <typename Iterator>
struct path_expression_grammar : qi::grammar<Iterator, std::vector<path_component>(), space_type>
{    
    path_expression_grammar()
        : path_expression_grammar::base_type(expr)
    {
        using boost::phoenix::construct;
        using standard_wide::char_;
        using qi::_1;
        using qi::_val;
        using qi::lit;
        using qi::lexeme;
        using phoenix::push_back;
        
        expr =
            * (
                str [ push_back(_val, _1)]
                |
                ( '[' >> attr [ push_back(_val, construct<attribute>( _1 )) ] >> ']')
                )
            ;
        
        attr %= +(char_ - ']');
        str  %= lexeme[+(char_ -'[')];
    }
    
    qi::rule<Iterator, std::vector<path_component>() , space_type> expr;
    qi::rule<Iterator, std::string() , space_type> attr;
    qi::rule<Iterator, std::string() > str;
};


inline path_expression_ptr parse_path(std::string const & str)
{
    path_expression_ptr path(new path_expression) ;
    path_expression_grammar<std::string::const_iterator> g;

    std::string::const_iterator itr = str.begin();
    std::string::const_iterator end = str.end();
    bool r = qi::phrase_parse(itr, end, g, space, *path);
    if (r  && itr == end)
    {
        return path;
    }
    else
    {
        throw std::runtime_error("Failed to parse path expression");
    }
}


}

#endif  // MAPNIK_PATH_EXPRESSIONS_GRAMMAR_HPP

