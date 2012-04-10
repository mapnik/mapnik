/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

// mapnik
#include <mapnik/util/conversions.hpp>

// boost
#include <boost/spirit/include/qi.hpp>

#define BOOST_SPIRIT_AUTO(domain_, name, expr)                  \
    typedef boost::proto::result_of::                           \
    deep_copy<BOOST_TYPEOF(expr)>::type name##_expr_type;       \
    BOOST_SPIRIT_ASSERT_MATCH(                                  \
        boost::spirit::domain_::domain, name##_expr_type);      \
    BOOST_AUTO(name, boost::proto::deep_copy(expr));            \


namespace mapnik { namespace util {

using namespace boost::spirit;

BOOST_SPIRIT_AUTO(qi, INTEGER, qi::int_)
BOOST_SPIRIT_AUTO(qi, FLOAT, qi::float_)
BOOST_SPIRIT_AUTO(qi, DOUBLE, qi::double_)

bool string2int(const char * value, int & result)
{
    size_t length = strlen(value);
    if (length < 1 || value == NULL)
        return false;
    const char *iter  = value;
    const char *end   = value + length;
    bool r = qi::phrase_parse(iter,end,INTEGER,ascii::space,result);
    return r && (iter == end);
}

bool string2int(std::string const& value, int & result)
{
    if (value.empty())
        return false;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,INTEGER,ascii::space,result);
    return r && (str_beg == str_end);
}

bool string2double(std::string const& value, double & result)
{
    if (value.empty())
        return false;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,DOUBLE,ascii::space,result);
    return r && (str_beg == str_end);
}

bool string2double(const char * value, double & result)
{
    size_t length = strlen(value);
    if (length < 1 || value == NULL)
        return false;
    const char *iter  = value;
    const char *end   = value + length;
    bool r = qi::phrase_parse(iter,end,DOUBLE,ascii::space,result);
    return r && (iter == end);
}

bool string2float(std::string const& value, float & result)
{
    if (value.empty())
        return false;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,FLOAT,ascii::space,result);
    return r && (str_beg == str_end);
}

bool string2float(const char * value, float & result)
{
    size_t length = strlen(value);
    if (length < 1 || value == NULL)
        return false;
    const char *iter  = value;
    const char *end   = value + length;
    bool r = qi::phrase_parse(iter,end,FLOAT,ascii::space,result);
    return r && (iter == end);
}

}
}
