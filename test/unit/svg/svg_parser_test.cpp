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

#include <mapnik/debug.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/svg/svg_parser.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>

#include <cmath>
#include <fstream>
#include <iterator>

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

    SECTION("SVG i/o")
    {
        mapnik::logger::instance().set_severity(mapnik::logger::none);
        std::string svg_name("FAIL");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_null>());
        mapnik::logger::instance().set_severity(mapnik::logger::error);
    }

    SECTION("SVG::parse i/o")
    {
        std::string svg_name("FAIL");

        using namespace mapnik::svg;
        mapnik::svg_storage_type path;
        vertex_stl_adapter<svg_path_storage> stl_storage(path.source());
        svg_path_adapter svg_path(stl_storage);
        svg_converter_type svg(svg_path, path.attributes());
        svg_parser p(svg);

        if (!p.parse(svg_name))
        {
            auto const& errors = p.error_messages();
            REQUIRE(errors.size() == 1);
            REQUIRE(errors[0] ==  "Unable to open 'FAIL'");
        }
    }

    SECTION("SVG::parse_from_string syntax error")
    {
        std::string svg_name("./test/data/svg/invalid.svg");
        std::ifstream in(svg_name.c_str());
        std::string svg_str((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());

        using namespace mapnik::svg;
        mapnik::svg_storage_type path;
        vertex_stl_adapter<svg_path_storage> stl_storage(path.source());
        svg_path_adapter svg_path(stl_storage);
        svg_converter_type svg(svg_path, path.attributes());
        svg_parser p(svg);

        if (!p.parse_from_string(svg_str))
        {
            auto const& errors = p.error_messages();
            REQUIRE(errors.size() == 1);
            REQUIRE(errors[0] ==  "Unable to parse '<?xml version=\"1.0\"?>\n<svg width=\"12cm\" height=\"4cm\" viewBox=\"0 0 1200 400\"\nxmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\">\n'");
        }
    }

    SECTION("SVG::parse_from_string syntax error")
    {
        std::string svg_name("./test/data/svg/invalid.svg");

        using namespace mapnik::svg;
        mapnik::svg_storage_type path;
        vertex_stl_adapter<svg_path_storage> stl_storage(path.source());
        svg_path_adapter svg_path(stl_storage);
        svg_converter_type svg(svg_path, path.attributes());
        svg_parser p(svg);

        if (!p.parse(svg_name))
        {
            auto const& errors = p.error_messages();
            REQUIRE(errors.size() == 1);
            REQUIRE(errors[0] ==  "svg_parser::parse - Unable to parse './test/data/svg/invalid.svg'");
        }
    }

    SECTION("SVG parser color <fail>")
    {

        std::string svg_name("./test/data/svg/color_fail.svg");
        std::ifstream in(svg_name.c_str());
        std::string svg_str((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());

        using namespace mapnik::svg;
        mapnik::svg_storage_type path;
        vertex_stl_adapter<svg_path_storage> stl_storage(path.source());
        svg_path_adapter svg_path(stl_storage);
        svg_converter_type svg(svg_path, path.attributes());
        svg_parser p(svg);

        if (!p.parse_from_string(svg_str))
        {
            auto const& errors = p.error_messages();
            REQUIRE(errors.size() == 3);
            REQUIRE(errors[0] ==  "Failed to parse color: \"fail\"");
            REQUIRE(errors[1] ==  "Failed to parse double: \"fail\"");
            REQUIRE(errors[2] ==  "Failed to parse color: \"fail\"");
        }
    }

    SECTION("SVG - cope with erroneous geometries")
    {
        std::string svg_name("./test/data/svg/errors.svg");
        std::ifstream in(svg_name.c_str());
        std::string svg_str((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());

        using namespace mapnik::svg;
        mapnik::svg_storage_type path;
        vertex_stl_adapter<svg_path_storage> stl_storage(path.source());
        svg_path_adapter svg_path(stl_storage);
        svg_converter_type svg(svg_path, path.attributes());
        svg_parser p(svg);
        if (!p.parse_from_string(svg_str))
        {
            auto const& errors = p.error_messages();
            REQUIRE(errors.size() == 13);
            REQUIRE(errors[0] == "parse_rect: Invalid width");
            REQUIRE(errors[1] == "Failed to parse double: \"FAIL\"");
            REQUIRE(errors[2] == "parse_rect: Invalid height");
            REQUIRE(errors[3] == "parse_rect: Invalid rx");
            REQUIRE(errors[4] == "parse_rect: Invalid ry");
            REQUIRE(errors[5] == "unable to parse invalid svg <path>");
            REQUIRE(errors[6] == "unable to parse invalid svg <path> with id 'fail-path'");
            REQUIRE(errors[7] == "unable to parse invalid svg <path> with id 'fail-path'");
            REQUIRE(errors[8] == "parse_circle: Invalid radius");
            REQUIRE(errors[9] == "Failed to parse <polygon> 'points'");
            REQUIRE(errors[10] == "Failed to parse <polyline> 'points'");
            REQUIRE(errors[11] == "parse_ellipse: Invalid rx");
            REQUIRE(errors[12] == "parse_ellipse: Invalid ry");
        }
    }

    SECTION("SVG parser double % <fail>")
    {

        std::string svg_name("./test/data/svg/gradient-radial-error.svg");
        std::ifstream in(svg_name.c_str());
        std::string svg_str((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());

        using namespace mapnik::svg;
        mapnik::svg_storage_type path;
        vertex_stl_adapter<svg_path_storage> stl_storage(path.source());
        svg_path_adapter svg_path(stl_storage);
        svg_converter_type svg(svg_path, path.attributes());
        svg_parser p(svg);

        if (!p.parse_from_string(svg_str))
        {
            auto const& errors = p.error_messages();
            REQUIRE(errors.size() == 1);
            REQUIRE(errors[0] ==  "Failed to parse double (optional %) from FAIL");
        }
    }

    SECTION("SVG parser display=none")
    {

        std::string svg_name("./test/data/svg/invisible.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(0, 0, 1, 1));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());
        mapnik::svg::svg_path_adapter path(stl_storage);
        double x,y;
        REQUIRE(path.vertex(&x,&y) == mapnik::SEG_END);
    }

    SECTION("SVG parser stroke-linecap=square")
    {

        std::string svg_name("./test/data/svg/stroke-linecap-square.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(5, 60, 220, 60));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());
        mapnik::svg::svg_path_adapter path(stl_storage);

        auto const& attrs = storage->attributes();
        agg::line_cap_e expected_cap(agg::square_cap);
        REQUIRE(attrs.size() == 1 );
        REQUIRE(attrs[0].line_cap == expected_cap);

        double x,y;
        unsigned cmd;
        std::vector<std::tuple<double,double,unsigned>> vec;
        while ((cmd = path.vertex(&x,&y)) != mapnik::SEG_END)
        {
            vec.emplace_back(x, y, cmd);
        }
        std::vector<std::tuple<double,double,unsigned>> expected = { std::make_tuple(5, 60, 1),
                                                                     std::make_tuple(220, 60, 2) };
        REQUIRE(std::equal(expected.begin(),expected.end(), vec.begin()));
    }

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

    SECTION("SVG viewbox fallback")
    {
        std::string svg_name("./test/data/svg/viewbox-missing-width-and-height.svg");
        std::ifstream in(svg_name.c_str());
        std::string svg_str((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());

        using namespace mapnik::svg;
        mapnik::svg_storage_type path;
        vertex_stl_adapter<svg_path_storage> stl_storage(path.source());
        svg_path_adapter svg_path(stl_storage);
        svg_converter_type svg(svg_path, path.attributes());
        svg_parser p(svg);
        p.parse_from_string(svg_str);
        auto width = svg.width();
        auto height = svg.height();
        REQUIRE(width == 100);
        REQUIRE(height == 100);
    }

    SECTION("SVG rounded <rect>s missing rx or ry")
    {
        std::string svg_name("./test/data/svg/rounded_rect-missing-one-radius.svg");
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

    SECTION("SVG beveled <rect>")
    {
        std::string svg_name("./test/data/svg/stroke-linejoin-bevel.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(10, 10, 30, 25));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());

        auto const& attrs = storage->attributes();
        agg::line_join_e expected_join(agg::bevel_join);
        REQUIRE(attrs.size() == 1 );
        REQUIRE(attrs[0].line_join == expected_join);
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

    SECTION("SVG <gradient>")
    {
        //
        std::string svg_name("./test/data/svg/gradient.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(1.0,1.0,799.0,599.0));
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
                                                                    std::make_tuple(799, 1, 2),
                                                                    std::make_tuple(799, 599, 2),
                                                                    std::make_tuple(1, 599, 2),
                                                                    std::make_tuple(1, 1, 79),
                                                                    std::make_tuple(0, 0, 0),
                                                                    std::make_tuple(100, 100, 1),
                                                                    std::make_tuple(700, 100, 2),
                                                                    std::make_tuple(700, 300, 2),
                                                                    std::make_tuple(100, 300, 2),
                                                                    std::make_tuple(100, 100, 79),
                                                                    std::make_tuple(0, 0, 0),
                                                                    std::make_tuple(100, 320, 1),
                                                                    std::make_tuple(700, 320, 2),
                                                                    std::make_tuple(700, 520, 2),
                                                                    std::make_tuple(100, 520, 2),
                                                                    std::make_tuple(100, 320, 79)};

        REQUIRE(std::equal(expected.begin(),expected.end(), vec.begin()));
    }

    SECTION("SVG missing <gradient> def")
    {
        std::string svg_name("./test/data/svg/gradient-nodef.svg");
        using namespace mapnik::svg;
        mapnik::svg_storage_type path;
        vertex_stl_adapter<svg_path_storage> stl_storage(path.source());
        svg_path_adapter svg_path(stl_storage);
        svg_converter_type svg(svg_path, path.attributes());
        svg_parser p(svg);
        REQUIRE(!p.parse(svg_name));
        auto const& errors = p.error_messages();
        REQUIRE(errors.size() == 2);
        REQUIRE(errors[0] == "Failed to find gradient fill: MyGradient");
        REQUIRE(errors[1] == "Failed to find gradient stroke: MyGradient");
    }

    SECTION("SVG missing <gradient> id")
    {

        std::string svg_name("./test/data/svg/gradient-no-id.svg");
        std::ifstream in(svg_name.c_str());
        std::string svg_str((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());

        using namespace mapnik::svg;
        mapnik::svg_storage_type path;
        vertex_stl_adapter<svg_path_storage> stl_storage(path.source());
        svg_path_adapter svg_path(stl_storage);
        svg_converter_type svg(svg_path, path.attributes());
        svg_parser p(svg);

        if (!p.parse_from_string(svg_str))
        {
            auto const& errors = p.error_messages();
            REQUIRE(errors.size() == 2);
            REQUIRE(errors[0] ==  "Failed to find gradient fill: MyGradient");
            REQUIRE(errors[1] ==  "Failed to find gradient stroke: MyGradient");
        }
    }

    SECTION("SVG missing <gradient> inheritance")
    {
        //
        std::string svg_name("./test/data/svg/gradient-inherit.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(1.0,1.0,699.0,199.0));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());

        auto const& attrs = storage->attributes();
        REQUIRE(attrs.size() == 3 );
        REQUIRE(attrs[1].fill_gradient == attrs[2].fill_gradient);

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
                                                                    std::make_tuple(699, 1, 2),
                                                                    std::make_tuple(699, 199, 2),
                                                                    std::make_tuple(1, 199, 2),
                                                                    std::make_tuple(1, 1, 79),
                                                                    std::make_tuple(0, 0, 0),
                                                                    std::make_tuple(100, 50, 1),
                                                                    std::make_tuple(300, 50, 2),
                                                                    std::make_tuple(300, 150, 2),
                                                                    std::make_tuple(100, 150, 2),
                                                                    std::make_tuple(100, 50, 79),
                                                                    std::make_tuple(0, 0, 0),
                                                                    std::make_tuple(400, 50, 1),
                                                                    std::make_tuple(600, 50, 2),
                                                                    std::make_tuple(600, 150, 2),
                                                                    std::make_tuple(400, 150, 2),
                                                                    std::make_tuple(400, 50, 79)};

        REQUIRE(std::equal(expected.begin(),expected.end(), vec.begin()));
    }

    SECTION("SVG <gradient> with transformations")
    {
        //
        std::string svg_name("./test/data/svg/gradient-transform.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(1.0,1.0,799.0,599.0));
        auto storage = svg.get_data();
        REQUIRE(storage);

        auto const& attrs = storage->attributes();
        REQUIRE(attrs.size() == 3 );
        REQUIRE(attrs[1].fill_gradient == attrs[2].fill_gradient);
        REQUIRE(attrs[1].fill_gradient.get_gradient_type() == mapnik::RADIAL);
        agg::trans_affine transform;
        transform *= agg::trans_affine_translation(240,155);
        REQUIRE(attrs[1].fill_gradient.get_transform() == transform);
    }

    SECTION("SVG <gradient> with xlink:href")
    {
        std::string svg_name("./test/data/svg/gradient-xhref.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(20,20,460,230));
        auto storage = svg.get_data();
        REQUIRE(storage);

        auto const& attrs = storage->attributes();
        REQUIRE(attrs.size() == 2 );
        REQUIRE(attrs[0].fill_gradient.get_gradient_type() == mapnik::LINEAR);
        REQUIRE(attrs[1].fill_gradient.get_gradient_type() == mapnik::LINEAR);
        REQUIRE(attrs[1].fill_gradient.has_stop());
    }

    SECTION("SVG <gradient> with radial percents")
    {
        std::string svg_name("./test/data/svg/gradient-radial-percents.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(0,0,200,200));
        auto storage = svg.get_data();
        REQUIRE(storage);

        double x1, x2, y1, y2, r;
        auto const& attrs = storage->attributes();
        REQUIRE(attrs.size() == 1 );
        REQUIRE(attrs[0].fill_gradient.get_gradient_type() == mapnik::RADIAL);
        REQUIRE(attrs[0].fill_gradient.has_stop());
        attrs[0].fill_gradient.get_control_points(x1, y1, x2, y2, r);
        REQUIRE(x1 == 0);
        REQUIRE(y1 == 0.25);
        REQUIRE(x2 == 0.10);
        REQUIRE(y2 == 0.10);
        REQUIRE(r == 0.75);
    }
}
