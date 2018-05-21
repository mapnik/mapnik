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

#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/geometry/geometry_type.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/util/fs.hpp>
#include "../../../plugins/input/postgis/connection_manager.hpp"

/*
  Compile and run just this test:
  clang++ -o test-postgis -g -I./test/ test/unit/run.cpp test/unit/datasource/postgis.cpp `mapnik-config --all-flags` && ./test-postgis -d yes
*/

#include <boost/optional/optional_io.hpp>

namespace {

bool run(std::string const& command, bool okay_to_fail = false)
{
    std::string cmd;
    if (std::getenv("DYLD_LIBRARY_PATH") != nullptr)
    {
        cmd += std::string("DYLD_LIBRARY_PATH=") + std::getenv("DYLD_LIBRARY_PATH") + " && ";
    }
    cmd += command;
    // silence output unless MAPNIK_TEST_DEBUG is defined
    if (std::getenv("MAPNIK_TEST_DEBUG") == nullptr)
    {
#ifndef _WINDOWS
        cmd += " 2>/dev/null";
#else
        cmd += " 2> nul";
#endif
    }
    else
    {
        std::clog << "Running " << cmd << "\n";
    }
    bool worked = (std::system(cmd.c_str()) == 0);
    if (okay_to_fail == true) return true;
    return worked;
}


std::string const dbname("mapnik-tmp-postgis-test-db");
bool status = false;

bool ping_postmaster()
{
    return (run("psql --version")
            && run("dropdb --if-exists " + dbname)
            && run("createdb -T template_postgis " + dbname));
}

}

