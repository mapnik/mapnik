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

#ifndef MAPNIK_FEATURE_KV_ITERATOR_HPP
#define MAPNIK_FEATURE_KV_ITERATOR_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/value.hpp>
// boost
#include <boost/tuple/tuple.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/variant.hpp>
// stl
#include <map>


namespace mapnik {

class feature_impl;

class MAPNIK_DECL feature_kv_iterator :
        public boost::iterator_facade<feature_kv_iterator,
                                      boost::tuple<std::string , value> const,
                                      boost::forward_traversal_tag>
{
public:
    typedef boost::tuple<std::string,value> value_type;

    feature_kv_iterator (feature_impl const& f, bool begin = false);
private:
    friend class boost::iterator_core_access;
    void increment();
    void decrement();
    void advance(boost::iterator_difference<feature_kv_iterator>::type);
    bool equal( feature_kv_iterator const& other) const;

    value_type const& dereference() const;

    feature_impl const& f_;
    std::map<std::string,std::size_t>::const_iterator itr_;
    mutable value_type kv_;

};

struct value_not_null
{
    bool operator() (feature_kv_iterator::value_type const& kv) const
    {
        return !boost::apply_visitor(is_null, boost::get<1>(kv).base());
    }
};

typedef boost::filter_iterator<value_not_null, feature_kv_iterator> feature_kv_iterator2;

}

#endif // MAPNIK_FEATURE_KV_ITERATOR_HPP

