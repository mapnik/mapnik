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
#include <mapnik/geometry_type.hpp>
#include <mapnik/util/fs.hpp>


namespace detail {

mapnik::feature_ptr fetch_first_feature(std::string const& filename, bool cache_features)
{
    mapnik::parameters params;
    params["type"] = "geojson";
    params["file"] = filename;
    params["cache-features"] = cache_features;
    auto ds = mapnik::datasource_cache::instance().create(params);
    auto fields = ds->get_descriptor().get_descriptors();
    mapnik::query query(ds->envelope());
    for (auto const& field : fields)
    {
        query.add_property_name(field.get_name());
    }
    auto features = ds->features(query);
    auto feature = features->next();
    return feature;
}

}

TEST_CASE("geojson") {

    std::string geojson_plugin("./plugins/input/geojson.input");
    if (mapnik::util::exists(geojson_plugin))
    {
        SECTION("GeoJSON Point")
        {
            {
                // cache features in memory
                auto feature = detail::fetch_first_feature("./test/data/json/point.json", true);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::Point);
                auto const& pt = mapnik::util::get<mapnik::geometry::point<double> >(geometry);
                REQUIRE(pt.x == 100);
                REQUIRE(pt.y == 0);
            }
            {
                // on-fly in-memory r-tree index + read features from disk
                auto feature = detail::fetch_first_feature("./test/data/json/point.json", false);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::Point);
                auto const& pt = mapnik::util::get<mapnik::geometry::point<double> >(geometry);
                REQUIRE(pt.x == 100);
                REQUIRE(pt.y == 0);
            }
        }

        SECTION("GeoJSON LineString")
        {
            mapnik::parameters params;
            params["type"] = "geojson";
            params["file"] = "./test/data/json/linestring.json";

            {
                // cache features in memory
                auto feature = detail::fetch_first_feature("./test/data/json/linestring.json", true);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::LineString);
                auto const& line = mapnik::util::get<mapnik::geometry::line_string<double> >(geometry);
                REQUIRE(line.size() == 2);
                REQUIRE(mapnik::geometry::envelope(line) == mapnik::box2d<double>(100,0,101,1));

            }
            {
                // on-fly in-memory r-tree index + read features from disk
                auto feature = detail::fetch_first_feature("./test/data/json/linestring.json", false);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::LineString);
                auto const& line = mapnik::util::get<mapnik::geometry::line_string<double> >(geometry);
                REQUIRE(line.size() == 2);
                REQUIRE(mapnik::geometry::envelope(line) == mapnik::box2d<double>(100,0,101,1));
            }
        }

        SECTION("GeoJSON Polygon")
        {
            mapnik::parameters params;
            params["type"] = "geojson";
            params["file"] = "./test/data/json/polygon.json";

            {
                // cache features in memory
                auto feature = detail::fetch_first_feature("./test/data/json/polygon.json", true);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::Polygon);
                auto const& poly = mapnik::util::get<mapnik::geometry::polygon<double> >(geometry);
                REQUIRE(poly.num_rings() == 2);
                REQUIRE(poly.exterior_ring.size() == 5);
                REQUIRE(poly.interior_rings.size() == 1);
                REQUIRE(poly.interior_rings[0].size() == 5);
                REQUIRE(mapnik::geometry::envelope(poly) == mapnik::box2d<double>(100,0,101,1));

            }
            {
                // on-fly in-memory r-tree index + read features from disk
                auto feature = detail::fetch_first_feature("./test/data/json/polygon.json", false);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::Polygon);
                auto const& poly = mapnik::util::get<mapnik::geometry::polygon<double> >(geometry);
                REQUIRE(poly.num_rings() == 2);
                REQUIRE(poly.exterior_ring.size() == 5);
                REQUIRE(poly.interior_rings.size() == 1);
                REQUIRE(poly.interior_rings[0].size() == 5);
                REQUIRE(mapnik::geometry::envelope(poly) == mapnik::box2d<double>(100,0,101,1));
            }
        }

        SECTION("GeoJSON MultiPoint")
        {
            {
                // cache features in memory
                auto feature = detail::fetch_first_feature("./test/data/json/multipoint.json", true);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::MultiPoint);
                auto const& multi_pt = mapnik::util::get<mapnik::geometry::multi_point<double> >(geometry);
                REQUIRE(multi_pt.size() == 2);
                REQUIRE(mapnik::geometry::envelope(multi_pt) == mapnik::box2d<double>(100,0,101,1));
            }
            {
                // on-fly in-memory r-tree index + read features from disk
                auto feature = detail::fetch_first_feature("./test/data/json/multipoint.json", false);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::MultiPoint);
                auto const& multi_pt = mapnik::util::get<mapnik::geometry::multi_point<double> >(geometry);
                REQUIRE(multi_pt.size() == 2);
                REQUIRE(mapnik::geometry::envelope(multi_pt) == mapnik::box2d<double>(100,0,101,1));
            }
        }

        SECTION("GeoJSON MultiLineString")
        {
            {
                // cache features in memory
                auto feature = detail::fetch_first_feature("./test/data/json/multilinestring.json", true);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::MultiLineString);
                auto const& multi_line = mapnik::util::get<mapnik::geometry::multi_line_string<double> >(geometry);
                REQUIRE(multi_line.size() == 2);
                REQUIRE(multi_line[0].size() == 2);
                REQUIRE(multi_line[1].size() == 2);
                REQUIRE(mapnik::geometry::envelope(multi_line) == mapnik::box2d<double>(100,0,103,3));

            }
            {
                // on-fly in-memory r-tree index + read features from disk
                auto feature = detail::fetch_first_feature("./test/data/json/multilinestring.json", false);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::MultiLineString);
                auto const& multi_line = mapnik::util::get<mapnik::geometry::multi_line_string<double> >(geometry);
                REQUIRE(multi_line.size() == 2);
                REQUIRE(multi_line[0].size() == 2);
                REQUIRE(multi_line[1].size() == 2);
                REQUIRE(mapnik::geometry::envelope(multi_line) == mapnik::box2d<double>(100,0,103,3));
            }
        }

        SECTION("GeoJSON MultiPolygon")
        {
            {
                // cache features in memory
                auto feature = detail::fetch_first_feature("./test/data/json/multipolygon.json", true);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::MultiPolygon);
                auto const& multi_poly = mapnik::util::get<mapnik::geometry::multi_polygon<double> >(geometry);
                REQUIRE(multi_poly.size() == 2);
                REQUIRE(multi_poly[0].num_rings() == 1);
                REQUIRE(multi_poly[1].num_rings() == 2);
                REQUIRE(mapnik::geometry::envelope(multi_poly) == mapnik::box2d<double>(100,0,103,3));

            }
            {
                // on-fly in-memory r-tree index + read features from disk
                auto feature = detail::fetch_first_feature("./test/data/json/multipolygon.json", false);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::MultiPolygon);
                auto const& multi_poly = mapnik::util::get<mapnik::geometry::multi_polygon<double> >(geometry);
                REQUIRE(multi_poly.size() == 2);
                REQUIRE(multi_poly[0].num_rings() == 1);
                REQUIRE(multi_poly[1].num_rings() == 2);
                REQUIRE(mapnik::geometry::envelope(multi_poly) == mapnik::box2d<double>(100,0,103,3));
            }
        }

        SECTION("GeoJSON GeometryCollection")
        {
            {
                // cache features in memory
                auto feature = detail::fetch_first_feature("./test/data/json/geometrycollection.json", true);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::GeometryCollection);
                auto const& collection = mapnik::util::get<mapnik::geometry::geometry_collection<double> >(geometry);
                REQUIRE(collection.size() == 2);
                REQUIRE(mapnik::geometry::geometry_type(collection[0]) == mapnik::geometry::Point);
                REQUIRE(mapnik::geometry::geometry_type(collection[1]) == mapnik::geometry::LineString);
                REQUIRE(mapnik::geometry::envelope(collection) == mapnik::box2d<double>(100,0,102,1));
            }
            {
                // cache features in memory
                auto feature = detail::fetch_first_feature("./test/data/json/geometrycollection.json", false);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::GeometryCollection);
                auto const& collection = mapnik::util::get<mapnik::geometry::geometry_collection<double> >(geometry);
                REQUIRE(collection.size() == 2);
                REQUIRE(mapnik::geometry::geometry_type(collection[0]) == mapnik::geometry::Point);
                REQUIRE(mapnik::geometry::geometry_type(collection[1]) == mapnik::geometry::LineString);
                REQUIRE(mapnik::geometry::envelope(collection) == mapnik::box2d<double>(100,0,102,1));
            }
        }

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
            for (auto const& field : fields)
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
            for (auto const& field : fields)
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
            for (auto const& field : fields)
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
            for (auto const& field : fields)
            {
                query.add_property_name(field.get_name());
            }
            auto features = ds->features(query);
            REQUIRE(features != nullptr);
            auto feature = features->next();
            REQUIRE(feature != nullptr);
            REQUIRE(feature->envelope() == mapnik::box2d<double>(123,456,123,456));
        }
        SECTION("json - ensure input fully consumed and throw exception otherwise")
        {
            mapnik::parameters params;
            params["type"] = "geojson";
            params["file"] = "./test/data/json/points-malformed.geojson"; // mismatched parentheses
            params["cache-features"] = false;
            REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));
            params["cache-features"] = true;
            REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));
        }
    }
}
