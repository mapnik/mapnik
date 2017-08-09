/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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
#include <mapnik/util/fs.hpp>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <locale>
#include <boost/optional/optional_io.hpp>


TEST_CASE("Geobuf") {

    std::string geobuf_plugin("./plugins/input/geobuf.input");
    if (mapnik::util::exists(geobuf_plugin))
    {
        SECTION("Point")
        {
            //{"type":"Feature","id":1,"geometry":{"type":"Point","coordinates":[102,0.5]},"properties":{"prop0":"value0"}}
            mapnik::parameters params;
            params["type"] = "geobuf";
            params["file"] = "./test/data/geobuf/point.geobuf";
            auto ds = mapnik::datasource_cache::instance().create(params);
            auto fs = all_features(ds);
            auto f = fs->next();
            auto const& geometry = f->get_geometry();
            auto const& pt = mapnik::util::get<mapnik::geometry::point<double> >(geometry);
            REQUIRE(pt.x == 102.0);
            REQUIRE(pt.y == 0.5);
            CHECK(fs->next() == nullptr);
        }

        SECTION("MultiPoint")
        {
            //{"type":"Feature","id":1,"geometry":{"type":"MultiPoint","coordinates":[[100,0],[101,1]]},"properties":{"prop0":"value0"}}
            mapnik::parameters params;
            params["type"] = "geobuf";
            params["file"] = "./test/data/geobuf/multipoint.geobuf";
            auto ds = mapnik::datasource_cache::instance().create(params);
            auto fs = all_features(ds);
            auto f = fs->next();
            auto const& geometry = f->get_geometry();
            auto const& mpt = mapnik::util::get<mapnik::geometry::multi_point<double> >(geometry);
            CHECK(mpt.size() == 2);
            REQUIRE(mpt[0].x == 100.0);
            REQUIRE(mpt[0].y == 0.0);
            REQUIRE(mpt[1].x == 101.0);
            REQUIRE(mpt[1].y == 1.0);
            CHECK(fs->next() == nullptr);
        }

        SECTION("LineString")
        {
            //{"type":"Feature","id":1,"geometry":{"type":"LineString","coordinates":[[102,0],[103,1],[104,0],[105,1]]},"properties":{"prop0":"value0","prop1":0}}
            mapnik::parameters params;
            params["type"] = "geobuf";
            params["file"] = "./test/data/geobuf/linestring.geobuf";
            auto ds = mapnik::datasource_cache::instance().create(params);
            auto fs = all_features(ds);
            auto f = fs->next();
            auto const& geometry = f->get_geometry();
            auto const& line = mapnik::util::get<mapnik::geometry::line_string<double> >(geometry);
            CHECK(line.size() == 4);
            REQUIRE(line[0].x == 102.0);
            REQUIRE(line[0].y == 0);
            REQUIRE(line[1].x == 103.0);
            REQUIRE(line[1].y == 1);
            REQUIRE(line[2].x == 104.0);
            REQUIRE(line[2].y == 0);
            REQUIRE(line[3].x == 105.0);
            REQUIRE(line[3].y == 1);
            CHECK(fs->next() == nullptr);
        }

        SECTION("MultiLineString")
        {
            //{"type":"Feature","id":1,"geometry":{"type":"MultiLineString","coordinates":[[[100,0],[101,1]],[[102,2],[103,3]]]},"properties":{"prop0":"value0","prop1":0}}
            mapnik::parameters params;
            params["type"] = "geobuf";
            params["file"] = "./test/data/geobuf/multilinestring.geobuf";
            auto ds = mapnik::datasource_cache::instance().create(params);
            auto fs = all_features(ds);
            auto f = fs->next();
            auto const& geometry = f->get_geometry();
            auto const& mline = mapnik::util::get<mapnik::geometry::multi_line_string<double> >(geometry);
            CHECK(mline.size() == 2);
            auto const& line1 = mline[0];
            REQUIRE(line1[0].x == 100.0);
            REQUIRE(line1[0].y == 0.0);
            REQUIRE(line1[1].x == 101.0);
            REQUIRE(line1[1].y == 1.0);
            auto const& line2 = mline[1];
            REQUIRE(line2[0].x == 102.0);
            REQUIRE(line2[0].y == 2.0);
            REQUIRE(line2[1].x == 103.0);
            REQUIRE(line2[1].y == 3.0);
            CHECK(fs->next() == nullptr);
        }

        SECTION("Polygon")
        {
            //{"type":"Feature","id":1,"geometry":{"type":"Polygon","coordinates":[[[100,0],[101,0],[101,1],[100,1],[100,0]],[[100.8,0.8],[100.8,0.2],[100.2,0.2],[100.2,0.8],[100.8,0.8]]]},"properties":{"prop0":"value0","prop1":"{\"this\":\"that\"}"}}
            auto files =
                {
                    "./test/data/geobuf/polygon.geobuf",
                    "./test/data/geobuf/standalone-feature.geobuf",
                    "./test/data/geobuf/standalone-geometry.geobuf"
                };

            mapnik::parameters params;
            params["type"] = "geobuf";

            for (auto const& filename : files)
            {
                params["file"] = filename;
                auto ds = mapnik::datasource_cache::instance().create(params);
                auto fs = all_features(ds);
                auto f = fs->next();
                auto const& geometry = f->get_geometry();
                auto const& poly = mapnik::util::get<mapnik::geometry::polygon<double> >(geometry);
                CHECK(poly.size() == 2);
                auto const& exterior = poly[0];
                REQUIRE(exterior[0].x == 100);
                REQUIRE(exterior[0].y == 0);
                REQUIRE(exterior[1].x == 101);
                REQUIRE(exterior[1].y == 0);
                REQUIRE(exterior[2].x == 101);
                REQUIRE(exterior[2].y == 1);
                REQUIRE(exterior[3].x == 100);
                REQUIRE(exterior[3].y == 1);
                REQUIRE(exterior[4].x == 100);
                REQUIRE(exterior[4].y == 0);
                auto const& interior = poly[1];
                REQUIRE(interior[0].x == 100.8);
                REQUIRE(interior[0].y == 0.8);
                REQUIRE(interior[1].x == 100.8);
                REQUIRE(interior[1].y == 0.2);
                REQUIRE(interior[2].x == 100.2);
                REQUIRE(interior[2].y == 0.2);
                REQUIRE(interior[3].x == 100.2);
                REQUIRE(interior[3].y == 0.8);
                REQUIRE(interior[4].x == 100.8);
                REQUIRE(interior[4].y == 0.8);

                CHECK(fs->next() == nullptr);
            }
        }

        SECTION("MultiPolygon")
        {
            //{"type":"Feature","id":1,"geometry":{"type":"MultiPolygon","coordinates":[[[[102,2],[103,2],[103,3],[102,3],[102,2]]],[[[100,0],[101,0],[101,1],[100,1],[100,0]],[[100.2,0.2],[100.2,0.8],[100.8,0.8],[100.8,0.2],[100.2,0.2]]]]},"properties":{"prop0":"value0","prop1":"{\"this\":\"that\"}"}}

            mapnik::parameters params;
            params["type"] = "geobuf";
            params["file"] = "./test/data/geobuf/multipolygon.geobuf";
            auto ds = mapnik::datasource_cache::instance().create(params);
            auto fs = all_features(ds);
            auto f = fs->next();
            auto const& geometry = f->get_geometry();
            auto const& mpoly = mapnik::util::get<mapnik::geometry::multi_polygon<double> >(geometry);
            CHECK(mpoly.size() == 2);
            {
                auto const& poly = mpoly[0];
                auto const& exterior = poly[0];
                REQUIRE(exterior[0].x == 102);
                REQUIRE(exterior[0].y == 2);
                REQUIRE(exterior[1].x == 103);
                REQUIRE(exterior[1].y == 2);
                REQUIRE(exterior[2].x == 103);
                REQUIRE(exterior[2].y == 3);
                REQUIRE(exterior[3].x == 102);
                REQUIRE(exterior[3].y == 3);
                REQUIRE(exterior[4].x == 102);
                REQUIRE(exterior[4].y == 2);
            }

            {
                auto const& poly = mpoly[1];
                auto const& exterior = poly[0];
                REQUIRE(exterior[0].x == 100);
                REQUIRE(exterior[0].y == 0);
                REQUIRE(exterior[1].x == 101);
                REQUIRE(exterior[1].y == 0);
                REQUIRE(exterior[2].x == 101);
                REQUIRE(exterior[2].y == 1);
                REQUIRE(exterior[3].x == 100);
                REQUIRE(exterior[3].y == 1);
                REQUIRE(exterior[4].x == 100);
                REQUIRE(exterior[4].y == 0);
                auto const& interior = poly[1];
                REQUIRE(interior[0].x == 100.2);
                REQUIRE(interior[0].y == 0.2);
                REQUIRE(interior[1].x == 100.2);
                REQUIRE(interior[1].y == 0.8);
                REQUIRE(interior[2].x == 100.8);
                REQUIRE(interior[2].y == 0.8);
                REQUIRE(interior[3].x == 100.8);
                REQUIRE(interior[3].y == 0.2);
                REQUIRE(interior[4].x == 100.2);
                REQUIRE(interior[4].y == 0.2);
            }
            CHECK(fs->next() == nullptr);
        }
        SECTION("GeometryCollection")
        {
            //{"type":"Feature","id":1,"geometry":{"type":"GeometryCollection","geometries":[{"type":"Point","coordinates":[100,0]},{"type":"LineString","coordinates":[[101,0],[102,1]]}]},"properties":{"prop0":"value0","prop1":"{\"this\":\"that\"}"}}
            mapnik::parameters params;
            params["type"] = "geobuf";
            params["file"] = "./test/data/geobuf/geometrycollection.geobuf";
            auto ds = mapnik::datasource_cache::instance().create(params);
            auto fs = all_features(ds);
            auto f = fs->next();
            auto const& geometry = f->get_geometry();
            auto const& collection = mapnik::util::get<mapnik::geometry::geometry_collection<double> >(geometry);
            CHECK(collection.size() == 2);
            auto const& pt = mapnik::util::get<mapnik::geometry::point<double> >(collection[0]);
            REQUIRE(pt.x == 100.0);
            REQUIRE(pt.y == 0.0);
            auto const& line = mapnik::util::get<mapnik::geometry::line_string<double> >(collection[1]);
            REQUIRE(line[0].x == 101.0);
            REQUIRE(line[0].y == 0.0);
            REQUIRE(line[1].x == 102.0);
            REQUIRE(line[1].y == 1);
            CHECK(fs->next() == nullptr);
        }
    }
}
