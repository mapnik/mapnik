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

// mapnik
#include <mapnik/util/conversions.hpp>
#include <mapnik/value_types.hpp>

#include <cstring>
#include <algorithm>

// karma is used by default
#define MAPNIK_KARMA_TO_STRING

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include <boost/spirit/include/qi.hpp>
#ifdef MAPNIK_KARMA_TO_STRING
  #include <boost/spirit/include/karma.hpp>
#endif
#pragma GCC diagnostic pop

#if _MSC_VER
#define snprintf _snprintf
#endif

namespace mapnik {

namespace util {

using namespace boost::spirit;

auto INTEGER = qi::int_type();
#ifdef BIGINT
auto LONGLONG = qi::long_long_type();
#endif
auto FLOAT = qi::float_type();
auto DOUBLE = qi::double_type();

bool string2bool(std::string const& value, bool & result)
{
    if (value.empty() || value.size() > 5) {
        return false;
    } else if (value == "true") {
        return result = true;
    } else if (value == "false") {
        result = false;
        return true;
    }
    std::string val(value);
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    if (val == "true" || val == "yes" || val == "1" || val == "on") {
        return result = true;
    } else if (val == "false" || val == "no" || val == "0" || val == "off") {
        result = false;
        return true;
    }
    return false;
}

bool string2bool(const char * iter, const char * end, bool & result)
{
    std::string val(iter,end);
    return string2bool(val,result);
}

bool string2int(const char * iter, const char * end, int & result)
{
    ascii::space_type space;
    bool r = qi::phrase_parse(iter,end,INTEGER,space,result);
    return r && (iter == end);
}

bool string2int(std::string const& value, int & result)
{
    ascii::space_type space;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,INTEGER,space,result);
    return r && (str_beg == str_end);
}

#ifdef BIGINT
bool string2int(const char * iter, const char * end, mapnik::value_integer & result)
{
    ascii::space_type space;
    bool r = qi::phrase_parse(iter,end,LONGLONG,space,result);
    return r && (iter == end);
}

bool string2int(std::string const& value, mapnik::value_integer & result)
{
    ascii::space_type space;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,LONGLONG,space,result);
    return r && (str_beg == str_end);
}
#endif

bool string2double(std::string const& value, double & result)
{
    ascii::space_type space;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,DOUBLE,space,result);
    return r && (str_beg == str_end);
}

bool string2double(const char * iter, const char * end, double & result)
{
    ascii::space_type space;
    bool r = qi::phrase_parse(iter,end,DOUBLE,space,result);
    return r && (iter == end);
}

bool string2float(std::string const& value, float & result)
{
    ascii::space_type space;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,FLOAT,space,result);
    return r && (str_beg == str_end);
}

bool string2float(const char * iter, const char * end, float & result)
{
    ascii::space_type space;
    bool r = qi::phrase_parse(iter,end,FLOAT,space,result);
    return r && (iter == end);
}

// double conversion - here we use sprintf over karma to work
// around https://github.com/mapnik/mapnik/issues/1741
bool to_string(std::string & s, double val)
{
    s.resize(s.capacity());
    while (true)
    {
        size_t n2 = static_cast<size_t>(snprintf(&s[0], s.size()+1, "%g", val));
        if (n2 <= s.size())
        {
            s.resize(n2);
            break;
        }
        s.resize(n2);
    }
    return true;
}

#ifdef MAPNIK_KARMA_TO_STRING

bool to_string(std::string & str, int value)
{
  namespace karma = boost::spirit::karma;
  std::back_insert_iterator<std::string> sink(str);
  return karma::generate(sink, value);
}

#ifdef BIGINT
bool to_string(std::string & str, mapnik::value_integer value)
{
  namespace karma = boost::spirit::karma;
  std::back_insert_iterator<std::string> sink(str);
  return karma::generate(sink, value);
}
#endif

bool to_string(std::string & str, unsigned value)
{
  namespace karma = boost::spirit::karma;
  std::back_insert_iterator<std::string> sink(str);
  return karma::generate(sink, value);
}

bool to_string(std::string & str, bool value)
{
  namespace karma = boost::spirit::karma;
  std::back_insert_iterator<std::string> sink(str);
  return karma::generate(sink, value);
}

#else

bool to_string(std::string & s, int val)
{
    s.resize(s.capacity());
    while (true)
    {
        size_t n2 = static_cast<size_t>(snprintf(&s[0], s.size()+1, "%d", val));
        if (n2 <= s.size())
        {
            s.resize(n2);
            break;
        }
        s.resize(n2);
    }
    return true;
}

#ifdef BIGINT
bool to_string(std::string & s, mapnik::value_integer val)
{
    s.resize(s.capacity());
    while (true)
    {
        size_t n2 = static_cast<size_t>(snprintf(&s[0], s.size()+1, "%lld", val));
        if (n2 <= s.size())
        {
            s.resize(n2);
            break;
        }
        s.resize(n2);
    }
    return true;
}
#endif

bool to_string(std::string & s, unsigned val)
{
    s.resize(s.capacity());
    while (true)
    {
        size_t n2 = static_cast<size_t>(snprintf(&s[0], s.size()+1, "%u", val));
        if (n2 <= s.size())
        {
            s.resize(n2);
            break;
        }
        s.resize(n2);
    }
    return true;
}

bool to_string(std::string & s, bool val)
{
  if (val) s = "true";
  else s = "false";
  return true;
}

#endif

} // end namespace util

}