TEST_CASE("postgis") {

    SECTION("Ping Postmaster (check if server is runnging and accessible")
    {
        if (!ping_postmaster())
        {
            WARN("Can't run postgis.input tests - check postmaster is running and accessible");
            return;
        }
        else
        {
            status = true;
        }
    }
    if (status)
    {
        SECTION("Postgis data initialization")
        {
            //don't add 'true' here, to get error message, when drop fails. If it works nothing is output
            REQUIRE(run("dropdb --if-exists " + dbname));
            REQUIRE(run("createdb -T template_postgis " + dbname));
            //REQUIRE(run("createdb " + dbname));
            // Breaks when raster support is missing (unfortunately this is common)
            //REQUIRE(run("psql -c 'CREATE EXTENSION postgis;' " + dbname, true));
            REQUIRE(run("psql -q -f ./test/data/sql/postgis-create-db-and-tables.sql " + dbname));
        }

        mapnik::parameters base_params;
        base_params["type"] = "postgis";
        base_params["dbname"] = dbname;

        SECTION("Postgis should throw without 'table' parameter")
        {
            mapnik::parameters params(base_params);
            CHECK_THROWS(mapnik::datasource_cache::instance().create(params));
        }

        SECTION("Postgis should throw with 'max_async_connection' greater than 'max_size'")
        {
            mapnik::parameters params(base_params);
            params["table"] = "test";
            params["max_async_connection"] = "2";
            params["max_size"] = "1";
            CHECK_THROWS(mapnik::datasource_cache::instance().create(params));
        }

        SECTION("Postgis should throw with invalid metadata query")
        {
            mapnik::parameters params(base_params);
            params["table"] = "does_not_exist";
            CHECK_THROWS(mapnik::datasource_cache::instance().create(params));
        }

        SECTION("Postgis should throw with invalid key field")
        {
            mapnik::parameters params(base_params);
            params["table"] = "test_invalid_id";
            params["key_field"] = "id";
            CHECK_THROWS(mapnik::datasource_cache::instance().create(params));
        }

        SECTION("Postgis should throw with multicolumn primary key")
        {
            mapnik::parameters params(base_params);
            params["table"] = "test_invalid_multi_col_pk";
            params["autodetect_key_field"] = "true";
            CHECK_THROWS(mapnik::datasource_cache::instance().create(params));
        }

        SECTION("Postgis should throw without geom column")
        {
            mapnik::parameters params(base_params);
            params["table"] = "test_no_geom_col";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(ds != nullptr);
            CHECK_THROWS(all_features(ds));
        }

        SECTION("Postgis should throw with invalid credentials")
        {
            mapnik::parameters params(base_params);
            params["table"] = "test";
            params["user"] = "not_a_valid_user";
            params["password"] = "not_a_valid_pwd";
            CHECK_THROWS(mapnik::datasource_cache::instance().create(params));
        }

        SECTION("Postgis initialize dataset with persist_connection, schema, extent, geometry field, autodectect key field, simplify_geometries, row_limit")
        {
            mapnik::parameters params(base_params);
            params["persist_connection"] = "false";
            params["table"] = "public.test";
            params["geometry_field"] = "geom";
            params["autodetect_key_field"] = "true";
            params["extent"] = "-1 -1, -1 2, 4 3, 3 -1, -1 -1";
            params["simplify_geometries"] = "true";
            params["row_limit"] = "1";
            auto ds = mapnik::datasource_cache::instance().create(params);
        }

        SECTION("Postgis dataset geometry type")
        {
            mapnik::parameters params(base_params);
            params["table"] = "(SELECT * FROM test WHERE gid=1) as data";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(ds != nullptr);
            CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
        }

        SECTION("Postgis properly escapes names with single quotes")
        {
            mapnik::parameters params(base_params);
            params["table"] = "\"test'single'quotes\"";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(ds != nullptr);
            CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
        }

        SECTION("Postgis properly escapes names with double quotes")
        {
            mapnik::parameters params(base_params);
            params["table"] = "\"test\"\"double\"\"quotes\"";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(ds != nullptr);
            CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
        }

        SECTION("Postgis query field names")
        {
            mapnik::parameters params(base_params);
            params["table"] = "test";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(ds != nullptr);
            REQUIRE(ds->type() == mapnik::datasource::datasource_t::Vector);
            auto fields = ds->get_descriptor().get_descriptors();
            require_field_names(fields, { "gid", "colbigint", "col_text", "col-char", "col+bool", "colnumeric", "colsmallint", "colfloat4", "colfloat8", "colcharacter" });
            require_field_types(fields, { mapnik::Integer, mapnik::Integer, mapnik::String, mapnik::String, mapnik::Boolean, mapnik::Double, mapnik::Integer, mapnik::Double, mapnik::Double, mapnik::String });
        }

        SECTION("Postgis iterate features")
        {
            mapnik::parameters params(base_params);
            params["table"] = "test";
            params["key_field"] = "gid";
            params["max_async_connection"] = "2";
            //params["cursor_size"] = "2";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(ds != nullptr);

            auto featureset = ds->features_at_point(mapnik::coord2d(1, 1));
            mapnik::feature_ptr feature;
            while ((bool(feature = featureset->next()))) {
                REQUIRE(feature->get(2).to_string() == feature->get("col_text").to_string());
                REQUIRE(feature->get(4).to_bool() == feature->get("col+bool").to_bool());
                REQUIRE(feature->get(5).to_double() == feature->get("colnumeric").to_double());
                REQUIRE(feature->get(5).to_string() == feature->get("colnumeric").to_string());
            }

            featureset = all_features(ds);
            feature = featureset->next();
            //deactivate char tests for now: not yet implemented.
            //add at postgis_datasource.cpp:423
            //case 18:    // char
            //REQUIRE("A" == feature->get("col-char").to_string());
            feature = featureset->next();
            //REQUIRE("B" == feature->get("col-char").to_string());
            feature = featureset->next();
            REQUIRE(false == feature->get("col+bool").to_bool());
        }

        SECTION("Postgis cursorresultest")
        {
            mapnik::parameters params(base_params);
            params["table"] = "(SELECT * FROM test) as data";
            params["cursor_size"] = "2";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(ds != nullptr);
            auto featureset = all_features(ds);
            CHECK(count_features(featureset) == 8);

            featureset = all_features(ds);
            mapnik::feature_ptr feature;
            while (bool(feature = featureset->next())) {
                CHECK(feature->size() == 10);
            }

            featureset = all_features(ds);
            require_geometry(featureset->next(), 1, mapnik::geometry::geometry_types::Point);
            require_geometry(featureset->next(), 1, mapnik::geometry::geometry_types::Point);
            require_geometry(featureset->next(), 2, mapnik::geometry::geometry_types::MultiPoint);
            require_geometry(featureset->next(), 1, mapnik::geometry::geometry_types::LineString);
            require_geometry(featureset->next(), 2, mapnik::geometry::geometry_types::MultiLineString);
            require_geometry(featureset->next(), 1, mapnik::geometry::geometry_types::Polygon);
            require_geometry(featureset->next(), 2, mapnik::geometry::geometry_types::MultiPolygon);
            require_geometry(featureset->next(), 3, mapnik::geometry::geometry_types::GeometryCollection);
        }

        SECTION("Postgis bbox query")
        {
            mapnik::parameters params(base_params);
            params["table"] = "(SELECT * FROM public.test) as data WHERE geom && !bbox!";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(ds != nullptr);
            mapnik::box2d<double> ext = ds->envelope();
            CAPTURE(ext);
            INFO(std::setprecision(6) << std::fixed << ext.minx() << "/" << ext.miny() << " " << ext.maxx() << "/" << ext.maxy());
            REQUIRE(ext.minx() == -2);
            REQUIRE(ext.miny() == -2);
            REQUIRE(ext.maxx() == 5);
            REQUIRE(ext.maxy() == 4);
        }

        SECTION("Postgis doesn't interpret @domain in email address as @variable")
        {
            mapnik::parameters params(base_params);
            params["table"] = "(SELECT gid, geom, 'fake@mail.ru' as email"
                              " FROM public.test LIMIT 1) AS data";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(ds != nullptr);
            auto featureset = all_features(ds);
            auto feature = featureset->next();
            CHECKED_IF(feature != nullptr)
            {
                CHECK(feature->get("email").to_string() == "fake@mail.ru");
            }
        }

        SECTION("Postgis interpolates !@uservar! tokens in query")
        {
            mapnik::parameters params(base_params);
            params["table"] = "(SELECT * FROM public.test"
                              " WHERE GeometryType(geom) = !@wantedGeomType!"
                              " LIMIT 1) AS data";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(ds != nullptr);

            mapnik::transcoder tr("utf8");
            mapnik::query qry(ds->envelope());
            qry.set_variables({{"wantedGeomType", tr.transcode("POINT")}});
            CHECK(qry.variables().count("wantedGeomType") == 1);

            auto featureset = ds->features(qry);
            auto feature = featureset->next();
            CHECKED_IF(feature != nullptr)
            {
                auto const& geom = feature->get_geometry();
                CHECK(mapnik::geometry::geometry_type(geom) == mapnik::geometry::Point);
            }

            qry.set_variables({{"wantedGeomType", tr.transcode("POLYGON")}});
            CHECK(qry.variables().count("wantedGeomType") == 1);

            featureset = ds->features(qry);
            feature = featureset->next();
            CHECKED_IF(feature != nullptr)
            {
                auto const& geom = feature->get_geometry();
                CHECK(mapnik::geometry::geometry_type(geom) == mapnik::geometry::Polygon);
            }
        }

        SECTION("Postgis query extent: full dataset")
        {
            //include schema to increase coverage
            mapnik::parameters params(base_params);
            params["table"] = "(SELECT * FROM public.test) as data";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(ds != nullptr);
            mapnik::box2d<double> ext = ds->envelope();
            CAPTURE(ext);
            INFO(std::setprecision(6) << std::fixed << ext.minx() << "/" << ext.miny() << " " << ext.maxx() << "/" << ext.maxy());
            REQUIRE(ext.minx() == -2);
            REQUIRE(ext.miny() == -2);
            REQUIRE(ext.maxx() == 5);
            REQUIRE(ext.maxy() == 4);
        }
/* deactivated for merging: still investigating a proper fix
   SECTION("Postgis query extent from subquery")
   {
   mapnik::parameters params(base_params);
   params["table"] = "(SELECT * FROM test where gid=4) as data";
   auto ds = mapnik::datasource_cache::instance().create(params);
   REQUIRE(ds != nullptr);
   mapnik::box2d<double> ext = ds->envelope();
   CAPTURE(ext);
   INFO(std::setprecision(6) << std::fixed << ext.minx() << "/" << ext.miny() << " " << ext.maxx() << "/" << ext.maxy());
   REQUIRE(ext.minx() == 0);
   REQUIRE(ext.miny() == 0);
   REQUIRE(ext.maxx() == 1);
   REQUIRE(ext.maxy() == 2);
   }
*/
        SECTION("Postgis query extent: from subquery with 'extent_from_subquery=true'")
        {
            mapnik::parameters params(base_params);
            params["table"] = "(SELECT * FROM test where gid=4) as data";
            params["extent_from_subquery"] = "true";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(ds != nullptr);
            mapnik::box2d<double> ext = ds->envelope();
            CAPTURE(ext);
            INFO(std::setprecision(6) << std::fixed << ext.minx() << "/" << ext.miny() << " " << ext.maxx() << "/" << ext.maxy());
            REQUIRE(ext.minx() == 0);
            REQUIRE(ext.miny() == 0);
            REQUIRE(ext.maxx() == 1);
            REQUIRE(ext.maxy() == 2);
        }
/* deactivated for merging: still investigating a proper fix
   SECTION("Postgis query extent: subset with 'extent_from_subquery=true' and 'scale_denominator'")
   {
   mapnik::parameters params(base_params);
   // !!!! postgis-vt-util::z() returns 'null' when 'scale_denominator > 600000000'
   // https://github.com/mapbox/postgis-vt-util/blob/559f073877696a6bfea41baf3e1065f9cf4d18d1/postgis-vt-util.sql#L615-L617
   params["table"] = "(SELECT * FROM test where gid=4 AND z(!scale_denominator!) BETWEEN 0 AND 22) as data";
   params["extent_from_subquery"] = "true";
   auto ds = mapnik::datasource_cache::instance().create(params);
   REQUIRE(ds != nullptr);
   mapnik::box2d<double> ext = ds->envelope();
   CAPTURE(ext);
   INFO("" << std::setprecision(6) << std::fixed << ext.minx() << "/" << ext.miny() << " " << ext.maxx() << "/" << ext.maxy());
   REQUIRE(ext.minx() == 0);
   REQUIRE(ext.miny() == 0);
   REQUIRE(ext.maxx() == 1);
   REQUIRE(ext.maxy() == 2);
   }
*/

    }
}


TEST_CASE("ConnectionCreator") {

SECTION("ConnectionCreator::id() should not expose password")
{
    ConnectionCreator<Connection> creator(boost::optional<std::string>("host"),
                                          boost::optional<std::string>("12345"),
                                          boost::optional<std::string>("dbname"),
                                          boost::optional<std::string>("user"),
                                          boost::optional<std::string>("pass"),
                                          boost::optional<std::string>("111"));

    CHECK(creator.id() == "host=host port=12345 dbname=dbname user=user connect_timeout=111");
}

}
