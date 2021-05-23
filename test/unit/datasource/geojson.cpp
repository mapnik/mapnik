/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include "ds_test_util.hpp"

#include <mapnik/unicode.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry/geometry_type.hpp>
#include <mapnik/json/geometry_parser.hpp>
#include <mapnik/util/geometry_to_geojson.hpp>
#include <mapnik/util/fs.hpp>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <locale>
#include <boost/optional/optional_io.hpp>

/*

Compile and run just this test:

clang++ -o test-geojson -g -I./test/ test/unit/run.cpp test/unit/datasource/geojson.cpp `mapnik-config --all-flags`
./test-geojson -d yes

*/

namespace {

std::pair<mapnik::datasource_ptr,mapnik::feature_ptr> fetch_first_feature(std::string const& filename, bool cache_features)
{
    mapnik::parameters params;
    params["type"] = "geojson";
    params["file"] = filename;
    params["cache_features"] = cache_features;
    auto ds = mapnik::datasource_cache::instance().create(params);
    CHECK(ds->type() == mapnik::datasource::datasource_t::Vector);
    auto fields = ds->get_descriptor().get_descriptors();
    mapnik::query query(ds->envelope());
    for (auto const& field : fields)
    {
        query.add_property_name(field.get_name());
    }
    auto features = ds->features(query);
    auto feature = features->next();
    return std::make_pair(ds,feature);
}


void iterate_over_features(mapnik::featureset_ptr features)
{
    auto feature = features->next();
    while (feature != nullptr)
    {
        feature = features->next();
    }
}

}

