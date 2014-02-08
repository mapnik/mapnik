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

#ifndef MAPNIK_FEATURE_COLLECTION_PARSER_HPP
#define MAPNIK_FEATURE_COLLECTION_PARSER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/noncopyable.hpp>
#include <mapnik/unicode.hpp>

// stl
#include <vector>

namespace mapnik { namespace json {

template <typename Iterator, typename FeatureType> struct feature_collection_grammar;
template <typename Iterator> struct generic_json;

template <typename Iterator>
class MAPNIK_DECL feature_collection_parser : private mapnik::noncopyable
{
    typedef Iterator iterator_type;
    typedef mapnik::feature_impl feature_type;
public:
    feature_collection_parser(generic_json<Iterator> & json, mapnik::context_ptr const& ctx, mapnik::transcoder const& tr);
    ~feature_collection_parser();
    bool parse(iterator_type first, iterator_type last, std::vector<mapnik::feature_ptr> & features);
private:
    const std::unique_ptr<feature_collection_grammar<iterator_type,feature_type> > grammar_;
};

}}

#endif //MAPNIK_FEATURE_COLLECTION_PARSER_HPP
