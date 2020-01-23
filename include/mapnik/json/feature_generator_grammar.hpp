/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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
#include <mapnik/feature.hpp>

#include <mapnik/json/geometry_generator_grammar.hpp>
#include <mapnik/json/properties_generator_grammar.hpp>

// boost
#include <boost/spirit/include/karma.hpp>

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

#if BOOST_VERSION >= 106900
template <>
struct transform_attribute<const mapnik::feature_impl, const mapnik::feature_impl &, boost::spirit::karma::domain, void>
    : detail::transform_attribute_base<mapnik::feature_impl const, mapnik::feature_impl const&,  boost::spirit::karma::domain>
{};

template <>
struct transform_attribute<const boost::fusion::cons<const mapnik::feature_impl &, boost::fusion::nil_>, const mapnik::feature_impl &,
      boost::spirit::karma::domain, void>
    : detail::transform_attribute_base<const boost::fusion::cons<const mapnik::feature_impl &, boost::fusion::nil_>, mapnik::feature_impl const&,  boost::spirit::karma::domain>
{};
#endif
}}}

namespace mapnik { namespace json {

namespace karma = boost::spirit::karma;

template <typename T>
struct get_id
{
    using feature_type = T;
    using result_type = mapnik::value_integer;
    result_type operator() (feature_type const& f) const
    {
        return f.id();
    }
};

struct extract_geometry
{
    using result_type = mapnik::geometry::geometry<double> const&;
    template <typename T>
    result_type operator() (T const& f) const
    {
        return f.get_geometry();
    }
};

template <typename OutputIterator, typename FeatureType>
struct feature_generator_grammar :
        karma::grammar<OutputIterator, FeatureType const&()>
{
    feature_generator_grammar();
    karma::rule<OutputIterator, FeatureType const&()> feature;
    geometry_generator_grammar<OutputIterator, mapnik::geometry::geometry<double> > geometry;
    properties_generator_grammar<OutputIterator, FeatureType> properties;
    boost::phoenix::function<get_id<FeatureType> > id_;
    boost::phoenix::function<extract_geometry> geom_;
};

}}

#endif // MAPNIK_JSON_FEATURE_GENERATOR_GRAMMAR_HPP
