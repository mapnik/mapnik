/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_SYMBOLIZER_GRAMMAR_HPP
#define MAPNIK_SYMBOLIZER_GRAMMAR_HPP

#include <mapnik/config.hpp>

// boost
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

// mapnik
#include <mapnik/util/variant.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/symbolizer_utils.hpp>
#include <mapnik/json/error_handler.hpp>
#include <mapnik/json/generic_json.hpp>

namespace mapnik { namespace json {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;
namespace standard_wide =  boost::spirit::standard_wide;
using standard_wide::space_type;

template <typename Symbolizer>
struct json_value_visitor
{
    json_value_visitor(Symbolizer & sym, mapnik::keys key)
        : sym_(sym), key_(key) {}

    void operator() (value_bool val) const
    {
        put<value_bool>(sym_, key_, val);
    }

    void operator() (value_integer val) const
    {
        put<value_integer>(sym_, key_, val);
    }

    void operator() (value_double val) const
    {
        put<value_double>(sym_, key_, val);
    }

    void operator() (std::string const& val) const
    {
        set_property(sym_, key_, val);
    }

    template <typename T>
    void operator() (T const& val) const
    {
        std::cerr << std::get<0>(get_meta(key_)) << ":" << val <<  std::endl;
        //put<T>(sym_, key_, val);
    }

    Symbolizer & sym_;
    keys key_;
};

template <typename T>
struct put_property_visitor
{
    using value_type = T;

    put_property_visitor(mapnik::keys key, value_type const& val)
        : key_(key), val_(val) {}

    template <typename Symbolizer>
    void operator() (Symbolizer & sym) const
    {
        mapnik::util::apply_visitor(json_value_visitor<Symbolizer>(sym, key_), val_);
    }

    keys key_;
    value_type const& val_;
};

struct put_property
{
    using result_type = void;
    template <typename T0,typename T1, typename T2>
    result_type operator() (T0 & sym, T1 const& name, T2 const& val) const
    {
        try
        {
            mapnik::util::apply_visitor(put_property_visitor<T2>(get_key(name),val), sym);
        }
        catch (std::runtime_error const& err)
        {
            std::cerr <<  err.what() << std::endl;
        }
    }
};

template <typename Iterator, typename ErrorHandler = error_handler<Iterator>>
struct symbolizer_grammar : qi::grammar<Iterator, space_type, symbolizer()>
{
    using json_value_type = util::variant<value_null,value_bool,value_integer,value_double, std::string>;
    symbolizer_grammar()
        : symbolizer_grammar::base_type(sym, "symbolizer"),
          json_()
    {
        qi::lit_type lit;
        qi::double_type double_;
        qi::int_type int_;
        qi::no_skip_type no_skip;
        qi::_val_type _val;
        qi::_a_type _a;
        qi::_r1_type _r1;
        qi::_1_type _1;
        standard_wide::char_type char_;
        using phoenix::construct;

        // generic json types
        json_.value =  json_.object | json_.array | json_.string_
            | json_.number
            ;

        json_.pairs = json_.key_value % lit(',')
            ;

        json_.key_value = (json_.string_ >> lit(':') >> json_.value)
            ;

        json_.object = lit('{')
            >> *json_.pairs
            >> lit('}')
            ;

        json_.array = lit('[')
            >> json_.value >> *(lit(',') >> json_.value)
            >> lit(']')
        ;

        json_.number %= json_.strict_double
            | json_.int__
            | lit("true") [_val = true]
            | lit ("false") [_val = false]
            | lit("null")[_val = construct<value_null>()]
            ;
        json_.unesc_char.add
            ("\\\"", '\"') // quotation mark
            ("\\\\", '\\') // reverse solidus
            ("\\/", '/')   // solidus
            ("\\b", '\b')  // backspace
            ("\\f", '\f')  // formfeed
            ("\\n", '\n')  // newline
            ("\\r", '\r')  // carrige return
            ("\\t", '\t')  // tab
        ;

        json_.string_ %= lit('"') >> no_skip[*(json_.unesc_char | "\\u" >> json_.hex4 | (char_ - lit('"')))] >> lit('"')
            ;

        sym = lit('{')
            >> lit("\"type\"") >> lit(':')
            >> (lit("\"PointSymbolizer\"")[_val = construct<point_symbolizer>()]
                |
                lit("\"LineSymbolizer\"")[_val = construct<line_symbolizer>()]
                |
                lit("\"PolygonSymbolizer\"")[_val = construct<polygon_symbolizer>()]
                )
            >> lit(',')
            >> lit("\"properties\"") >> lit(':')
            >> ((lit('{') >> *property(_val) >> lit('}')) | lit("null"))
            >> lit('}')
            ;

        property = (json_.string_ [_a = _1] >> lit(':') >> property_value [put_property_(_r1,_a,_1)]) % lit(',')
            ;

        property_value %= json_.number | json_.string_  ;


    }

    // generic JSON
    generic_json<Iterator> json_;
    // symbolizer
    qi::rule<Iterator, space_type, mapnik::symbolizer()> sym;
    qi::rule<Iterator,qi::locals<std::string>, void(mapnik::symbolizer&),space_type> property;
    qi::rule<Iterator, space_type, json_value_type()> property_value;

    phoenix::function<put_property> put_property_;
    // error
    //qi::on_error<qi::fail>(sym, error_handler(_1, _2, _3, _4));
};


}}

#endif // MAPNIK_SYMBOLIZER_GRAMMAR_HPP
