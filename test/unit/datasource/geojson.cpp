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

#include <mapnik/map.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/geometry.hpp>


static const std::string geojson_plugin("./plugins/input/geojson.input");
const bool registered = mapnik::datasource_cache::instance().register_datasources(geojson_plugin);

TEST_CASE("geojson") {

SECTION("json feature cache-feature=\"true\"")
{
    // check the GeoJSON datasource can be loaded
    const std::vector<std::string> plugin_names =
        mapnik::datasource_cache::instance().plugin_names();
    const bool have_csv_plugin =
        std::find(plugin_names.begin(), plugin_names.end(), "geojson") != plugin_names.end();
    // Create datasource
    mapnik::parameters params;
    params["type"] = "geojson";
    params["file"] = "./test/data/json/feature.json";
    params["cache-features"] = false;
    auto ds = mapnik::datasource_cache::instance().create(params);
    REQUIRE(bool(ds));
    auto fields = ds->get_descriptor().get_descriptors();
    mapnik::query query(ds->envelope());
    for (auto const &field : fields)
    {
        query.add_property_name(field.get_name());
    }
    auto features = ds->features(query);
    auto feature = features->next();
    REQUIRE(feature != nullptr);
}

SECTION("json feature cache-feature=\"false\"")
{
    mapnik::parameters params;
    params["type"] = "geojson";
    params["file"] = "./test/data/json/feature.json";
    params["cache-features"] = false;
    auto ds = mapnik::datasource_cache::instance().create(params);
    REQUIRE(bool(ds));
    auto fields = ds->get_descriptor().get_descriptors();
    mapnik::query query(ds->envelope());
    for (auto const &field : fields)
    {
        query.add_property_name(field.get_name());
    }
    auto features = ds->features(query);
    auto feature = features->next();
    REQUIRE(feature != nullptr);
}

}
