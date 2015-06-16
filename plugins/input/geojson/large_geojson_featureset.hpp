/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef LARGE_GEOJSON_FEATURESET_HPP
#define LARGE_GEOJSON_FEATURESET_HPP

#include <mapnik/feature.hpp>
#include "geojson_datasource.hpp"

#include <vector>
#include <deque>
#include <fstream>
#include <cstdio>

class large_geojson_featureset : public mapnik::Featureset
{
public:
    using array_type = std::deque<geojson_datasource::item_type>;
    using file_ptr = std::unique_ptr<std::FILE, int (*)(std::FILE *)>;

    large_geojson_featureset(std::string const& filename,
                             array_type && index_array);
    virtual ~large_geojson_featureset();
    mapnik::feature_ptr next();

private:
    file_ptr file_;

    const array_type index_array_;
    array_type::const_iterator index_itr_;
    array_type::const_iterator index_end_;
    mapnik::context_ptr ctx_;
};

#endif // LARGE_GEOJSON_FEATURESET_HPP
