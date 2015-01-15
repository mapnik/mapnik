/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/debug.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/util/fs.hpp>

// std
#include <sstream>

#include "dataset_deliverer.h"

osm_dataset * dataset_deliverer::dataset = nullptr;
std::string dataset_deliverer::last_bbox = "";
std::string dataset_deliverer::last_filename = "";

osm_dataset* dataset_deliverer::load_from_file(const string& file, const string& parser)
{
    // Only actually load from file if we haven't done so already
    if (dataset == nullptr)
    {
        if (!mapnik::util::exists(file))
        {
            throw mapnik::datasource_exception("OSM Plugin: '" + file + "' does not exist");
        }

        dataset = new osm_dataset;
        if (dataset->load(file.c_str(), parser) == false)
        {
            return nullptr;
        }

        atexit(dataset_deliverer::release);
        last_filename = file;
    }
    else if(file != last_filename)
    {
        dataset = new osm_dataset;
        if (dataset->load(file.c_str(), parser) == false)
        {
            return nullptr;
        }
        last_filename = file;
    }
    return dataset;
}
