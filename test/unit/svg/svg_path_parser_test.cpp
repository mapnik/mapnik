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

#include <mapnik/debug.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/svg/svg_path_parser.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/marker.hpp>
#include "util.hpp"

namespace {

template <typename Expected>
void test_path_parser(std::string const& str, Expected const& expected)
{
    using namespace mapnik::svg;
    mapnik::svg_path_ptr marker_path(std::make_shared<mapnik::svg_storage_type>());
    vertex_stl_adapter<svg_path_storage> stl_storage(marker_path->source());
    svg_path_adapter svg_path(stl_storage);
    svg_converter_type svg(svg_path, marker_path->attributes());

    CHECK(mapnik::svg::parse_path(str.c_str(), svg));
    double x,y;
    unsigned cmd;
    auto & p = svg.storage();
    std::vector<std::tuple<double,double,unsigned>> vec;
    while ((cmd = p.vertex(&x,&y)) != mapnik::SEG_END)
    {
        vec.emplace_back(x, y, cmd);
        //std::cerr << "std::make_tuple(" << x << ", " << y << ", " << cmd  << ")," << std::endl;
    }
    REQUIRE(std::equal(expected.begin(),expected.end(), vec.begin(), vertex_equal<3>()));
}
} // anonymous ns

TEST_CASE("SVG path parser") {

    SECTION("MoveTo/LineTo")
    {
        std::string str = "M 100 100 L 300 100 L 200 300 z";
        std::string str2 = "M100,100L300,100L200,300z";
        std::string str3 = "M100,100l200 0L200,300z";
        std::vector<std::tuple<double,double,unsigned>> expected = {
            std::make_tuple(100, 100, 1),
            std::make_tuple(300, 100, 2),
            std::make_tuple(200, 300, 2),
            std::make_tuple(100, 100, 79) };
        test_path_parser(str, expected);
        test_path_parser(str2, expected);
        test_path_parser(str3, expected);
    }

    SECTION("MoveTo/HLine/VLine")
    {
        std::string str = "M100 100H300V200z";
        std::string str2 = "M100,100h200v100z";
        std::vector<std::tuple<double,double,unsigned>> expected = {
            std::make_tuple(100, 100, 1),
            std::make_tuple(300, 100, 2),
            std::make_tuple(300, 200, 2),
            std::make_tuple(100, 100, 79)
        };
        test_path_parser(str, expected);
        test_path_parser(str2, expected);
    }

    SECTION("Arcs")
    {
        std::string str = "M300,200 h-150 a150,150 0 1,0 150,-150 z";

        std::vector<std::tuple<double,double,unsigned>> expected = {
            std::make_tuple(300, 200, 1),
            std::make_tuple(150, 200, 2),
            std::make_tuple(150, 282.843, 4),
            std::make_tuple(217.157, 350, 4),
            std::make_tuple(300, 350, 4),
            std::make_tuple(382.843, 350, 4),
            std::make_tuple(450, 282.843, 4),
            std::make_tuple(450, 200, 4),
            std::make_tuple(450, 117.157, 4),
            std::make_tuple(382.843, 50, 4),
            std::make_tuple(300, 50, 4),
            std::make_tuple(300, 200, 79)};
        test_path_parser(str, expected);
    }


    SECTION("Arcs 2")
    {
        std::string str = "M275,175 v-150 a150,150 0 0,0 -150,150 z";

        std::vector<std::tuple<double,double,unsigned>> expected = {
            std::make_tuple(275, 175, 1),
            std::make_tuple(275, 25, 2),
            std::make_tuple(192.157, 25, 4),
            std::make_tuple(125, 92.1573, 4),
            std::make_tuple(125, 175, 4),
            std::make_tuple(275, 175, 79)};
        test_path_parser(str, expected);
    }

    SECTION("Arcs 3")
    {
        std::string str = "M600,350 l 50,-25"
            "a25,25 -30 0,1 50,-25 l 50,-25"
            "a25,50 -30 01 50,-25 l 50,-25"
            "a25,75 -30 0150,-25 l 50,-25"
            "a25,100-30 0150-25l50-25";

        std::vector<std::tuple<double,double,unsigned>> expected = {
            std::make_tuple(600, 350, 1),
            std::make_tuple(650, 325, 2),
            std::make_tuple(643.096, 311.193, 4),
            std::make_tuple(648.693, 294.404, 4),
            std::make_tuple(662.5, 287.5, 4),
            std::make_tuple(676.307, 280.596, 4),
            std::make_tuple(693.096, 286.193, 4),
            std::make_tuple(700, 300, 4),
            std::make_tuple(750, 275, 2),
            std::make_tuple(734.991, 248.079, 4),
            std::make_tuple(734.017, 220.66, 4),
            std::make_tuple(747.825, 213.756, 4),
            std::make_tuple(761.632, 206.852, 4),
            std::make_tuple(784.991, 223.079, 4),
            std::make_tuple(800, 250, 4),
            std::make_tuple(850, 225, 2),
            std::make_tuple(827.153, 184.812, 4),
            std::make_tuple(819.825, 146.636, 4),
            std::make_tuple(833.632, 139.733, 4),
            std::make_tuple(847.44, 132.829, 4),
            std::make_tuple(877.153, 159.812, 4),
            std::make_tuple(900, 200, 4),
            std::make_tuple(950, 175, 2),
            std::make_tuple(919.382, 121.506, 4),
            std::make_tuple(905.754, 72.5436, 4),
            std::make_tuple(919.561, 65.64, 4),
            std::make_tuple(933.368, 58.7365, 4),
            std::make_tuple(969.382, 96.5057, 4),
            std::make_tuple(1000, 150, 4),
            std::make_tuple(1050, 125, 2)};
        test_path_parser(str, expected);
    }

    SECTION("Quadratic Bézier")
    {
        std::string str = "M200,300 Q400,50 600,300 T1000,300";

        std::vector<std::tuple<double,double,unsigned>> expected = {
            std::make_tuple(200,  300, 1),
            std::make_tuple(400,   50, 3),
            std::make_tuple(600,  300, 3),
            std::make_tuple(800,  550, 3),
            std::make_tuple(1000, 300, 3)};
        test_path_parser(str, expected);
    }

    SECTION("Cubic Bézier")
    {
        std::string str = "M100,200 C100,100 250,100 250,200S400,300 400,200";

        std::vector<std::tuple<double,double,unsigned>> expected = {
            std::make_tuple(100, 200, 1),
            std::make_tuple(100, 100, 4),
            std::make_tuple(250, 100, 4),
            std::make_tuple(250, 200, 4),
            std::make_tuple(250, 300, 4),
            std::make_tuple(400, 300, 4),
            std::make_tuple(400, 200, 4)};

        test_path_parser(str, expected);
    }
}
