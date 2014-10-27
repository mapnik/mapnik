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
#include <mapnik/value_types.hpp>
#include <mapnik/value.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry_container.hpp>
#include <mapnik/json/geometry_generator_grammar.hpp>
#include <mapnik/feature_kv_iterator.hpp>

// boost
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/cons.hpp>

namespace boost { namespace spirit { namespace traits {

template <>
struct is_container<mapnik::feature_impl const> : mpl::false_ {} ;

template <>
struct container_iterator<mapnik::feature_impl const>
{
    using type = mapnik::feature_kv_iterator2;
};

template <>
struct begin_container<mapnik::feature_impl const>
{
    static mapnik::feature_kv_iterator2
    call (mapnik::feature_impl const& f)
    {
        return mapnik::feature_kv_iterator2(mapnik::value_not_null(),f.begin(),f.end());
    }
};

template <>
struct end_container<mapnik::feature_impl const>
{
    static mapnik::feature_kv_iterator2
    call (mapnik::feature_impl const& f)
    {
        return mapnik::feature_kv_iterator2(mapnik::value_not_null(),f.end(),f.end());
    }
};

template <>
struct transform_attribute<const boost::fusion::cons<mapnik::feature_impl const&, boost::fusion::nil>,
                           mapnik::geometry_container const& ,karma::domain>
{
    using type = mapnik::geometry_container const&;
    static type pre(const boost::fusion::cons<mapnik::feature_impl const&, boost::fusion::nil>& f)
    {
        return boost::fusion::at<mpl::int_<0> >(f).paths();
    }
};

}}}

namespace mapnik { namespace json {

namespace karma = boost::spirit::karma;

struct get_id
{
    template <typename T>
    struct result { using type =  int; };

    int operator() (mapnik::feature_impl const& f) const
    {
        return f.id();
    }
};

struct make_properties_range
{
    using properties_range_type = boost::iterator_range<mapnik::feature_kv_iterator>;

    template <typename T>
    struct result { using type = properties_range_type; };

    properties_range_type operator() (mapnik::feature_impl const& f) const
    {
        return boost::make_iterator_range(f.begin(),f.end());
    }
};

struct utf8
{
    template <typename T>
    struct result { using type = std::string; };

    std::string operator() (mapnik::value_unicode_string const& ustr) const
    {
        std::string result;
        to_utf8(ustr,result);
        return result;
    }
};

template <typename OutputIterator>
struct escaped_string
    : karma::grammar<OutputIterator, std::string(char const*)>
{
    escaped_string();
    karma::rule<OutputIterator, std::string(char const*)> esc_str;
    karma::symbols<char, char const*> esc_char;
};

struct extract_string
{
    using result_type = std::tuple<std::string,bool>;

    result_type operator() (mapnik::value const& val) const
    {
        bool need_quotes = val.is<value_unicode_string>();
        return std::make_tuple(val.to_string(), need_quotes);
    }
};

template <typename OutputIterator>
struct feature_generator_grammar:
        karma::grammar<OutputIterator, mapnik::feature_impl const&()>
{
    using pair_type = std::tuple<std::string, mapnik::value>;
    using range_type = make_properties_range::properties_range_type;

    feature_generator_grammar();
    karma::rule<OutputIterator, mapnik::feature_impl const&()> feature;
    multi_geometry_generator_grammar<OutputIterator, mapnik::geometry_container> geometry;
    escaped_string<OutputIterator> escaped_string_;
    karma::rule<OutputIterator, mapnik::feature_impl const&()> properties;
    karma::rule<OutputIterator, pair_type()> pair;
    karma::rule<OutputIterator, std::tuple<std::string,bool>()> value;
    karma::rule<OutputIterator, mapnik::value_null()> value_null_;
    karma::rule<OutputIterator, mapnik::value_unicode_string()> ustring;
    typename karma::int_generator<mapnik::value_integer,10, false> int__;
    boost::phoenix::function<get_id> id_;
    boost::phoenix::function<extract_string> extract_string_;
    boost::phoenix::function<utf8> utf8_;
    std::string quote_;
};

}}

#endif // MAPNIK_JSON_FEATURE_GENERATOR_GRAMMAR_HPP
