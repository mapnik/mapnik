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
#include <mapnik/datasource.hpp>

// boost
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>
// stl
#include <vector>

namespace mapnik { namespace json {

template <typename Iterator, typename FeatureType> struct feature_collection_grammar;

template <typename Iterator>
class feature_collection_parser : private boost::noncopyable
{
    typedef Iterator iterator_type;
    typedef mapnik::Feature feature_type;
public:
    feature_collection_parser(mapnik::context_ptr const& ctx, mapnik::transcoder const& tr);
    ~feature_collection_parser();
    bool parse(iterator_type first, iterator_type last, std::vector<mapnik::feature_ptr> & features);  
private:
    boost::scoped_ptr<feature_collection_grammar<iterator_type,feature_type> > grammar_; 
};

}}

#endif //MAPNIK_FEATURE_COLLECTION_PARSER_HPP
