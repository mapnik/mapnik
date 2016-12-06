/*****************************************************************************
*
* This file is part of Mapnik (c++ mapping toolkit)
*
* Copyright (C) 2016 jsimomaa
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

#include <mapnik/map.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_types.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/util/fs.hpp>
#include <boost/format.hpp>
#include <boost/optional/optional_io.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
#pragma GCC diagnostic pop

#include <iostream>

namespace {
    mapnik::datasource_ptr get_sqlite_ds(std::string const& file_name, std::string const& table = "", bool strict = true, std::string const& base = "")
    {
        mapnik::parameters params;

        params["type"]  = std::string("sqlite");
        params["file"]  = file_name;
        params["geometry_field"] = "geometry";
        params["table"] = table;
        if (!base.empty()) {
            params["base"] = base;
        }
        params["strict"] = mapnik::value_bool(strict);
        auto ds = mapnik::datasource_cache::instance().create(params);
        // require a non-null pointer returned
        REQUIRE(ds != nullptr);
        return ds;
    }
} // anonymous namespace

TEST_CASE("sqlite"){
    std::string sqlite_plugin("./plugins/input/sqlite.input");

    if (mapnik::util::exists(sqlite_plugin)) {
        // make the tests silent since we intentionally test error conditions that are noisy
        auto const severity = mapnik::logger::instance().get_severity();
        mapnik::logger::instance().set_severity(mapnik::logger::none);

        // check the sqlite datasource is loaded
        const std::vector<std::string> plugin_names = mapnik::datasource_cache::instance().plugin_names();
        const bool have_sqlite_plugin =
          std::find(plugin_names.begin(), plugin_names.end(), "sqlite") != plugin_names.end();

        SECTION("SQLite I/O errors")
        {
            std::string filename = "does_not_exist.sqlite";

            for (auto create_index : { true, false }) {
                if (create_index) {
                    int ret       = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    // index wont be created
                    CHECK(!mapnik::util::exists(filename + ".index"));
                }
                mapnik::parameters params;
                params["type"] = "sqlite";
                params["file"] = filename;
                REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));
                params["base"] = "";
                REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));
                params["base"] = "/";
                REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));
            }
        }

        SECTION("sqlite emptydb support")
        {
            for (auto create_index : { false, true }) {
                if (have_sqlite_plugin) {
                    mapnik::datasource_ptr sqlite_ds = get_sqlite_ds("test/data/sqlite/empty.db", "(select * from empty where \"a\"!=\"b\" and !intersects!)");
                    auto fs = all_features(sqlite_ds);
                    REQUIRE(mapnik::is_valid(fs));
                }
            }
        } // END SECTION

#ifdef HAS_SPATIALITE
        SECTION("sqlite spatialite support")
        {
            mapnik::datasource_ptr sqlite_ds = get_sqlite_ds("test/data/sqlite/qgis_spatiallite.sqlite", "(select ST_IsValid(geometry) from lines)");
        }
#endif

        mapnik::logger::instance().set_severity(severity);
    }
} // END TEST CASE
