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
#include <mapnik/memory_datasource.hpp>
#include <mapnik/datasource_cache.hpp>


TEST_CASE("memory datasource") {

    SECTION("empty featureset")
    {
        mapnik::parameters params;
        mapnik::datasource_ptr ds = std::make_shared<mapnik::memory_datasource>(params);
        CHECK(ds != nullptr);
        auto fs = all_features(ds);
        REQUIRE(!mapnik::is_valid(fs));
        while (auto f = fs->next())
        {
            CHECK(false); // shouldn't get here
        }
    }
}