TEST_CASE("geojson") {

    std::string geojson_plugin("./plugins/input/geojson.input");
    if (mapnik::util::exists(geojson_plugin))
    {
        SECTION("GeoJSON I/O errors")
        {
            std::string filename = "does_not_exist.geojson";
            for (auto create_index : { true, false })
            {
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    // index wont be created
                    CHECK(!mapnik::util::exists(filename + ".index"));
                }

                for (auto cache_features : {true, false})
                {
                    mapnik::parameters params;
                    params["type"] = "geojson";
                    params["file"] = filename;
                    params["cache_features"] = cache_features;
                    REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));
                }
            }
        }

        SECTION("GeoJSON an empty FeatureCollection")
        {
            for (auto cache_features : {true, false})
            {
                mapnik::parameters params;
                params["type"] = "geojson";
                params["file"] = "./test/data/json/empty_featurecollection.json";
                params["cache_features"] = cache_features;
                REQUIRE(mapnik::datasource_cache::instance().create(params));
            }
        }

        SECTION("GeoJSON empty Geometries handling")
        {
            auto valid_empty_geometries =
                {
                    "null", // Point can't be empty
                    "{ \"type\": \"LineString\", \"coordinates\": [] }",
                    "{ \"type\": \"Polygon\", \"coordinates\": [ [ ] ] } ",
                    "{ \"type\": \"MultiPoint\", \"coordinates\": [ ] }",
                    "{ \"type\": \"MultiLineString\", \"coordinates\": [ [] ] }",
                    "{ \"type\": \"MultiPolygon\", \"coordinates\": [[ []] ] }"
                };

            for (auto const& in  : valid_empty_geometries)
            {
                std::string json(in);
                mapnik::geometry::geometry<double> geom;
                CHECK(mapnik::json::from_geojson(json, geom));
                // round trip
                std::string json_out;
                CHECK(mapnik::util::to_geojson(json_out, geom));
                json.erase(std::remove_if(
                               std::begin(json), std::end(json),
                               [l = std::locale{}](auto ch) { return std::isspace(ch, l); }
                               ), std::end(json));
                REQUIRE(json == json_out);
            }

            auto invalid_empty_geometries =
                {
                    "{ \"type\": \"Point\", \"coordinates\": [] }",
                    "{ \"type\": \"LineString\", \"coordinates\": [[]] }"
                    "{ \"type\": \"Polygon\", \"coordinates\": [[[]]] }",
                    "{ \"type\": \"MultiPoint\", \"coordinates\": [[]] }",
                    "{ \"type\": \"MultiLineString\", \"coordinates\": [[[]]] }",
                    "{ \"type\": \"MultiPolygon\", \"coordinates\": [[[[]]]] }"
                };

            for (auto const& json  : invalid_empty_geometries)
            {
                mapnik::geometry::geometry<double> geom;
                CHECK(!mapnik::json::from_geojson(json, geom));
            }
        }

        SECTION("GeoJSON num_features_to_query")
        {
            std::string filename = "./test/data/json/featurecollection-multipleprops.geojson";
            for (mapnik::value_integer num_features_to_query : { mapnik::value_integer(-1),
                        mapnik::value_integer(0),
                        mapnik::value_integer(1),
                        mapnik::value_integer(2),
                        mapnik::value_integer(3),
                        std::numeric_limits<mapnik::value_integer>::max()})
            {
                for (auto create_index : { true, false })
                {
                    if (create_index)
                    {
                        int ret = create_disk_index(filename);
                        int ret_posix = (ret >> 8) & 0x000000ff;
                        INFO(ret);
                        INFO(ret_posix);
                        CHECK(mapnik::util::exists(filename + ".index"));
                    }

                    for (auto cache_features : {true, false})
                    {
                        mapnik::parameters params;
                        params["type"] = "geojson";
                        params["file"] = filename;
                        params["cache_features"] = cache_features;
                        params["num_features_to_query"] = num_features_to_query;
                        auto ds = mapnik::datasource_cache::instance().create(params);
                        CHECK(ds != nullptr);
                        auto fields = ds->get_descriptor().get_descriptors();
                        if (!create_index && cache_features)
                        {
                            // when there's no index and caching is enabled descriptor is always fully initialised
                            REQUIRE(fields.size() == 2);
                        }
                        else
                        {
                            // at least 1 feature should be queried
                            REQUIRE(fields.size() == std::min(std::max(mapnik::value_integer(1), num_features_to_query),
                                                              mapnik::value_integer(2)));
                        }
                    }
                    // cleanup
                    if (create_index && mapnik::util::exists(filename + ".index"))
                    {
                        mapnik::util::remove(filename + ".index");
                    }
                }
            }
        }

        SECTION("GeoJSON attribute descriptors are alphabetically ordered")
        {
            for (auto cache_features : {true, false})
            {
                mapnik::parameters params;
                params["type"] = "geojson";
                params["file"] = "./test/data/json/properties.json";
                params["cache_features"] = cache_features;
                auto ds = mapnik::datasource_cache::instance().create(params);
                CHECK(ds != nullptr);
                std::vector<std::string> expected_names = {"a", "b", "c", "d", "e"};
                auto fields = ds->get_descriptor().get_descriptors();
                std::size_t index = 0;
                for (auto const& field : fields)
                {
                    REQUIRE(field.get_name() == expected_names[index++]);
                }
            }
        }

        SECTION("GeoJSON invalid Point")
        {
            for (auto cache_features : {true, false})
            {
                mapnik::parameters params;
                params["type"] = "geojson";
                params["file"] = "./test/data/json/point-invalid.json";
                params["cache_features"] = cache_features;
                REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));
            }
        }

        SECTION("GeoJSON Point")
        {
            for (auto cache_features : {true, false})
            {
                auto result = fetch_first_feature("./test/data/json/point.json", cache_features);
                auto feature = result.second;
                auto ds = result.first;
                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::Point);
                auto const& pt = mapnik::util::get<mapnik::geometry::point<double> >(geometry);
                REQUIRE(pt.x == 100);
                REQUIRE(pt.y == 0);
            }
        }

        SECTION("GeoJSON LineString")
        {
            for (auto cache_features : {true, false})
            {
                auto result = fetch_first_feature("./test/data/json/linestring.json", cache_features);
                auto feature = result.second;
                auto ds = result.first;
                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::LineString);
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::LineString);
                auto const& line = mapnik::util::get<mapnik::geometry::line_string<double> >(geometry);
                REQUIRE(line.size() == 2);
                REQUIRE(mapnik::geometry::envelope(line) == mapnik::box2d<double>(100,0,101,1));

            }
        }

        SECTION("GeoJSON Polygon")
        {
            for (auto cache_features : {true, false})
            {
                auto result = fetch_first_feature("./test/data/json/polygon.json", cache_features);
                auto feature = result.second;
                auto ds = result.first;
                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Polygon);
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::Polygon);
                auto const& poly = mapnik::util::get<mapnik::geometry::polygon<double> >(geometry);
                REQUIRE(poly.size() == 2);
                REQUIRE(poly[0].size() == 5);
                REQUIRE(poly[1].size() == 5);
                REQUIRE(mapnik::geometry::envelope(poly) == mapnik::box2d<double>(100,0,101,1));

            }
        }

        SECTION("GeoJSON MultiPoint")
        {
            for (auto cache_features : {true, false})
            {
                auto result = fetch_first_feature("./test/data/json/multipoint.json", cache_features);
                auto feature = result.second;
                auto ds = result.first;
                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::MultiPoint);
                auto const& multi_pt = mapnik::util::get<mapnik::geometry::multi_point<double> >(geometry);
                REQUIRE(multi_pt.size() == 2);
                REQUIRE(mapnik::geometry::envelope(multi_pt) == mapnik::box2d<double>(100,0,101,1));
            }
        }

        SECTION("GeoJSON MultiLineString")
        {
            for (auto cache_features : {true, false})
            {
                auto result = fetch_first_feature("./test/data/json/multilinestring.json", cache_features);
                auto feature = result.second;
                auto ds = result.first;
                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::LineString);
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
            for (auto cache_features : {true, false})
            {
                auto result = fetch_first_feature("./test/data/json/multipolygon.json", cache_features);
                auto feature = result.second;
                auto ds = result.first;
                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Polygon);
                // test
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::MultiPolygon);
                auto const& multi_poly = mapnik::util::get<mapnik::geometry::multi_polygon<double> >(geometry);
                REQUIRE(multi_poly.size() == 2);
                REQUIRE(multi_poly[0].size() == 1);
                REQUIRE(multi_poly[1].size() == 2);
                REQUIRE(mapnik::geometry::envelope(multi_poly) == mapnik::box2d<double>(100,0,103,3));

            }
        }

        SECTION("GeoJSON GeometryCollection")
        {
            std::string filename("./test/data/json/geometrycollection.json");
            for (auto create_index : { true, false })
            {
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    // index will not exist because this is not a featurecollection
                    CHECK(!mapnik::util::exists(filename + ".index"));
                }

                for (auto cache_features : {true, false})
                {
                    auto result = fetch_first_feature(filename, cache_features);
                    auto feature = result.second;
                    auto ds = result.first;
                    CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Collection);
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
        }

        SECTION("GeoJSON Feature")
        {
            // Create datasource
            mapnik::parameters params;
            params["type"] = "geojson";
            std::string base("./test/data/json/");
            std::string file("feature.json");
            params["base"] = base;
            params["file"] = file;
            std::string filename = base + file;
            for (auto create_index : { true, false })
            {
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    // index will not exist because this is not a featurecollection
                    CHECK(!mapnik::util::exists(filename + ".index"));
                }

                for (auto cache_features : {true, false})
                {
                    params["cache_features"] = cache_features;
                    auto ds = mapnik::datasource_cache::instance().create(params);
                    REQUIRE(bool(ds));
                    CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
                    auto fields = ds->get_descriptor().get_descriptors();
                    mapnik::query query(ds->envelope());
                    for (auto const& field : fields)
                    {
                        query.add_property_name(field.get_name());
                    }
                    auto features = ds->features(query);
                    auto features2 = ds->features_at_point(ds->envelope().center(),0);
                    auto feature = features->next();
                    auto feature2 = features2->next();
                    REQUIRE(feature != nullptr);
                    REQUIRE(feature2 != nullptr);
                    CHECK(feature->id() == 1);
                    CHECK(feature2->id() == 1);
                    mapnik::value val = feature->get("name");
                    CHECK(val.to_string() == "Dinagat Islands");
                    mapnik::value val2 = feature2->get("name");
                    CHECK(val2.to_string() == "Dinagat Islands");
                    REQUIRE(features->next() == nullptr);
                    REQUIRE(features2->next() == nullptr);
                }
            }
        }

        SECTION("GeoJSON ensure empty and null properties are handled")
        {
            mapnik::parameters params;
            params["type"] = "geojson";

            for (auto const& c_str : {"./test/data/json/feature-null-properties.json",
                        "./test/data/json/feature-empty-properties.json"})
            {
                std::string filename(c_str);
                params["file"] = filename;

                // cleanup in the case of a failed previous run
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }

                for (auto create_index : { true, false })
                {
                    if (create_index)
                    {
                        CHECK(!mapnik::util::exists(filename + ".index"));
                        int ret = create_disk_index(filename);
                        int ret_posix = (ret >> 8) & 0x000000ff;
                        INFO(ret);
                        INFO(ret_posix);
                        CHECK(!mapnik::util::exists(filename + ".index"));
                    }

                    for (auto cache_features : {true, false})
                    {
                        params["cache_features"] = cache_features;
                        auto ds = mapnik::datasource_cache::instance().create(params);
                        REQUIRE(bool(ds));
                        CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
                        auto fields = ds->get_descriptor().get_descriptors();
                        CHECK(fields.size() == 0);
                    }

                    // cleanup
                    if (create_index && mapnik::util::exists(filename + ".index"))
                    {
                        mapnik::util::remove(filename + ".index");
                    }
                }
            }
        }

        SECTION("GeoJSON FeatureCollection")
        {
            std::string filename("./test/data/json/featurecollection.json");

            // cleanup in the case of a failed previous run
            if (mapnik::util::exists(filename + ".index"))
            {
                mapnik::util::remove(filename + ".index");
            }

            for (auto create_index : { true, false })
            {
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }

                mapnik::parameters params;
                params["type"] = "geojson";
                params["file"] = filename;

                for (auto cache_features : {true, false})
                {
                    params["cache_features"] = cache_features;

                    auto ds = mapnik::datasource_cache::instance().create(params);
                    CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Collection);
                    auto fields = ds->get_descriptor().get_descriptors();
                    mapnik::query query(ds->envelope());
                    for (auto const& field : fields)
                    {
                        query.add_property_name(field.get_name());
                    }
                    auto features = ds->features(query);
                    auto features2 = ds->features_at_point(ds->envelope().center(),10);
                    auto bounding_box = ds->envelope();
                    mapnik::box2d<double> bbox;
                    mapnik::value_integer count = 0;
                    while (true)
                    {
                        auto feature = features->next();
                        auto feature2 = features2->next();
                        if (!feature || !feature2) break;
                        if (!bbox.valid()) bbox = feature->envelope();
                        else bbox.expand_to_include(feature->envelope());
                        ++count;
                        REQUIRE(feature->id() == count);
                        REQUIRE(feature2->id() == count);
                    }
                    REQUIRE(count == 3);
                    REQUIRE(bounding_box == bbox);
                }
                if (mapnik::util::exists(filename + ".index"))
                {
                    CHECK(mapnik::util::remove(filename + ".index"));
                }
            }
        }

        SECTION("GeoJSON extra properties")
        {
            // Create datasource
            mapnik::parameters params;
            params["type"] = "geojson";
            std::string filename("./test/data/json/feature_collection_extra_properties.json");
            params["file"] = filename;

            // cleanup in the case of a failed previous run
            if (mapnik::util::exists(filename + ".index"))
            {
                mapnik::util::remove(filename + ".index");
            }

            for (auto create_index : { true, false })
            {
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }

                for (auto cache_features : {true, false})
                {
                    params["cache_features"] = cache_features;
                    auto ds = mapnik::datasource_cache::instance().create(params);
                    CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
                    REQUIRE(bool(ds));
                    auto fields = ds->get_descriptor().get_descriptors();
                    mapnik::query query(ds->envelope());
                    for (auto const& field : fields)
                    {
                        query.add_property_name(field.get_name());
                    }
                    auto features = ds->features(query);
                    auto feature = features->next();
                    REQUIRE(feature != nullptr);
                    REQUIRE(feature->envelope() == mapnik::box2d<double>(123,456,123,456));
                }

                // cleanup
                if (create_index && mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        }

        SECTION("GeoJSON \"type\":\"FeatureCollection\" in Feature properties (#4140)")
        {
            // Create datasource
            mapnik::parameters params;
            params["type"] = "geojson";
            std::string filename("./test/data/json/feature_collection_issue_4140.json");
            params["file"] = filename;

            // cleanup in the case of a failed previous run
            if (mapnik::util::exists(filename + ".index"))
            {
                mapnik::util::remove(filename + ".index");
            }

            for (auto create_index : { true, false })
            {
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }

                for (auto cache_features : {true, false})
                {
                    params["cache_features"] = cache_features;
                    auto ds = mapnik::datasource_cache::instance().create(params);
                    CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
                    REQUIRE(bool(ds));
                    auto fields = ds->get_descriptor().get_descriptors();
                    mapnik::query query(ds->envelope());
                    for (auto const& field : fields)
                    {
                        query.add_property_name(field.get_name());
                    }
                    auto features = ds->features(query);
                    auto feature1 = features->next();
                    REQUIRE(feature1 != nullptr);
                    REQUIRE(feature1->envelope() == mapnik::box2d<double>(-122.0,48.0,-122.0,48.0));
                    auto feature2 = features->next();
                    REQUIRE(feature2 != nullptr);
                    REQUIRE(feature2->envelope() == mapnik::box2d<double>(0.0,51.0,0.0,51.0));
                    REQUIRE(ds->envelope() == mapnik::box2d<double>(-122.0,48.0,0.0,51.0));
                }

                // cleanup
                if (create_index && mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        }

        SECTION("GeoJSON ensure mapnik::datasource_cache::instance().create() throws on malformed input")
        {
            mapnik::parameters params;
            params["type"] = "geojson";

            for (auto const& c_str : {"./test/data/json/feature-malformed-1.geojson",
                        "./test/data/json/feature-malformed-2.geojson",
                        "./test/data/json/feature-malformed-3.geojson"})
            {
                std::string filename(c_str);
                params["file"] = filename;

                // cleanup in the case of a failed previous run
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }

                for (auto create_index : { true, false })
                {
                    if (create_index)
                    {
                        CHECK(!mapnik::util::exists(filename + ".index"));
                        int ret = create_disk_index(filename);
                        int ret_posix = (ret >> 8) & 0x000000ff;
                        INFO(ret);
                        INFO(ret_posix);
                        CHECK(!mapnik::util::exists(filename + ".index"));
                    }

                    for (auto cache_features : {true, false})
                    {
                        params["cache_features"] = cache_features;
                        CHECK_THROWS(mapnik::datasource_cache::instance().create(params));
                    }

                    // cleanup
                    if (create_index && mapnik::util::exists(filename + ".index"))
                    {
                        mapnik::util::remove(filename + ".index");
                    }
                }
            }
        }

        SECTION("GeoJSON ensure mapnik::featureset::next() throws on malformed input")
        {
            std::string filename{"./test/data/json/featurecollection-malformed.json"};
            mapnik::parameters params;
            params["type"] = "geojson";
            params["file"] = filename;

            // cleanup in the case of a failed previous run
            if (mapnik::util::exists(filename + ".index"))
            {
                mapnik::util::remove(filename + ".index");
            }

            CHECK(!mapnik::util::exists(filename + ".index"));
            int ret = create_disk_index(filename);
            int ret_posix = (ret >> 8) & 0x000000ff;
            INFO(ret);
            INFO(ret_posix);
            CHECK(mapnik::util::exists(filename + ".index"));

            for (auto cache_features : {true,false})
            {
                params["cache_features"] = cache_features;
                auto ds = mapnik::datasource_cache::instance().create(params);
                auto fields = ds->get_descriptor().get_descriptors();
                mapnik::query query(ds->envelope());
                auto features = ds->features(query);
                REQUIRE_THROWS(iterate_over_features(features));
            }

            // cleanup
            if (mapnik::util::exists(filename + ".index"))
            {
                mapnik::util::remove(filename + ".index");
            }
        }

        SECTION("GeoJSON ensure input fully consumed and throw exception otherwise")
        {
            mapnik::parameters params;
            params["type"] = "geojson";

            std::string filename("./test/data/json/points-malformed.geojson");
            params["file"] = filename; // mismatched parentheses

            // cleanup in the case of a failed previous run
            if (mapnik::util::exists(filename + ".index"))
            {
                mapnik::util::remove(filename + ".index");
            }

            for (auto create_index : { true, false })
            {
                if (create_index)
                {
                    CHECK(!mapnik::util::exists(filename + ".index"));
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }

                for (auto cache_features : {true, false})
                {
                    // unfortunately when using an index or not
                    // caching features we use the bbox grammar
                    // which is not strict (and would be a perf hit if it were strict).
                    // So this is one known hole where invalid data may silently parse okay
                    // refs https://github.com/mapnik/mapnik/issues/3125
                    if (!create_index && cache_features == true)
                    {
                        std::stringstream msg;
                        msg << "testcase: create index " << create_index << " cache_features " << cache_features;
                        params["cache_features"] = cache_features;
                        INFO(msg.str());
                        CHECK_THROWS(mapnik::datasource_cache::instance().create(params));
                    }
                }

                // cleanup
                if (create_index && mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        }

        SECTION("GeoJSON ensure original feature ordering is preserved")
        {
            mapnik::parameters params;
            params["type"] = "geojson";

            std::string filename("./test/data/json/ordered.json");
            params["file"] = filename;

            // cleanup in the case of a failed previous run
            if (mapnik::util::exists(filename + ".index"))
            {
                mapnik::util::remove(filename + ".index");
            }

            for (auto create_index : { true, false })
            {
                if (create_index)
                {
                    CHECK(!mapnik::util::exists(filename + ".index"));
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }

                for (auto cache_features : {true, false})
                {
                    params["cache_features"] = cache_features;
                    auto ds = mapnik::datasource_cache::instance().create(params);
                    REQUIRE(bool(ds));
                    auto fields = ds->get_descriptor().get_descriptors();
                    mapnik::query query(ds->envelope());
                    for (auto const& field : fields)
                    {
                        query.add_property_name(field.get_name());
                    }
                    auto features = ds->features(query);
                    auto feature = features->next();
                    mapnik::value_integer count = 0;
                    while (feature != nullptr)
                    {
                        // ids are in ascending order, starting from 1
                        mapnik::value val= feature->get("id");
                        REQUIRE(val.get<mapnik::value_integer>() == ++count);
                        feature = features->next();
                    }
                }
                // cleanup
                if (create_index && mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        }

        SECTION("GeoJSON descriptor returns all field names")
        {
            mapnik::parameters params;
            params["type"] = "geojson";

            std::string filename("./test/data/json/featurecollection-multipleprops.geojson");
            params["file"] = filename;

            // cleanup in the case of a failed previous run
            if (mapnik::util::exists(filename + ".index"))
            {
                mapnik::util::remove(filename + ".index");
            }

            for (auto create_index : { true, false })
            {
                if (create_index)
                {
                    CHECK(!mapnik::util::exists(filename + ".index"));
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }

                for (auto cache_features : {true, false})
                {
                    params["cache_features"] = cache_features;
                    auto ds = mapnik::datasource_cache::instance().create(params);
                    REQUIRE(bool(ds));
                    auto fields = ds->get_descriptor().get_descriptors();
                    std::initializer_list<std::string> names = {"one", "two"};
                    REQUIRE_FIELD_NAMES(fields, names);
                }
                // cleanup
                if (create_index && mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        }

        SECTION("GeoJSON properties are properly expressed")
        {
            mapnik::transcoder tr("utf8");
            mapnik::parameters params;
            params["type"] = "geojson";

            std::string filename("./test/data/json/escaped.geojson");
            params["file"] = filename;

            // cleanup in the case of a failed previous run
            if (mapnik::util::exists(filename + ".index"))
            {
                mapnik::util::remove(filename + ".index");
            }

            for (auto create_index : { true, false })
            {
                if (create_index)
                {
                    CHECK(!mapnik::util::exists(filename + ".index"));
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }

                for (auto cache_features : {true, false})
                {
                    params["cache_features"] = cache_features;
                    auto ds = mapnik::datasource_cache::instance().create(params);
                    REQUIRE(bool(ds));
                    auto fields = ds->get_descriptor().get_descriptors();
                    std::initializer_list<std::string> names = {"NOM_FR","array","boolean","description","double","empty_array", "empty_object","int","name","object","spaces"};
                    REQUIRE_FIELD_NAMES(fields, names);

                    auto fs = all_features(ds);
                    std::initializer_list<attr> attrs = {
                        attr{"name", tr.transcode("Test")},
                        attr{"NOM_FR", tr.transcode("Québec")},
                        attr{"boolean", mapnik::value_bool(true)},
                        attr{"description", tr.transcode("Test: \u005C")},
                        attr{"double", mapnik::value_double(1.1)},
                        attr{"int", mapnik::value_integer(1)},
                        attr{"object", tr.transcode("{\"name\":\"waka\",\"spaces\":\"value with spaces\",\"int\":1,\"double\":1.1,\"boolean\":false"
                                                    ",\"NOM_FR\":\"Québec\",\"array\":[\"string\",\"value with spaces\",3,1.1,null,true"
                                                    ",\"Québec\"],\"another_object\":{\"name\":\"nested object\"}}")},
                        attr{"spaces", tr.transcode("this has spaces")},
                        attr{"array", tr.transcode("[\"string\",\"value with spaces\",3,1.1,null,true,"
                                                   "\"Québec\",{\"name\":\"object within an array\"},"
                                                   "[\"array\",\"within\",\"an\",\"array\"]]")},
                        attr{"empty_array", tr.transcode("[]")},
                        attr{"empty_object", tr.transcode("{}")},
                    };
                    auto feature = fs->next();
                    REQUIRE(bool(feature));
                    REQUIRE_ATTRIBUTES(feature, attrs);
                }
                // cleanup
                if (create_index && mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        }
    }
}
