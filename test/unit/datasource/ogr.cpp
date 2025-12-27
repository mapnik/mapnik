/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/datasource_cache.hpp>
#include "../../../plugins/input/gdal+ogr/ogr_utils.hpp"

inline std::vector<ogr_utils::option_ptr> split_options(std::string const& options)
{
    return ogr_utils::split_open_options(options);
}

void assert_option(std::vector<ogr_utils::option_ptr> const& options, size_t const& index, std::string const expected)
{
    auto const* opt = options.at(index).get();
    REQUIRE(opt != nullptr);
    REQUIRE_FALSE(strcmp(opt, expected.c_str()));
}

TEST_CASE("ogr")
{
    bool const have_ogr_plugin = mapnik::datasource_cache::instance().plugin_registered("ogr");
    if (have_ogr_plugin)
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

TEST_CASE("ogr open_options")
{
    bool const have_ogr_plugin = mapnik::datasource_cache::instance().plugin_registered("ogr");
    if (have_ogr_plugin)
    {
        SECTION("splitting open_options string")
        {
            std::string opts = "ZOOM=5";
            std::vector<ogr_utils::option_ptr> v = split_options(opts);
            assert_option(v, 0, opts);
        }
        SECTION("splitting open_options string with multiple components")
        {
            std::string opts = "ZOOM=8 USE_BOUNDS=YES";
            std::vector<ogr_utils::option_ptr> v = split_options(opts);
            assert_option(v, 0, "ZOOM=8");
            assert_option(v, 1, "USE_BOUNDS=YES");
        }
        SECTION("open_options string with escaped character")
        {
            std::string opts = "MY_KEY=THIS\\ VALUE OTHER_KEY=SOMETHING";
            std::vector<ogr_utils::option_ptr> v = split_options(opts);
            assert_option(v, 0, "MY_KEY=THIS VALUE");
            assert_option(v, 1, "OTHER_KEY=SOMETHING");
        }
        SECTION("splitting open_options string with escaping")
        {
            std::string opts = "ZOOM=14 FANCY_OPTION=WITH\\ SPACE_VALUE";
            std::vector<ogr_utils::option_ptr> v = split_options(opts);
            assert_option(v, 0, "ZOOM=14");
            assert_option(v, 1, "FANCY_OPTION=WITH SPACE_VALUE");
        }
        SECTION("splitting open_options string, starts with space")
        {
            std::string opts = " ZOOM=14 K23=V45";
            std::vector<ogr_utils::option_ptr> v = split_options(opts);
            assert_option(v, 0, "ZOOM=14");
            assert_option(v, 1, "K23=V45");
        }
        SECTION("splitting open_options string, ends with space")
        {
            std::string opts = "ZOOM=14 K23=V46 ";
            std::vector<ogr_utils::option_ptr> v = split_options(opts);
            assert_option(v, 0, "ZOOM=14");
            assert_option(v, 1, "K23=V46");
        }
        SECTION("splitting open_options string, doubled space")
        {
            std::string opts = "ZOOM=14  K23=V47";
            std::vector<ogr_utils::option_ptr> v = split_options(opts);
            assert_option(v, 0, "ZOOM=14");
            assert_option(v, 1, "K23=V47");
        }
    }
}
