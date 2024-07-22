/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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
#include <boost/spirit/home/support/attributes.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/fusion/include/adapt_adt.hpp>
#include <boost/spirit/include/support_adapt_adt_attributes.hpp>

namespace mapnik {

struct kv_store
{
    using value_type = mapnik::feature_impl::value_type;
    using iterator_type = mapnik::feature_kv_iterator2;
    kv_store(mapnik::feature_impl const& f)
        : start_(mapnik::value_not_null(), f.begin(), f.end())
        , end_(mapnik::value_not_null(), f.end(), f.end())
    {}
    iterator_type start_;
    iterator_type end_;
};

} // namespace mapnik

namespace boost {
namespace spirit {
namespace traits {

template<>
struct is_container<mapnik::kv_store const> : mpl::false_
{};

template<>
struct container_iterator<mapnik::kv_store const>
{
    using type = mapnik::kv_store::iterator_type;
};

template<>
struct begin_container<mapnik::kv_store const>
{
    static mapnik::kv_store::iterator_type call(mapnik::kv_store const& kv) { return kv.start_; }
};

template<>
struct end_container<mapnik::kv_store const>
{
    static mapnik::kv_store::iterator_type call(mapnik::kv_store const& kv) { return kv.end_; }
};

} // namespace traits
} // namespace spirit
} // namespace boost

BOOST_FUSION_ADAPT_ADT(mapnik::feature_impl,
                       (mapnik::value_integer, mapnik::value_integer, obj.id(),
                        /**/)(mapnik::geometry::geometry<double> const&,
                              mapnik::geometry::geometry<double> const&,
                              obj.get_geometry(),
                              /**/)(mapnik::kv_store const, mapnik::kv_store const, mapnik::kv_store(obj), /**/))

namespace mapnik {
namespace json {
namespace detail {
template<typename T>
#if BOOST_VERSION >= 107000
struct attribute_type
{
    using type = T();
};
#else
struct attribute_type
{
    using type = T const&();
};
#endif
} // namespace detail

namespace karma = boost::spirit::karma;

template<typename OutputIterator, typename FeatureType>
struct feature_generator_grammar : karma::grammar<OutputIterator, typename detail::attribute_type<FeatureType>::type>
{
    feature_generator_grammar();
    karma::rule<OutputIterator, typename detail::attribute_type<FeatureType>::type> feature;
    geometry_generator_grammar<OutputIterator, mapnik::geometry::geometry<double>> geometry;
    properties_generator_grammar<OutputIterator, mapnik::kv_store> properties;
};

} // namespace json
} // namespace mapnik

#endif // MAPNIK_JSON_FEATURE_GENERATOR_GRAMMAR_HPP
