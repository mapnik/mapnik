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

// mapnik
#include <mapnik/json/topology.hpp>
#include <mapnik/json/topojson_utils.hpp>
// stl
#include <string>
#include <vector>
#include <fstream>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/range/adaptor/reversed.hpp>
MAPNIK_DISABLE_WARNING_POP

#include "topojson_featureset.hpp"

topojson_featureset::topojson_featureset(mapnik::topojson::topology const& topo,
                                         mapnik::transcoder const& tr,
                                         array_type&& index_array)
    : ctx_(std::make_shared<mapnik::context_type>())
    , topo_(topo)
    , tr_(tr)
    , index_array_(std::move(index_array))
    , index_itr_(index_array_.begin())
    , index_end_(index_array_.end())
    , feature_id_(1)
{}

topojson_featureset::~topojson_featureset() {}

mapnik::feature_ptr topojson_featureset::next()
{
    if (index_itr_ != index_end_)
    {
        topojson_datasource::item_type const& item = *index_itr_++;
        std::size_t index = item.second;
        if (index < topo_.geometries.size())
        {
            mapnik::topojson::geometry const& geom = topo_.geometries[index];
            mapnik::feature_ptr feature = mapnik::util::apply_visitor(
              mapnik::topojson::feature_generator<mapnik::context_ptr>(ctx_, tr_, topo_, feature_id_++),
              geom);
            return feature;
        }
    }

    return mapnik::feature_ptr();
}
