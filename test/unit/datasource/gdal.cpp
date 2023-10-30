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

#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/raster.hpp>
#include <mapnik/util/fs.hpp>

namespace {

mapnik::datasource_ptr get_gdal_ds(std::string const& file_name, boost::optional<mapnik::value_integer> band)
{
    const bool have_gdal_plugin = mapnik::datasource_cache::instance().plugin_registered("gdal");
    if (!have_gdal_plugin)
    {
        return mapnik::datasource_ptr();
    }

    mapnik::parameters params;
    params["type"] = std::string("gdal");
    params["file"] = file_name;
    if (band)
    {
        params["band"] = *band;
    }

    auto ds = mapnik::datasource_cache::instance().create(params);
    REQUIRE(ds != nullptr);

    return ds;
}

} // anonymous namespace

TEST_CASE("gdal")
{
    SECTION("upsampling")
    {
        std::string dataset = "test/data/tiff/ndvi_256x256_gray32f_tiled.tif";
        mapnik::datasource_ptr ds = get_gdal_ds(dataset, 1);

        if (!ds)
        {
            // GDAL plugin not built.
            return;
        }

        mapnik::box2d<double> envelope = ds->envelope();
        CHECK(envelope.width() == 256);
        CHECK(envelope.height() == 256);

        // Indicate four times bigger resolution.
        mapnik::query::resolution_type resolution(2.0, 2.0);
        mapnik::query query(envelope, resolution, 1.0);

        auto features = ds->features(query);
        auto feature = features->next();

        mapnik::raster_ptr raster = feature->get_raster();
        REQUIRE(raster != nullptr);

        // Check whether result is not scaled up.
        CHECK(raster->data_.width() == 256);
        CHECK(raster->data_.height() == 256);
    }

} // END TEST CASE
