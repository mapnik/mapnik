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

#ifndef GEOJSON_INDEX_FEATURESET_HPP
#define GEOJSON_INDEX_FEATURESET_HPP

#include <mapnik/feature.hpp>
#include "geojson_datasource.hpp"

#include <deque>
#include <cstdio>

class geojson_index_featureset : public mapnik::Featureset
{
public:
    using file_ptr = std::unique_ptr<std::FILE, int (*)(std::FILE *)>;
    geojson_index_featureset(std::string const& filename);
    virtual ~geojson_index_featureset();
    mapnik::feature_ptr next();

private:
    file_ptr file_;
    mapnik::context_ptr ctx_;
};

#endif // GEOJSON_INDEX_FEATURESE_HPP
