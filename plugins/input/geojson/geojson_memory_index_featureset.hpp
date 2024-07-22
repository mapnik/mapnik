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

#ifndef GEOJSON_MEMORY_INDEX_FEATURESET_HPP
#define GEOJSON_MEMORY_INDEX_FEATURESET_HPP

#include <mapnik/feature.hpp>
#include "geojson_datasource.hpp"

#include <deque>
#include <cstdio>

class geojson_memory_index_featureset : public mapnik::Featureset
{
  public:
    using array_type = std::deque<geojson_datasource::item_type>;
    using file_ptr = std::unique_ptr<std::FILE, int (*)(std::FILE*)>;

    geojson_memory_index_featureset(std::string const& filename, array_type&& index_array);
    virtual ~geojson_memory_index_featureset();
    mapnik::feature_ptr next();

  private:
    file_ptr file_;
    mapnik::value_integer feature_id_ = 1;
    const array_type index_array_;
    array_type::const_iterator index_itr_;
    array_type::const_iterator index_end_;
    mapnik::context_ptr ctx_;
};

#endif // GEOJSON_MEMORY_INDEX_FEATURESET_HPP
