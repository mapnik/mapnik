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

#include "catch.hpp"

#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/util/fs.hpp>
#include <cstdlib>
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
#pragma GCC diagnostic pop

namespace {

std::size_t count_shapefile_features(std::string const& filename)
{
    mapnik::parameters params;
    params["type"] = "shape";
    params["file"] = filename;
    auto ds = mapnik::datasource_cache::instance().create(params);
    CHECK(ds->type() == mapnik::datasource::datasource_t::Vector);
    auto fields = ds->get_descriptor().get_descriptors();
    mapnik::query query(ds->envelope());
    for (auto const& field : fields)
    {
        query.add_property_name(field.get_name());
    }
    auto features = ds->features(query);

    std::size_t feature_count = 0;
    auto feature = features->next();
    while (feature)
    {
        ++feature_count;
        feature = features->next();
    }
    return feature_count;
}

int create_shapefile_index(std::string const& filename, bool silent = true)
{
    std::string cmd;
    if (std::getenv("DYLD_LIBRARY_PATH") != nullptr)
    {
        cmd += std::string("export DYLD_LIBRARY_PATH=") + std::getenv("DYLD_LIBRARY_PATH") + " && ";
    }
    cmd += "shapeindex " + filename;
    if (silent)
    {
#ifndef _WINDOWS
        cmd += " 2>/dev/null";
#else
        cmd += " 2> nul";
#endif
    }
    return std::system(cmd.c_str());
}

}

TEST_CASE("shapeindex")
{
    std::string shape_plugin("./plugins/input/shape.input");
    if (mapnik::util::exists(shape_plugin))
    {
        SECTION("Shapefile index")
        {
            for (auto const& path : mapnik::util::list_directory("test/data/shp/"))
            {
                if (boost::iends_with(path,".shp"))
                {
                    std::string index_path = path.substr(0, path.rfind(".")) + ".index";
                    // remove *.index if present
                    if (mapnik::util::exists(index_path))
                    {
                        mapnik::util::remove(index_path);
                    }
                    // count features
                    std::size_t feature_count = count_shapefile_features(path);
                    // create *.index
                    create_shapefile_index(path);
                    if (feature_count == 0)
                    {
                        REQUIRE(!mapnik::util::exists(index_path)); // index won't be created if there's no features
                    }
                    // count features
                    std::size_t feature_count_indexed = count_shapefile_features(path);
                    // ensure number of features are the same
                    REQUIRE(feature_count == feature_count_indexed);
                    // remove *.index if present
                    if (mapnik::util::exists(index_path))
                    {
                        mapnik::util::remove(index_path);
                    }
                }
            }
        }
    }
}
