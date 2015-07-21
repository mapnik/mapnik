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

#include <mapnik/version.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <libxml/parser.h> // for xmlInitParser(), xmlCleanupParser()
#include <cmath>

namespace detail {

template <int N = 6>
struct vertex_equal
{
    template <typename T>
    bool operator() (T const& lhs, T const& rhs) const
    {
        static const double eps = 1.0 / std::pow(10,N);
        return (std::fabs(std::get<0>(lhs) - std::get<0>(rhs)) < eps)
            && (std::fabs(std::get<1>(lhs) - std::get<1>(rhs)) < eps)
            && std::get<2>(lhs) == std::get<2>(rhs);
    }
};
}

TEST_CASE("SVG parser") {

    xmlInitParser();
    SECTION("SVG <rect>")
    {
        //<rect width="20" height="15" style="fill:rgb(0,0,255);stroke-width:1;stroke:rgb(0,0,0)" />
        std::string svg_name("./test/data/svg/rect.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(0, 0, 20, 15));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());
        mapnik::svg::svg_path_adapter path(stl_storage);
        double x,y;
        unsigned cmd;
        std::vector<std::tuple<double,double,unsigned>> vec;
        while ((cmd = path.vertex(&x,&y)) != mapnik::SEG_END)
        {
            vec.emplace_back(x, y, cmd);
            //std::cerr << x << "," << y << " cmd=" << cmd << std::endl;
        }
        std::vector<std::tuple<double,double,unsigned>> expected = { std::make_tuple(0, 0, 1),
                                                                     std::make_tuple(20, 0, 2),
                                                                     std::make_tuple(20, 15, 2),
                                                                     std::make_tuple(0, 15, 2),
                                                                     std::make_tuple(0, 0, 79) };
        REQUIRE(std::equal(expected.begin(),expected.end(), vec.begin()));
    }
    SECTION("SVG rounded <rect>")
    {
        //<rect width="20" height="15" rx="5" ry="10" style="fill:rgb(0,0,255);stroke-width:1;stroke:rgb(0,0,0)" />
        std::string svg_name("./test/data/svg/rounded_rect.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(0, 0, 20, 15));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());
        mapnik::svg::svg_path_adapter path(stl_storage);
        double x,y;
        unsigned cmd;
        std::vector<std::tuple<double,double,unsigned>> vec;

        while ((cmd = path.vertex(&x,&y)) != mapnik::SEG_END)
        {
            vec.emplace_back(x, y, cmd);
        }

        std::vector<std::tuple<double,double,unsigned>> expected = {std::make_tuple(0, 5,1),
                                                                    std::make_tuple(0.481856, 2.85842,2),
                                                                    std::make_tuple(1.83455, 1.12961,2),
                                                                    std::make_tuple(3.79736, 0.146789,2),
                                                                    std::make_tuple(5, 0,2),
                                                                    std::make_tuple(15, 0,2),
                                                                    std::make_tuple(17.1416, 0.481856,2),
                                                                    std::make_tuple(18.8704, 1.83455,2),
                                                                    std::make_tuple(19.8532, 3.79736,2),
                                                                    std::make_tuple(20, 5,2),
                                                                    std::make_tuple(20, 10,2),
                                                                    std::make_tuple(19.5181, 12.1416,2),
                                                                    std::make_tuple(18.1654, 13.8704,2),
                                                                    std::make_tuple(16.2026, 14.8532,2),
                                                                    std::make_tuple(15, 15,2),
                                                                    std::make_tuple(5, 15,2),
                                                                    std::make_tuple(2.85842, 14.5181,2),
                                                                    std::make_tuple(1.12961, 13.1654,2),
                                                                    std::make_tuple(0.146789, 11.2026,2),
                                                                    std::make_tuple(0, 10,2),
                                                                    std::make_tuple(0, 10,95)};

        REQUIRE(std::equal(expected.begin(),expected.end(), vec.begin(),detail::vertex_equal<3>()));
    }

    SECTION("SVG <line>")
    {
        //
        std::string svg_name("./test/data/svg/line.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(1.0,1.0,1199.0,399.0));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());
        mapnik::svg::svg_path_adapter path(stl_storage);
        double x,y;
        unsigned cmd;
        std::vector<std::tuple<double,double,unsigned>> vec;
        std::size_t num_vertices = path.total_vertices();
        //std::cerr << "Num vertices = " << num_vertices << std::endl;
        //std::cerr << "{";
        for (std::size_t i = 0; i < num_vertices; ++i)
        {
            cmd = path.vertex(&x,&y);
            vec.emplace_back(x, y, cmd);
            //if (vec.size() > 1) std::cerr << ",";
            //std::cerr << std::setprecision(6) << "std::make_tuple(" << x << ", " << y << ", " << cmd << ")";
        }
        //std::cerr << "}" << std::endl;

        std::vector<std::tuple<double,double,unsigned>> expected = {std::make_tuple(1, 1, 1),
                                                                    std::make_tuple(1199, 1, 2),
                                                                    std::make_tuple(1199, 399, 2),
                                                                    std::make_tuple(1, 399, 2),
                                                                    std::make_tuple(1, 1, 79),
                                                                    std::make_tuple(0, 0, 0),
                                                                    std::make_tuple(100, 300, 1),
                                                                    std::make_tuple(300, 100, 2),
                                                                    std::make_tuple(0, 0, 0),
                                                                    std::make_tuple(300, 300, 1),
                                                                    std::make_tuple(500, 100, 2),
                                                                    std::make_tuple(0, 0, 0),
                                                                    std::make_tuple(500, 300, 1),
                                                                    std::make_tuple(700, 100, 2),
                                                                    std::make_tuple(0, 0, 0),
                                                                    std::make_tuple(700, 300, 1),
                                                                    std::make_tuple(900, 100, 2),
                                                                    std::make_tuple(0, 0, 0),
                                                                    std::make_tuple(900, 300, 1),
                                                                    std::make_tuple(1100, 100, 2)};

        REQUIRE(std::equal(expected.begin(),expected.end(), vec.begin()));
    }

    SECTION("SVG <polyline>")
    {
        //
        std::string svg_name("./test/data/svg/polyline.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(1.0,1.0,1199.0,399.0));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());
        mapnik::svg::svg_path_adapter path(stl_storage);
        double x,y;
        unsigned cmd;
        std::vector<std::tuple<double,double,unsigned>> vec;
        std::size_t num_vertices = path.total_vertices();
        for (std::size_t i = 0; i < num_vertices; ++i)
        {
            cmd = path.vertex(&x,&y);
            vec.emplace_back(x, y, cmd);
        }

        std::vector<std::tuple<double,double,unsigned>> expected = {std::make_tuple(1, 1, 1),
                                                                    std::make_tuple(1199, 1, 2),
                                                                    std::make_tuple(1199, 399, 2),
                                                                    std::make_tuple(1, 399, 2),
                                                                    std::make_tuple(1, 1, 79),
                                                                    std::make_tuple(0, 0, 0),
                                                                    std::make_tuple(50, 375, 1),
                                                                    std::make_tuple(150, 375, 2),
                                                                    std::make_tuple(150, 325, 2),
                                                                    std::make_tuple(250, 325, 2),
                                                                    std::make_tuple(250, 375, 2),
                                                                    std::make_tuple(350, 375, 2),
                                                                    std::make_tuple(350, 250, 2),
                                                                    std::make_tuple(450, 250, 2),
                                                                    std::make_tuple(450, 375, 2),
                                                                    std::make_tuple(550, 375, 2),
                                                                    std::make_tuple(550, 175, 2),
                                                                    std::make_tuple(650, 175, 2),
                                                                    std::make_tuple(650, 375, 2),
                                                                    std::make_tuple(750, 375, 2),
                                                                    std::make_tuple(750, 100, 2),
                                                                    std::make_tuple(850, 100, 2),
                                                                    std::make_tuple(850, 375, 2),
                                                                    std::make_tuple(950, 375, 2),
                                                                    std::make_tuple(950, 25, 2),
                                                                    std::make_tuple(1050, 25, 2),
                                                                    std::make_tuple(1050, 375, 2),
                                                                    std::make_tuple(1150, 375, 2)};

        REQUIRE(std::equal(expected.begin(),expected.end(), vec.begin()));
    }

    SECTION("SVG <polygon>")
    {
        //
        std::string svg_name("./test/data/svg/polygon.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(1.0,1.0,1199.0,399.0));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());
        mapnik::svg::svg_path_adapter path(stl_storage);
        double x,y;
        unsigned cmd;
        std::vector<std::tuple<double,double,unsigned>> vec;
        std::size_t num_vertices = path.total_vertices();
        for (std::size_t i = 0; i < num_vertices; ++i)
        {
            cmd = path.vertex(&x,&y);
            vec.emplace_back(x, y, cmd);
        }

        std::vector<std::tuple<double,double,unsigned>> expected = {std::make_tuple(1, 1, 1),
                                                                    std::make_tuple(1199, 1, 2),
                                                                    std::make_tuple(1199, 399, 2),
                                                                    std::make_tuple(1, 399, 2),
                                                                    std::make_tuple(1, 1, 79),
                                                                    std::make_tuple(0, 0, 0),
                                                                    std::make_tuple(350, 75, 1),
                                                                    std::make_tuple(379, 161, 2),
                                                                    std::make_tuple(469, 161, 2),
                                                                    std::make_tuple(397, 215, 2),
                                                                    std::make_tuple(423, 301, 2),
                                                                    std::make_tuple(350, 250, 2),
                                                                    std::make_tuple(277, 301, 2),
                                                                    std::make_tuple(303, 215, 2),
                                                                    std::make_tuple(231, 161, 2),
                                                                    std::make_tuple(321, 161, 2),
                                                                    std::make_tuple(350, 75, 79),
                                                                    std::make_tuple(0, 0, 0),
                                                                    std::make_tuple(850, 75, 1),
                                                                    std::make_tuple(958, 137.5, 2),
                                                                    std::make_tuple(958, 262.5, 2),
                                                                    std::make_tuple(850, 325, 2),
                                                                    std::make_tuple(742, 262.6, 2),
                                                                    std::make_tuple(742, 137.5, 2),
                                                                    std::make_tuple(850, 75, 79)};

        REQUIRE(std::equal(expected.begin(),expected.end(), vec.begin()));
    }

    xmlCleanupParser();
}
