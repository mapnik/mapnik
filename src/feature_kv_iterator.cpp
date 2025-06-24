/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#include <mapnik/feature_kv_iterator.hpp>
#include <mapnik/feature.hpp>

namespace mapnik {

feature_kv_iterator::feature_kv_iterator(feature_impl const& f, bool begin)
    : f_(f)
    , itr_(begin ? f_.ctx_->begin() : f_.ctx_->end())
{}

void feature_kv_iterator::increment()
{
    ++itr_;
}

void feature_kv_iterator::decrement()
{
    // dummy //--itr_;
}

void feature_kv_iterator::advance(boost::iterator_difference<feature_kv_iterator>::type)
{
    // dummy
}

bool feature_kv_iterator::equal(feature_kv_iterator const& other) const
{
    return (itr_ == other.itr_);
}

feature_kv_iterator::value_type const& feature_kv_iterator::dereference() const
{
    std::get<0>(kv_) = itr_->first;
    std::get<1>(kv_) = f_.get(itr_->second);
    return kv_;
}

} // namespace mapnik
