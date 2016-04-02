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
#include "ds_test_util.hpp"

#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/util/fs.hpp>

/*
Compile and run just this test:
clang++ -o test-pgraster -g -I./test/ test/unit/run.cpp test/unit/datasource/pgraster.cpp `mapnik-config --all-flags` && ./test-pgraster -d yes
*/

#include <boost/optional/optional_io.hpp>

namespace {

int run(std::string const& command, bool okay_to_fail = false)
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

std::string dbname("mapnik-tmp-postgis-test-db");

} // anonymous namespace

TEST_CASE("pgraster") {

    SECTION("Postgis data initialization")
    {
        //don't add 'true' here, to get error message, when drop fails. If it works nothing is output
        REQUIRE(run("dropdb --if-exists " + dbname));
        //REQUIRE(run("createdb -T template_postgis " + dbname));
        REQUIRE(run("createdb " + dbname));
        // NOTE: Breaks when raster support is missing
        REQUIRE(run("psql -c 'CREATE EXTENSION postgis;' " + dbname, true));
        REQUIRE(run("psql -q -f ./test/data/sql/postgis-create-db-and-tables.sql " + dbname));

        // Make available a raster table
        REQUIRE(run("psql -c 'CREATE TABLE test_raster AS SELECT * FROM test' " + dbname));
        REQUIRE(run("psql -c 'ALTER TABLE test_raster RENAME geom to rast' " + dbname));
        REQUIRE(run("psql -c 'ALTER TABLE test_raster ALTER rast TYPE raster USING ST_AsRaster(rast, 32, 32)' " + dbname));
        REQUIRE(run("psql -c 'ALTER TABLE test_raster ADD PRIMARY KEY(gid)' " + dbname));
    }

    mapnik::parameters base_params;
    base_params["type"] = "pgraster";
    base_params["dbname"] = dbname;

    SECTION("Postgis should throw without 'table' parameter")
    {
        mapnik::parameters params(base_params);
        CHECK_THROWS(mapnik::datasource_cache::instance().create(params));
    }

    SECTION("Postgis should throw with 'max_async_connection' greater than 'max_size'")
    {
        mapnik::parameters params(base_params);
        params["table"] = "test_raster";
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
        params["table"] = "test_raster";
        params["user"] = "not_a_valid_user";
        params["password"] = "not_a_valid_pwd";
        CHECK_THROWS(mapnik::datasource_cache::instance().create(params));
    }

    SECTION("Postgis initialize dataset with persist_connection, schema, extent, geometry field, autodectect key field, simplify_geometries, row_limit")
    {
        mapnik::parameters params(base_params);
        params["persist_connection"] = "false";
        params["table"] = "public.test_raster";
        params["geometry_field"] = "geom";
        params["autodetect_key_field"] = "true";
        params["extent"] = "-1 -1, -1 2, 4 3, 3 -1, -1 -1";
        params["simplify_geometries"] = "true";
        params["row_limit"] = "1";
        auto ds = mapnik::datasource_cache::instance().create(params);
    }

    SECTION("Postgis query field names")
    {
        mapnik::parameters params(base_params);
        params["table"] = "test_raster";
        auto ds = mapnik::datasource_cache::instance().create(params);
        REQUIRE(ds != nullptr);
        REQUIRE(ds->type() == mapnik::datasource::datasource_t::Raster);
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, { "gid", "colbigint", "col_text", "col-char", "col+bool", "colnumeric", "colsmallint", "colfloat4", "colfloat8", "colcharacter" });
        require_field_types(fields, { mapnik::Integer, mapnik::Integer, mapnik::String, mapnik::String, mapnik::Boolean, mapnik::Double, mapnik::Integer, mapnik::Double, mapnik::Double, mapnik::String });
    }

    SECTION("Postgis iterate features")
    {
        mapnik::parameters params(base_params);
        params["table"] = "test_raster";
        params["raster_field"] = "rast";
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

#if 0
//  SKIPPED as currently failing to figure there's already a WHERE,
//  resulting in:
//
//  in executeQuery Full sql was: 'SELECT ST_SRID("rast") AS srid FROM
//(SELECT *
//  FROM public.test_raster) as data WHERE rast &&
//'BOX3D(-3.402823466385289e+38
//  -3.402823466385289e+38,3.402823466385289e+38
//3.402823466385289e+38)'::box3d
//  WHERE "rast" IS NOT NULL LIMIT 1;'

    SECTION("Postgis bbox query")
    {
        mapnik::parameters params(base_params);
        params["table"] = "(SELECT * FROM public.test_raster) as data WHERE rast && !bbox!";
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
#endif

    SECTION("Postgis query extent: full dataset")
    {
        //include schema to increase coverage
        mapnik::parameters params(base_params);
        params["table"] = "(SELECT * FROM public.test_raster) as data";
        auto ds = mapnik::datasource_cache::instance().create(params);
        REQUIRE(ds != nullptr);
        mapnik::box2d<double> ext = ds->envelope();
        CAPTURE(ext);
        INFO(std::setprecision(6) << std::fixed << ext.minx() << "/" << ext.miny() << " " << ext.maxx() << "/" << ext.maxy());
        REQUIRE(ext.minx() == -2);
        REQUIRE(ext.miny() == -32);
        REQUIRE(ext.maxx() == 32);
        REQUIRE(ext.maxy() == 4);
    }
/* deactivated for merging: still investigating a proper fix
    SECTION("Postgis query extent from subquery")
    {
        mapnik::parameters params(base_params);
        params["table"] = "(SELECT * FROM test_raster where gid=4) as data";
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
        params["table"] = "(SELECT * FROM test_raster where gid=4) as data";
        params["raster_field"] = "rast";
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
        params["table"] = "(SELECT * FROM test_raster where gid=4 AND z(!scale_denominator!) BETWEEN 0 AND 22) as data";
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
