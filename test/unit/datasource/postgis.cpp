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

clang++ -o test-postgis -g -I./test/ test/unit/run.cpp test/unit/datasource/postgis.cpp `mapnik-config --all-flags` && ./test-postgis -d yes

*/

namespace {

int run(std::string const& command, bool silent = false)
{
    std::string cmd;
    if (std::getenv("DYLD_LIBRARY_PATH") != nullptr)
    {
        cmd += std::string("export DYLD_LIBRARY_PATH=") + std::getenv("DYLD_LIBRARY_PATH") + " && ";
    }
    cmd += command;
    if (silent)
    {
#ifndef _WINDOWS
        cmd += " 2>/dev/null";
#else
        cmd += " 2> nul";
#endif
    }
    bool worked = (std::system(cmd.c_str()) == 0);
    if (silent == true) return true;
    return worked;
}


TEST_CASE("postgis") {

    SECTION("Postgis data initialization")
    {
        REQUIRE(run("dropdb mapnik-tmp-postgis-test-db",true));
        REQUIRE(run("createdb -T template_postgis mapnik-tmp-postgis-test-db"));
        std::stringstream cmd;
        cmd << "psql -q mapnik-tmp-postgis-test-db -f ./test/data/sql/table1.sql";
        REQUIRE(run(cmd.str()));
    }

    std::string datasource_plugin("./plugins/input/postgis.input");
    if (mapnik::util::exists(datasource_plugin))
    {
        SECTION("Postgis plugin initialization")
        {
            mapnik::parameters params;
            params["type"] = "postgis";
            params["dbname"] = "mapnik-tmp-postgis-test-db";
            params["table"] = "test";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(ds != nullptr);
            CHECK(ds->type() == mapnik::datasource::datasource_t::Vector);
            auto fields = ds->get_descriptor().get_descriptors();
            require_field_names(fields, {"gid"});
            require_field_types(fields, {mapnik::Integer});
        }
    }
}


}