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
#include <cstdlib>
#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/util/fs.hpp>

TEST_CASE("ogr")
{
    std::string geojson_plugin("./plugins/input/ogr.input");
    if (mapnik::util::exists(geojson_plugin))
    {
        SECTION("ogr point feature")
        {
            mapnik::Map m(256, 256);
            mapnik::load_map(m, "./test/data/good_maps/point_json.xml");
            std::string fontdir("fonts/");
            REQUIRE(m.register_fonts(fontdir, true));
            m.zoom_all();
            mapnik::image_rgba8 im(256, 256);
            mapnik::agg_renderer<mapnik::image_rgba8> ren(m, im);
            ren.apply();
            std::string filename("./test/data/images/point_json.png");
            if (std::getenv("UPDATE") != nullptr)
            {
                mapnik::save_to_file(im, filename);
            }
            std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename, "png"));
            mapnik::image_any data = reader->read(0, 0, reader->width(), reader->height());
            mapnik::image_rgba8 expected = mapnik::util::get<mapnik::image_rgba8>(data);
            REQUIRE(mapnik::compare(expected, im) == 0);
        }
    }
}
