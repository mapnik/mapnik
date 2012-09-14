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

#ifndef MAPNIK_JSON_FEATURE_GENERATOR_GRAMMAR_HPP
#define MAPNIK_JSON_FEATURE_GENERATOR_GRAMMAR_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/json/geometry_generator_grammar.hpp>
#include <mapnik/feature_kv_iterator.hpp>

// boost
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/cons.hpp>

namespace boost { namespace spirit { namespace traits {

template <>
struct is_container<mapnik::Feature const> : mpl::false_ {} ;

template <>
struct container_iterator<mapnik::Feature const>
{
    typedef mapnik::feature_kv_iterator2 type;
};

template <>
struct begin_container<mapnik::Feature const>
{
    static mapnik::feature_kv_iterator2
    call (mapnik::Feature const& f)
    {
        return mapnik::feature_kv_iterator2(mapnik::value_not_null(),f.begin(),f.end());
    }
};

template <>
struct end_container<mapnik::Feature const>
{
    static mapnik::feature_kv_iterator2
    call (mapnik::Feature const& f)
    {
        return mapnik::feature_kv_iterator2(mapnik::value_not_null(),f.end(),f.end());
    }
};

template <>
struct transform_attribute<const boost::fusion::cons<mapnik::feature_impl const&, boost::fusion::nil>,
                           mapnik::geometry_container const& ,karma::domain>
{
    typedef mapnik::geometry_container const& type;
    static type pre(const boost::fusion::cons<mapnik::feature_impl const&, boost::fusion::nil>& f)
    {
        return boost::fusion::at<mpl::int_<0> >(f).paths();
    }
};

}}}

namespace mapnik { namespace json {

namespace karma = boost::spirit::karma;
namespace phoenix = boost::phoenix;

struct get_id
{
    template <typename T>
    struct result { typedef int type; };

    int operator() (mapnik::Feature const& f) const
    {
        return f.id();
    }
};

struct make_properties_range
{
    typedef boost::iterator_range<mapnik::feature_kv_iterator> properties_range_type;

    template <typename T>
    struct result { typedef properties_range_type type; };

    properties_range_type operator() (mapnik::Feature const& f) const
    {
        return boost::make_iterator_range(f.begin(),f.end());
    }
};


struct utf8
{
    template <typename T>
    struct result { typedef std::string type; };

    std::string operator() (UnicodeString const& ustr) const
    {
        std::string result;
        to_utf8(ustr,result);
        return result;
    }
};

struct value_base
{
    template <typename T>
    struct result { typedef mapnik::value_base const& type; };

    mapnik::value_base const& operator() (mapnik::value const& val) const
    {
        return val.base();
    }
};

template <typename OutputIterator>
struct escaped_string
    : karma::grammar<OutputIterator, std::string(char const*)>
{
    escaped_string()
        : escaped_string::base_type(esc_str)
    {
        using boost::spirit::karma::maxwidth;
        using boost::spirit::karma::right_align;

        esc_char.add
            ('"', "\\\"")
            ('\\', "\\\\")
            ('\b', "\\b")
            ('\f', "\\f")
            ('\n', "\\n")
            ('\r', "\\r")
            ('\t', "\\t")
            ;

        esc_str =   karma::lit(karma::_r1)
            << *(esc_char
                 | karma::print
                 | "\\u" << right_align(4,karma::lit('0'))[karma::hex])
            <<  karma::lit(karma::_r1)
            ;
    }

    karma::rule<OutputIterator, std::string(char const*)> esc_str;
    karma::symbols<char, char const*> esc_char;

};

template <typename OutputIterator>
struct feature_generator_grammar:
        karma::grammar<OutputIterator, mapnik::Feature const&()>
{
    typedef boost::tuple<std::string, mapnik::value> pair_type;
    typedef make_properties_range::properties_range_type range_type;

    feature_generator_grammar()
        : feature_generator_grammar::base_type(feature)
        , quote_("\"")

    {
        using boost::spirit::karma::lit;
        using boost::spirit::karma::uint_;
        using boost::spirit::karma::bool_;
        using boost::spirit::karma::int_;
        using boost::spirit::karma::double_;
        using boost::spirit::karma::_val;
        using boost::spirit::karma::_1;
        using boost::spirit::karma::_r1;
        using boost::spirit::karma::string;
        using boost::spirit::karma::eps;

        feature = lit("{\"type\":\"Feature\",\"id\":")
            << uint_[_1 = id_(_val)]
            << lit(",\"geometry\":") << geometry
            << lit(",\"properties\":") << properties
            << lit('}')
            ;

        properties = lit('{')
            << -(pair % lit(','))
            << lit('}')
            ;

        pair = lit('"')
            << string[_1 = phoenix::at_c<0>(_val)] << lit('"')
            << lit(':')
            << value(phoenix::at_c<1>(_val))
            ;

        value = (value_null_| bool_ | int_| double_ | ustring)[_1 = value_base_(_r1)]
            ;

        value_null_ = string[_1 = "null"]
            ;

        ustring = escaped_string_(quote_.c_str())[_1 = utf8_(_val)]
            ;
    }

    // rules
    karma::rule<OutputIterator, mapnik::Feature const&()> feature;
    multi_geometry_generator_grammar<OutputIterator> geometry;
    escaped_string<OutputIterator> escaped_string_;
    karma::rule<OutputIterator, mapnik::Feature const&()> properties;
    karma::rule<OutputIterator, pair_type()> pair;
    karma::rule<OutputIterator, void(mapnik::value const&)> value;
    karma::rule<OutputIterator, mapnik::value_null()> value_null_;
    karma::rule<OutputIterator, UnicodeString()> ustring;
    // phoenix functions
    phoenix::function<get_id> id_;
    phoenix::function<value_base> value_base_;
    phoenix::function<utf8> utf8_;
    std::string quote_;
};

}}

#endif // MAPNIK_JSON_FEATURE_GENERATOR_GRAMMAR_HPP
