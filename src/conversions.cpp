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
#include <mapnik/value_types.hpp>

#include <cstring>

#include <boost/spirit/include/qi.hpp>

#define BOOST_SPIRIT_AUTO(domain_, name, expr)                  \
    typedef boost::proto::result_of::                           \
    deep_copy<BOOST_TYPEOF(expr)>::type name##_expr_type;       \
    BOOST_SPIRIT_ASSERT_MATCH(                                  \
        boost::spirit::domain_::domain, name##_expr_type);      \
    BOOST_AUTO(name, boost::proto::deep_copy(expr));            \

//#define MAPNIK_KARMA_TO_STRING

#ifdef MAPNIK_KARMA_TO_STRING
#include <boost/version.hpp>
#if BOOST_VERSION < 104500
#error you must have >= boost 104500 to use karma for string output
#endif

#include <boost/spirit/include/karma.hpp>
#include <cmath> // log10
#include <boost/math/special_functions/trunc.hpp> // trunc to avoid needing C++11
#endif

namespace mapnik {

namespace util {

using namespace boost::spirit;

BOOST_SPIRIT_AUTO(qi, INTEGER, qi::int_)
#ifdef BIGINT
BOOST_SPIRIT_AUTO(qi, LONGLONG, qi::long_long)
#endif
BOOST_SPIRIT_AUTO(qi, FLOAT, qi::float_)
BOOST_SPIRIT_AUTO(qi, DOUBLE, qi::double_)

struct bool_symbols : qi::symbols<char,bool>
{
    bool_symbols()
    {
        add("true",true)
            ("false",false)
            ("yes",true)
            ("no",false)
            ("on",true)
            ("off",false)
            ("1",true)
            ("0",false);
    }
};

bool string2bool(const char * value, bool & result)
{
    using boost::spirit::qi::no_case;
    const char *iter = value;
    const char *end  = value + std::strlen(value);
    bool r = qi::phrase_parse(iter,end, no_case[bool_symbols()] ,ascii::space,result);
    return r && (iter == end);
}

bool string2bool(std::string const& value, bool & result)
{
    using boost::spirit::qi::no_case;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,no_case[bool_symbols()],ascii::space,result);
    return r && (str_beg == str_end);
}

bool string2int(const char * value, int & result)
{
    const char *iter = value;
    const char *end  = value + std::strlen(value);
    bool r = qi::phrase_parse(iter,end,INTEGER,ascii::space,result);
    return r && (iter == end);
}

bool string2int(std::string const& value, int & result)
{
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,INTEGER,ascii::space,result);
    return r && (str_beg == str_end);
}

#ifdef BIGINT
bool string2int(const char * value, mapnik::value_integer & result)
{
    const char *iter  = value;
    const char *end   = value + std::strlen(value);
    bool r = qi::phrase_parse(iter,end,LONGLONG,ascii::space,result);
    return r && (iter == end);
}

bool string2int(std::string const& value, mapnik::value_integer & result)
{
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,LONGLONG,ascii::space,result);
    return r && (str_beg == str_end);
}
#endif

bool string2double(std::string const& value, double & result)
{
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,DOUBLE,ascii::space,result);
    return r && (str_beg == str_end);
}

bool string2double(const char * value, double & result)
{
    const char *iter = value;
    const char *end  = value + std::strlen(value);
    bool r = qi::phrase_parse(iter,end,DOUBLE,ascii::space,result);
    return r && (iter == end);
}

bool string2float(std::string const& value, float & result)
{
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,FLOAT,ascii::space,result);
    return r && (str_beg == str_end);
}

bool string2float(const char * value, float & result)
{
    const char *iter  = value;
    const char *end   = value + std::strlen(value);
    bool r = qi::phrase_parse(iter,end,FLOAT,ascii::space,result);
    return r && (iter == end);
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

namespace detail {
  template <typename T>
  struct double_policy : boost::spirit::karma::real_policies<T>
  {
      typedef boost::spirit::karma::real_policies<T> base_type;

      static int floatfield(T n) {
        using namespace boost::spirit; // for traits

        if (traits::test_zero(n))
            return base_type::fmtflags::fixed;

        T abs_n = traits::get_absolute_value(n);
        return (abs_n >= 1e16 || abs_n < 1e-4)
          ? base_type::fmtflags::scientific : base_type::fmtflags::fixed;
      }

      static unsigned precision(T n) {
        if ( n == 0.0 ) return 0;
        using namespace boost::spirit; // for traits
        return static_cast<unsigned>(15 - boost::math::trunc(log10(traits::get_absolute_value(n))));
      }

      template <typename OutputIterator>
      static bool dot(OutputIterator& sink, T n, unsigned precision) {
        if (n == 0.0) return true; // avoid trailing zeroes
        return base_type::dot(sink, n, precision);
      }

      template <typename OutputIterator>
      static bool fraction_part (OutputIterator& sink, T n
        , unsigned precision_, unsigned precision)
      {
          // NOTE: copied from karma only to avoid trailing zeroes
          //       (maybe a bug ?)

          // allow for ADL to find the correct overload for floor and log10
          using namespace std;

          using namespace boost::spirit; // for traits
          using namespace boost::spirit::karma; // for char_inserter

          if ( traits::test_zero(n) ) return true; // this part added to karma

          // The following is equivalent to:
          //    generate(sink, right_align(precision, '0')[ulong], n);
          // but it's spelled out to avoid inter-modular dependencies.

          typename boost::remove_const<T>::type digits =
              (traits::test_zero(n) ? 0 : floor(log10(n))) + 1;
          bool r = true;
          for (/**/; r && digits < precision_; digits = digits + 1)
              r = char_inserter<>::call(sink, '0');
          if (precision && r)
              r = int_inserter<10>::call(sink, n);
          return r;
      }

      template <typename CharEncoding, typename Tag, typename OutputIterator>
      static bool exponent (OutputIterator& sink, long n)
      {
          // NOTE: copied from karma to force sign in exponent
          const bool force_sign = true;

          using namespace boost::spirit; // for traits
          using namespace boost::spirit::karma; // for char_inserter, sign_inserter

          unsigned long abs_n = traits::get_absolute_value(n);
          bool r = char_inserter<CharEncoding, Tag>::call(sink, 'e') &&
                   sign_inserter::call(sink, traits::test_zero(n)
                      , traits::test_negative(n), force_sign);

          // the C99 Standard requires at least two digits in the exponent
          if (r && abs_n < 10)
              r = char_inserter<CharEncoding, Tag>::call(sink, '0');
          return r && int_inserter<10>::call(sink, abs_n);
      }

  };
}

bool to_string(std::string & str, double value)
{
    namespace karma = boost::spirit::karma;
    typedef karma::real_generator<double, detail::double_policy<double> > double_type;
    std::back_insert_iterator<std::string> sink(str);
    return karma::generate(sink, double_type(), value);
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

bool to_string(std::string & s, double val)
{
    double abs_n = std::fabs(val);
    std::string format;
    if (abs_n >= 1e16 || abs_n < 1e-4)
    {
        format = "%e";
    }
    else
    {
        format = "%g";
    }
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

#endif

} // end namespace util

}
