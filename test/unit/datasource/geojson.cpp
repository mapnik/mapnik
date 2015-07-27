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
#include <mapnik/util/fs.hpp>

TEST_CASE("geojson") {

    std::string geojson_plugin("./plugins/input/geojson.input");
    if (mapnik::util::exists(geojson_plugin))
    {
        SECTION("json feature cache-feature=\"true\"")
        {
            // Create datasource
            mapnik::parameters params;
            params["type"] = "geojson";
            params["file"] = "./test/data/json/feature.json";
            params["cache-features"] = true;
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(bool(ds));
            auto fields = ds->get_descriptor().get_descriptors();
            mapnik::query query(ds->envelope());
            for (auto const &field : fields)
            {
                query.add_property_name(field.get_name());
            }
            auto features = ds->features(query);
            REQUIRE(features != nullptr);
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
            REQUIRE(features != nullptr);
            auto feature = features->next();
            REQUIRE(feature != nullptr);
        }

        SECTION("json extra properties cache-feature=\"true\"")
        {
            // Create datasource
            mapnik::parameters params;
            params["type"] = "geojson";
            params["file"] = "./test/data/json/feature_collection_extra_properties.json";
            params["cache-features"] = true;
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(bool(ds));
            auto fields = ds->get_descriptor().get_descriptors();
            mapnik::query query(ds->envelope());
            for (auto const &field : fields)
            {
                query.add_property_name(field.get_name());
            }
            auto features = ds->features(query);
            REQUIRE(features != nullptr);
            auto feature = features->next();
            REQUIRE(feature != nullptr);
            REQUIRE(feature->envelope() == mapnik::box2d<double>(123,456,123,456));
        }

        SECTION("json extra properties cache-feature=\"false\"")
        {
            // Create datasource
            mapnik::parameters params;
            params["type"] = "geojson";
            params["file"] = "./test/data/json/feature_collection_extra_properties.json";
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
            REQUIRE(features != nullptr);
            auto feature = features->next();
            REQUIRE(feature != nullptr);
            REQUIRE(feature->envelope() == mapnik::box2d<double>(123,456,123,456));
        }

    }
}
