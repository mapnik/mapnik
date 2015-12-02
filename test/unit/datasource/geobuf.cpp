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
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/util/fs.hpp>
#include <cstdlib>

#include <boost/optional/optional_io.hpp>

namespace {

std::pair<mapnik::datasource_ptr,mapnik::feature_ptr> fetch_first_feature(std::string const& filename, bool cache_features)
{
    mapnik::parameters params;
    params["type"] = "geobuf";
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
    REQUIRE(features != nullptr);
    auto feature = features->next();
    REQUIRE(feature != nullptr);
    return std::make_pair(ds,feature);
}

int create_disk_index(std::string const& filename, bool silent = true)
{
    std::string cmd;
    if (std::getenv("DYLD_LIBRARY_PATH") != nullptr)
    {
        cmd += std::string("export DYLD_LIBRARY_PATH=") + std::getenv("DYLD_LIBRARY_PATH") + " && ";
    }
    cmd += "mapnik-index " + filename;
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

TEST_CASE("geobuf") {

    std::string geobuf_plugin("./plugins/input/geobuf.input");
    if (mapnik::util::exists(geobuf_plugin))
    {
        SECTION("Geobuf I/O errors")
        {
            std::string filename = "does_not_exist.geobuf";
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
                    params["type"] = "geobuf";
                    params["file"] = filename;
                    params["cache_features"] = cache_features;
                    REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));
                }
            }
        }

        SECTION("geobuf point featurecollection")
        {
            for (auto cache_features : {true, false})
            {
                auto result = fetch_first_feature("./test/data/geobuf/point-fc.geobuf", cache_features);
                auto feature = result.second;
                auto ds = result.first;
                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::Point);
                auto const& pt = mapnik::util::get<mapnik::geometry::point<double> >(geometry);
                REQUIRE(pt.x == 100);
                REQUIRE(pt.y == 0.0);
            }
        }

        SECTION("geobuf point geometry")
        {
            for (auto cache_features : {true, false})
            {
                auto result = fetch_first_feature("./test/data/geobuf/point.geobuf", cache_features);
                auto feature = result.second;
                auto ds = result.first;
                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
                auto const& geometry = feature->get_geometry();
                REQUIRE(mapnik::geometry::geometry_type(geometry) == mapnik::geometry::Point);
                auto const& pt = mapnik::util::get<mapnik::geometry::point<double> >(geometry);
                REQUIRE(pt.x == 100);
                REQUIRE(pt.y == 0.0);
            }
        }

        SECTION("geobuf lineString geometry")
        {
            for (auto cache_features : {true, false})
            {
                auto result = fetch_first_feature("./test/data/geobuf/linestring.geobuf", cache_features);
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

        SECTION("geobuf polygon geometry")
        {
            for (auto cache_features : {true, false})
            {
                auto result = fetch_first_feature("./test/data/geobuf/polygon.geobuf", cache_features);
                auto feature = result.second;
                auto ds = result.first;
                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Polygon);
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

        SECTION("geobuf multipoint geometry")
        {
            for (auto cache_features : {true, false})
            {
                auto result = fetch_first_feature("./test/data/geobuf/multipoint.geobuf", cache_features);
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

        SECTION("geobuf multilinestring geometry")
        {
            for (auto cache_features : {true, false})
            {
                auto result = fetch_first_feature("./test/data/geobuf/multilinestring.geobuf", cache_features);
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

        SECTION("geobuf multipolygon geometry")
        {
            for (auto cache_features : {true, false})
            {
                auto result = fetch_first_feature("./test/data/geobuf/multipolygon.geobuf", cache_features);
                auto feature = result.second;
                auto ds = result.first;
                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Polygon);
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

        SECTION("geobuf geometrycollection geometry")
        {
            std::string filename("./test/data/geobuf/geometrycollection.geobuf");
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

    }
}
