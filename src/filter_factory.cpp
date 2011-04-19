/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

#include <mapnik/filter_factory.hpp>
#include <mapnik/expression_grammar.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/unicode.hpp>

// boost
#include <boost/algorithm/string.hpp>

namespace mapnik
{

class filter_factory
{
public:
    static expression_ptr compile(std::string const& str,transcoder const& tr)
    {
        expression_ptr expr(new expr_node(true));
       
        std::string::const_iterator itr = str.begin();
        std::string::const_iterator end = str.end();
        mapnik::expression_grammar<std::string::const_iterator> g(tr);
       
        bool r = boost::spirit::qi::phrase_parse(itr,end,g, boost::spirit::standard_wide::space,*expr);
        if (r  && itr==end)
        {
            return expr;
        }
        else
        {
            // Backward compatiblity for version pre 2.0.0
            // To be removed in 2.1.0...
            if (!boost::algorithm::icontains(str,"'") 
                && !boost::algorithm::icontains(str,"[")
                && !boost::algorithm::icontains(str,"]")
                && !boost::algorithm::icontains(str," ")
                && !boost::algorithm::icontains(str,"\""))
            {

                expression_ptr expr1(new expr_node(true));
                mapnik::expression_grammar<std::string::const_iterator> g1(tr);
                std::string str1("[" + str + "]");
                std::string::const_iterator itr1 = str1.begin();
                std::string::const_iterator end1 = str1.end();
                bool r1 = boost::spirit::qi::phrase_parse(itr1,end1,g1, boost::spirit::standard_wide::space,*expr1);
                if (r1  && itr1==end1)
                {
                    std::clog << "Deprecation Warning: symbolizer value now an expression, please wrap properly in brackets like \"" + str1 + "\"\n";
                    return expr1;
                }
                else
                {
                    throw config_error( "Failed to parse expression: \"" + str + "\"" );
                }
            }
            else
            {
                throw config_error( "Failed to parse expression: \"" + str + "\"" );           
            }
        }
    }
};

expression_ptr parse_expression (std::string const& wkt,std::string const& encoding)
{
    transcoder tr(encoding);
    return filter_factory::compile(wkt,tr);
}

expression_ptr parse_expression (std::string const& wkt)
{
    return parse_expression(wkt,"utf8");
}
}
