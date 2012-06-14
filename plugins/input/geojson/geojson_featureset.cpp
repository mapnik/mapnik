/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/feature.hpp>
// stl
#include <string>
#include <vector>
#include <fstream>

#include "geojson_featureset.hpp"

geojson_featureset::geojson_featureset(std::vector<mapnik::feature_ptr> const& features, 
                                       std::deque<std::size_t>::const_iterator index_itr,
                                       std::deque<std::size_t>::const_iterator index_end)
    : features_(features),
      index_itr_(index_itr),
      index_end_(index_end) {}

geojson_featureset::~geojson_featureset() {}

mapnik::feature_ptr geojson_featureset::next()
{
    if (index_itr_ != index_end_)
    {
        std::size_t index = *index_itr_++;        
        if ( index < features_.size())
        {
            return features_.at(index);
        }
    }    
    return mapnik::feature_ptr();
}
