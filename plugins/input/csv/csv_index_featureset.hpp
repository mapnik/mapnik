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

#ifndef CSV_INDEX_FEATURESET_HPP
#define CSV_INDEX_FEATURESET_HPP

#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/util/spatial_index.hpp>
#include "csv_utils.hpp"
#include "csv_datasource.hpp"

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
MAPNIK_DISABLE_WARNING_POP
#include <mapnik/mapped_memory_cache.hpp>
#endif

class csv_index_featureset : public mapnik::Featureset
{
    using value_type = mapnik::util::index_record;
    using locator_type = csv_utils::geometry_column_locator;

  public:

    csv_index_featureset(std::string const& filename,
                         mapnik::bounding_box_filter<float> const& filter,
                         locator_type const& locator,
                         char separator,
                         char quote,
                         std::vector<std::string> const& headers,
                         mapnik::context_ptr const& ctx);
    ~csv_index_featureset();
    mapnik::feature_ptr next();

  private:
    mapnik::feature_ptr parse_feature(char const* beg, char const* end);
    char separator_;
    char quote_;
    std::vector<std::string> headers_;
    mapnik::context_ptr ctx_;
    mapnik::value_integer feature_id_ = 0;
    locator_type const& locator_;
    mapnik::transcoder tr_;
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    using file_source_type = boost::interprocess::ibufferstream;
    mapnik::mapped_region_ptr mapped_region_;
#else
    using file_ptr = std::unique_ptr<std::FILE, int (*)(std::FILE*)>;
    file_ptr file_;
#endif
    std::vector<value_type> positions_;
    std::vector<value_type>::iterator itr_;
};

#endif // CSV_INDEX_FEATURESET_HPP
