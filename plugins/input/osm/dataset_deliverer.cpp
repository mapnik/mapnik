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
#include <mapnik/debug.hpp>
#include <mapnik/datasource.hpp>

// boost
#include <boost/filesystem/operations.hpp>

// std
#include <sstream>

#include "dataset_deliverer.h"
#include "basiccurl.h"

osm_dataset * dataset_deliverer::dataset = NULL;
std::string dataset_deliverer::last_bbox = "";
std::string dataset_deliverer::last_filename = "";

osm_dataset* dataset_deliverer::load_from_file(const string& file, const string& parser)
{
    // Only actually load from file if we haven't done so already
    if (dataset == NULL)
    {
        if (!boost::filesystem::exists(file))
        {
            throw mapnik::datasource_exception("OSM Plugin: '" + file + "' does not exist");
        }

        dataset = new osm_dataset;
        if (dataset->load(file.c_str(), parser) == false)
        {
            return NULL;
        }

        atexit(dataset_deliverer::release);
        last_filename = file;
    }
    else if(file != last_filename)
    {
        dataset = new osm_dataset;
        if (dataset->load(file.c_str(), parser) == false)
        {
            return NULL;
        }
        last_filename = file;
    }
    return dataset;
}

osm_dataset* dataset_deliverer::load_from_url(const string& url, const string& bbox, const string& parser)
{
    if (dataset == NULL)
    {
        dataset = new osm_dataset;
        if (dataset->load_from_url(url.c_str(), bbox, parser) == false)
        {
            return NULL;
        }

        atexit(dataset_deliverer::release);
        last_bbox = bbox;
    }
    else if (bbox != last_bbox)
    {
        MAPNIK_LOG_WARN(osm) << "osm_dataset_deliverer: BBoxes are different=" << last_bbox << "," << bbox;

        // Reload the dataset
        dataset->clear();
        if (dataset->load_from_url(url.c_str(), bbox, parser) == false)
        {
            return NULL;
        }

        last_bbox = bbox;
    }
    return dataset;
}
