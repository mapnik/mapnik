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

#ifndef GEOJSON_INDEX_FEATURESET_HPP
#define GEOJSON_INDEX_FEATURESET_HPP

#include "geojson_datasource.hpp"
#include <mapnik/feature.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/util/spatial_index.hpp>

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
MAPNIK_DISABLE_WARNING_POP
#include <mapnik/mapped_memory_cache.hpp>
#endif

#include <deque>
#include <cstdio>

class geojson_index_featureset : public mapnik::Featureset
{
    using value_type = mapnik::util::index_record;
public:
    geojson_index_featureset(std::string const& filename, mapnik::bounding_box_filter<float> const& filter);
    virtual ~geojson_index_featureset();
    mapnik::feature_ptr next();

private:
#if defined (MAPNIK_MEMORY_MAPPED_FILE)
    using file_source_type = boost::interprocess::ibufferstream;
    mapnik::mapped_region_ptr mapped_region_;
#else
    using file_ptr = std::unique_ptr<std::FILE, int (*)(std::FILE *)>;
    file_ptr file_;
#endif
    mapnik::value_integer feature_id_ = 1;
    mapnik::context_ptr ctx_;
    std::vector<value_type> positions_;
    std::vector<value_type>::iterator itr_;
};

#endif // GEOJSON_INDEX_FEATURESE_HPP
