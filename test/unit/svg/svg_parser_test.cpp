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

#include <mapnik/debug.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/svg/svg_parser.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/util/fs.hpp>
#include "util.hpp"
#include <fstream>
#include <iterator>
#include <iomanip>
#include <sstream>
// boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace // internal
{

std::ostream& operator<<(std::ostream& os, agg::rgba8 const& c)
{
    os << mapnik::color{c.r, c.g, c.b, c.a};
    return os;
}

std::ostream& operator<<(std::ostream& os, mapnik::gradient const& gr)
{
    double cx, cy, fx, fy, r;
    gr.get_control_points(fx, fy, cx, cy, r);

    os << "<gradient cx=\"" << cx << "\" cy=\"" << cy << "\"" << " fx=\"" << fx << "\" fy=\"" << fy << "\" r=\"" << r
       << "\">\n";
    for (auto const& stop : gr.get_stop_array())
    {
        os << "  <stop offset=\"" << std::get<0>(stop) << "\" color=\"" << std::get<1>(stop) << "\"/>\n";
    }

    os << "</gradient>\n";

    return os;
}

std::ostream& operator<<(std::ostream& os, agg::trans_affine const& tr)
{
    os << "<transform>matrix(" << tr.sx << "," << tr.shy << "," << tr.shx << "," << tr.sy << "," << tr.tx << ","
       << tr.ty << ")</transform>";
    return os;
}

struct group_attribute_visitor
{
    group_attribute_visitor(std::stringstream& ss, unsigned padding)
        : ss_(ss)
        , padding_(padding)
    {}

    void operator()(mapnik::svg::group const& g) const
    {
        padding_ += 2;
        // ss_ << "padding=>" << padding_ << std::endl;
        std::string pad(padding_, ' ');
        ss_ << pad << "<g opacity=\"" << g.opacity << "\">" << std::endl;
        for (auto const& elem : g.elements)
        {
            mapbox::util::apply_visitor(group_attribute_visitor(ss_, padding_), elem);
        }
        ss_ << pad << "</g>" << std::endl;
        padding_ -= 2;
    }
    void operator()(mapnik::svg::path_attributes const& attr) const
    {
        padding_ += 2;
        std::string pad(padding_, ' ');
        ss_ << pad << "<attribute index=\"" << attr.index << "\">" << std::endl;
        ss_ << pad << attr.transform << std::endl;
        if (!attr.fill_gradient.get_stop_array().empty())
            ss_ << pad << attr.fill_gradient;
        if (!attr.stroke_gradient.get_stop_array().empty())
            ss_ << pad << attr.stroke_gradient;

        ss_ << pad << "  <opacity>" << attr.opacity << "</opacity>" << std::endl;
        ss_ << pad << "  <fill>" << attr.fill_color << "</fill>" << std::endl;
        ss_ << pad << "  <fill-opacity>" << attr.fill_opacity << "</fill-opacity>" << std::endl;
        ss_ << pad << "  <stroke>" << attr.stroke_color << "</stroke>" << std::endl;
        ss_ << pad << "  <stroke-width>" << attr.stroke_width << "</stroke-width>" << std::endl;
        ss_ << pad << "  <stroke-opacity>" << attr.stroke_opacity << "</stroke-opacity>" << std::endl;
        ss_ << pad << "</attribute>" << std::endl;
        padding_ -= 2;
    }
    std::stringstream& ss_;
    mutable unsigned padding_;
};

bool check_equal_attributes(std::string const& svg_file, mapnik::svg::group const& g)
{
    unsigned padding = 0;
    std::stringstream ss;
    ss << "<svg opacity=\"" << g.opacity << "\">" << std::endl;
    for (auto const& elem : g.elements)
    {
        mapbox::util::apply_visitor(group_attribute_visitor(ss, padding), elem);
    }
    ss << "</svg>" << std::endl;

    std::string well_known_xml = svg_file + ".xml";
    if (!mapnik::util::exists(well_known_xml))
    {
        std::cerr << "Well-known-xml-filename:" << well_known_xml << std::endl;
        std::cerr << ss.str() << std::endl;
        return false;
    }
    else
    {
        boost::property_tree::ptree t0, t1;
        boost::property_tree::read_xml(well_known_xml, t0, boost::property_tree::xml_parser::trim_whitespace);
        boost::property_tree::read_xml(ss, t1, boost::property_tree::xml_parser::trim_whitespace);

        if (t0 != t1)
        {
            std::cerr << ss.str() << std::endl;
        }
        return (t0 == t1);
    }
}

struct test_parser
{
    mapnik::svg_storage_type path;
    mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage;
    mapnik::svg::svg_path_adapter svg_path;
    mapnik::svg::svg_converter_type svg;
    mapnik::svg::svg_parser p;

    explicit test_parser(bool strict = false)
        : stl_storage(path.source())
        , svg_path(stl_storage)
        , svg(svg_path, path.svg_group())
        , p(svg, strict)
    {}

    mapnik::svg::svg_parser* operator->() { return &p; }
};

template<typename C>
std::string join(C const& container)
{
    std::string result;
    for (auto const& str : container)
    {
        if (!result.empty())
            result += "\n ";
        result += str;
    }
    return result;
}
} // namespace

TEST_CASE("SVG parser")
{
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
        char const* expected_errors[] = {"SVG error: unable to open \"FAIL\""};

        test_parser p;
        try
        {
            p->parse(svg_name);
        }
        catch (std::exception const& ex)
        {
            REQUIRE(ex.what() == join(expected_errors));
        }
    }

    SECTION("SVG::parse_from_string syntax error")
    {
        std::string svg_name("./test/data/svg/invalid.svg");
        char const* expected_errors[] = {
          "SVG error: unable to parse \"<?xml version=\"1.0\"?>\n<svg width=\"12cm\" height=\"4cm\" viewBox=\"0 0 1200 "
          "400\"\nxmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\">\n\""};

        std::ifstream in(svg_name.c_str());
        std::string svg_str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

        test_parser p;
        try
        {
            p->parse_from_string(svg_str);
        }
        catch (std::exception const& ex)
        {
            REQUIRE(ex.what() == join(expected_errors));
        }
    }

    SECTION("SVG::parse_from_string syntax error")
    {
        std::string svg_name("./test/data/svg/invalid.svg");
        char const* expected_errors[] = {"SVG error: unable to parse \"./test/data/svg/invalid.svg\""};

        test_parser p;
        try
        {
            p->parse(svg_name);
        }
        catch (std::exception const& ex)
        {
            REQUIRE(ex.what() == join(expected_errors));
        }
    }

    SECTION("SVG parser color <fail>")
    {
        std::string svg_name("./test/data/svg/color_fail.svg");
        char const* expected_errors[] = {
          "SVG parse error: can't infer valid image dimensions from width:\"100%\" height:\"100%\"",
          "SVG parse error: failed to parse <color> with value \"fail\"",
          "SVG parse error: failed to parse <number> with value \"fail\""};

        std::ifstream in(svg_name.c_str());
        std::string svg_str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

        {
            test_parser p;
            p->parse_from_string(svg_str);
            REQUIRE(join(p->err_handler().error_messages()) == join(expected_errors));
        }
        {
            test_parser p(true);
            try
            {
                p->parse_from_string(svg_str);
            }
            catch (std::exception const& ex)
            {
                REQUIRE(ex.what() == std::string(expected_errors[0]));
            }
        }
    }

    SECTION("SVG - cope with erroneous geometries")
    {
        std::string svg_name("./test/data/svg/errors.svg");
        char const* expected_errors[] = {
          "SVG parse error: can't infer valid image dimensions from width:\"100%\" height:\"100%\"",
          "SVG validation error: invalid <rect> width \"-100\"",
          "SVG parse error: failed to parse <number> with value \"FAIL\"",
          "SVG validation error: invalid <rect> height \"-100\"",
          "SVG validation error: invalid <rect> rx \"-1000\"",
          "SVG validation error: invalid <rect> ry \"-1000\"",
          "SVG parse error: failed to parse <number> with value \"100invalidunit\"",
          "SVG parse error: failed to parse <path>",
          "SVG parse error: failed to parse <path> with <id> \"fail-path\"",
          "SVG validation error: invalid <circle> radius \"-50\"",
          "SVG parse error: failed to parse <polygon> points",
          "SVG parse error: failed to parse <polyline> points",
          "SVG validation error: invalid <ellipse> rx \"-10\"",
          "SVG validation error: invalid <ellipse> ry \"-10\""};

        std::ifstream in(svg_name.c_str());
        std::string svg_str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

        {
            test_parser p;
            p->parse_from_string(svg_str);
            REQUIRE(join(p->err_handler().error_messages()) == join(expected_errors));
        }

        {
            // strict
            test_parser p(true);
            try
            {
                p->parse_from_string(svg_str);
            }
            catch (std::exception const& ex)
            {
                REQUIRE(ex.what() == std::string(expected_errors[0]));
            }
        }
    }

    SECTION("SVG parser double % <fail>")
    {
        std::string svg_name("./test/data/svg/gradient-radial-error.svg");
        char const* expected_errors[] = {"SVG parse error: failed to parse <number> with value \"FAIL\""};

        std::ifstream in(svg_name.c_str());
        std::string svg_str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

        {
            test_parser p;
            p->parse_from_string(svg_str);
            REQUIRE(join(p->err_handler().error_messages()) == join(expected_errors));
        }
        {
            test_parser p(true);
            try
            {
                p->parse_from_string(svg_str);
            }
            catch (std::exception const& ex)
            {
                REQUIRE(ex.what() == std::string(expected_errors[0]));
            }
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
        double x, y;
        REQUIRE(path.vertex(&x, &y) == mapnik::SEG_END);
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

        auto const& group_attrs = storage->svg_group();

        REQUIRE(check_equal_attributes(svg_name, group_attrs));

        double x, y;
        unsigned cmd;
        std::vector<std::tuple<double, double, unsigned>> vec;
        while ((cmd = path.vertex(&x, &y)) != mapnik::SEG_END)
        {
            vec.emplace_back(x, y, cmd);
        }
        std::vector<std::tuple<double, double, unsigned>> expected = {std::make_tuple(5, 60, 1),
                                                                      std::make_tuple(220, 60, 2)};
        REQUIRE(std::equal(expected.begin(), expected.end(), vec.begin()));
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
        double x, y;
        unsigned cmd;
        std::vector<std::tuple<double, double, unsigned>> vec;
        while ((cmd = path.vertex(&x, &y)) != mapnik::SEG_END)
        {
            vec.emplace_back(x, y, cmd);
            // std::cerr << x << "," << y << " cmd=" << cmd << std::endl;
        }
        std::vector<std::tuple<double, double, unsigned>> expected = {std::make_tuple(0, 0, 1),
                                                                      std::make_tuple(20, 0, 2),
                                                                      std::make_tuple(20, 15, 2),
                                                                      std::make_tuple(0, 15, 2),
                                                                      std::make_tuple(0, 0, 79)};
        REQUIRE(std::equal(expected.begin(), expected.end(), vec.begin()));
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
        double x, y;
        unsigned cmd;
        std::vector<std::tuple<double, double, unsigned>> vec;

        while ((cmd = path.vertex(&x, &y)) != mapnik::SEG_END)
        {
            vec.emplace_back(x, y, cmd);
        }

        std::vector<std::tuple<double, double, unsigned>> expected = {std::make_tuple(0, 5, 1),
                                                                      std::make_tuple(0.481856, 2.85842, 2),
                                                                      std::make_tuple(1.83455, 1.12961, 2),
                                                                      std::make_tuple(3.79736, 0.146789, 2),
                                                                      std::make_tuple(5, 0, 2),
                                                                      std::make_tuple(15, 0, 2),
                                                                      std::make_tuple(17.1416, 0.481856, 2),
                                                                      std::make_tuple(18.8704, 1.83455, 2),
                                                                      std::make_tuple(19.8532, 3.79736, 2),
                                                                      std::make_tuple(20, 5, 2),
                                                                      std::make_tuple(20, 10, 2),
                                                                      std::make_tuple(19.5181, 12.1416, 2),
                                                                      std::make_tuple(18.1654, 13.8704, 2),
                                                                      std::make_tuple(16.2026, 14.8532, 2),
                                                                      std::make_tuple(15, 15, 2),
                                                                      std::make_tuple(5, 15, 2),
                                                                      std::make_tuple(2.85842, 14.5181, 2),
                                                                      std::make_tuple(1.12961, 13.1654, 2),
                                                                      std::make_tuple(0.146789, 11.2026, 2),
                                                                      std::make_tuple(0, 10, 2),
                                                                      std::make_tuple(0, 10, 95)};

        REQUIRE(std::equal(expected.begin(), expected.end(), vec.begin(), vertex_equal<3>()));
    }

    SECTION("SVG viewbox fallback")
    {
        std::string svg_name("./test/data/svg/viewbox-missing-width-and-height.svg");
        std::ifstream in(svg_name.c_str());
        std::string svg_str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        test_parser p;
        p->parse_from_string(svg_str);
        auto width = p.svg.width();
        auto height = p.svg.height();
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
        double x, y;
        unsigned cmd;
        std::vector<std::tuple<double, double, unsigned>> vec;

        while ((cmd = path.vertex(&x, &y)) != mapnik::SEG_END)
        {
            vec.emplace_back(x, y, cmd);
        }
        std::vector<std::tuple<double, double, unsigned>> expected = {std::make_tuple(0, 5, 1),
                                                                      std::make_tuple(0.481856, 2.85842, 2),
                                                                      std::make_tuple(1.83455, 1.12961, 2),
                                                                      std::make_tuple(3.79736, 0.146789, 2),
                                                                      std::make_tuple(5, 0, 2),
                                                                      std::make_tuple(15, 0, 2),
                                                                      std::make_tuple(17.1416, 0.481856, 2),
                                                                      std::make_tuple(18.8704, 1.83455, 2),
                                                                      std::make_tuple(19.8532, 3.79736, 2),
                                                                      std::make_tuple(20, 5, 2),
                                                                      std::make_tuple(20, 10, 2),
                                                                      std::make_tuple(19.5181, 12.1416, 2),
                                                                      std::make_tuple(18.1654, 13.8704, 2),
                                                                      std::make_tuple(16.2026, 14.8532, 2),
                                                                      std::make_tuple(15, 15, 2),
                                                                      std::make_tuple(5, 15, 2),
                                                                      std::make_tuple(2.85842, 14.5181, 2),
                                                                      std::make_tuple(1.12961, 13.1654, 2),
                                                                      std::make_tuple(0.146789, 11.2026, 2),
                                                                      std::make_tuple(0, 10, 2),
                                                                      std::make_tuple(0, 10, 95)};

        REQUIRE(std::equal(expected.begin(), expected.end(), vec.begin(), vertex_equal<3>()));
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
        auto const& group_attrs = storage->svg_group();
        REQUIRE(check_equal_attributes(svg_name, group_attrs));
    }

    SECTION("SVG <line>")
    {
        std::string svg_name("./test/data/svg/line.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(
          bbox ==
          mapnik::box2d<double>(0.3779527559055118, 0.3779527559055118, 453.1653543307086807, 150.8031496062992005));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());
        mapnik::svg::svg_path_adapter path(stl_storage);
        double x, y;
        unsigned cmd;
        std::vector<std::tuple<double, double, unsigned>> vec;
        std::size_t num_vertices = path.total_vertices();
        for (std::size_t i = 0; i < num_vertices; ++i)
        {
            cmd = path.vertex(&x, &y);
            vec.emplace_back(x, y, cmd);
        }
        std::vector<std::tuple<double, double, unsigned>> expected = {
          std::make_tuple(1, 1, 1),     std::make_tuple(1199, 1, 2),  std::make_tuple(1199, 399, 2),
          std::make_tuple(1, 399, 2),   std::make_tuple(1, 1, 79),    std::make_tuple(0, 0, 0),
          std::make_tuple(100, 300, 1), std::make_tuple(300, 100, 2), std::make_tuple(0, 0, 0),
          std::make_tuple(300, 300, 1), std::make_tuple(500, 100, 2), std::make_tuple(0, 0, 0),
          std::make_tuple(500, 300, 1), std::make_tuple(700, 100, 2), std::make_tuple(0, 0, 0),
          std::make_tuple(700, 300, 1), std::make_tuple(900, 100, 2), std::make_tuple(0, 0, 0),
          std::make_tuple(900, 300, 1), std::make_tuple(1100, 100, 2)};

        REQUIRE(std::equal(expected.begin(), expected.end(), vec.begin()));
    }

    SECTION("SVG <polyline>")
    {
        std::string svg_name("./test/data/svg/polyline.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(
          bbox ==
          mapnik::box2d<double>(0.3779527559055118, 0.3779527559055118, 453.1653543307086807, 150.8031496062992005));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());
        mapnik::svg::svg_path_adapter path(stl_storage);
        double x, y;
        unsigned cmd;
        std::vector<std::tuple<double, double, unsigned>> vec;
        std::size_t num_vertices = path.total_vertices();
        for (std::size_t i = 0; i < num_vertices; ++i)
        {
            cmd = path.vertex(&x, &y);
            vec.emplace_back(x, y, cmd);
        }

        std::vector<std::tuple<double, double, unsigned>> expected = {
          std::make_tuple(1, 1, 1),     std::make_tuple(1199, 1, 2),  std::make_tuple(1199, 399, 2),
          std::make_tuple(1, 399, 2),   std::make_tuple(1, 1, 79),    std::make_tuple(0, 0, 0),
          std::make_tuple(50, 375, 1),  std::make_tuple(150, 375, 2), std::make_tuple(150, 325, 2),
          std::make_tuple(250, 325, 2), std::make_tuple(250, 375, 2), std::make_tuple(350, 375, 2),
          std::make_tuple(350, 250, 2), std::make_tuple(450, 250, 2), std::make_tuple(450, 375, 2),
          std::make_tuple(550, 375, 2), std::make_tuple(550, 175, 2), std::make_tuple(650, 175, 2),
          std::make_tuple(650, 375, 2), std::make_tuple(750, 375, 2), std::make_tuple(750, 100, 2),
          std::make_tuple(850, 100, 2), std::make_tuple(850, 375, 2), std::make_tuple(950, 375, 2),
          std::make_tuple(950, 25, 2),  std::make_tuple(1050, 25, 2), std::make_tuple(1050, 375, 2),
          std::make_tuple(1150, 375, 2)};

        REQUIRE(std::equal(expected.begin(), expected.end(), vec.begin()));
    }

    SECTION("SVG <polygon>")
    {
        std::string svg_name("./test/data/svg/polygon.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(
          bbox ==
          mapnik::box2d<double>(0.3779527559055118, 0.3779527559055118, 453.1653543307086807, 150.8031496062992005));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());
        mapnik::svg::svg_path_adapter path(stl_storage);
        double x, y;
        unsigned cmd;
        std::vector<std::tuple<double, double, unsigned>> vec;
        std::size_t num_vertices = path.total_vertices();
        for (std::size_t i = 0; i < num_vertices; ++i)
        {
            cmd = path.vertex(&x, &y);
            vec.emplace_back(x, y, cmd);
        }

        std::vector<std::tuple<double, double, unsigned>> expected = {
          std::make_tuple(1, 1, 1),     std::make_tuple(1199, 1, 2),    std::make_tuple(1199, 399, 2),
          std::make_tuple(1, 399, 2),   std::make_tuple(1, 1, 79),      std::make_tuple(0, 0, 0),
          std::make_tuple(350, 75, 1),  std::make_tuple(379, 161, 2),   std::make_tuple(469, 161, 2),
          std::make_tuple(397, 215, 2), std::make_tuple(423, 301, 2),   std::make_tuple(350, 250, 2),
          std::make_tuple(277, 301, 2), std::make_tuple(303, 215, 2),   std::make_tuple(231, 161, 2),
          std::make_tuple(321, 161, 2), std::make_tuple(350, 75, 79),   std::make_tuple(0, 0, 0),
          std::make_tuple(850, 75, 1),  std::make_tuple(958, 137.5, 2), std::make_tuple(958, 262.5, 2),
          std::make_tuple(850, 325, 2), std::make_tuple(742, 262.6, 2), std::make_tuple(742, 137.5, 2),
          std::make_tuple(850, 75, 79)};

        REQUIRE(std::equal(expected.begin(), expected.end(), vec.begin()));
    }

    SECTION("SVG <gradient>")
    {
        std::string svg_name("./test/data/svg/gradient.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();

        REQUIRE(
          bbox ==
          mapnik::box2d<double>(75.7795275590551114, 0.1889763779527559, 226.5826771653543119, 113.1968503937007853));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());
        mapnik::svg::svg_path_adapter path(stl_storage);
        double x, y;
        unsigned cmd;
        std::vector<std::tuple<double, double, unsigned>> vec;
        std::size_t num_vertices = path.total_vertices();

        for (std::size_t i = 0; i < num_vertices; ++i)
        {
            cmd = path.vertex(&x, &y);
            vec.emplace_back(x, y, cmd);
        }

        std::vector<std::tuple<double, double, unsigned>> expected = {std::make_tuple(1, 1, 1),
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

        REQUIRE(std::equal(expected.begin(), expected.end(), vec.begin()));
    }

    SECTION("SVG missing <gradient> def")
    {
        std::string svg_name("./test/data/svg/gradient-nodef.svg");
        char const* expected_errors[] = {"SVG parse error: failed to locate fill with <id> \"MyGradient\"",
                                         "SVG parse error: failed to locate stroke with <id> \"MyGradient\""};
        {
            test_parser p;
            p->parse(svg_name);
            REQUIRE(join(p->err_handler().error_messages()) == join(expected_errors));
        }
        {
            test_parser p(true);
            try
            {
                p->parse(svg_name);
            }
            catch (std::exception const& ex)
            {
                REQUIRE(ex.what() == std::string(expected_errors[0]));
            }
        }
    }

    SECTION("SVG missing <gradient> id")
    {
        std::string svg_name("./test/data/svg/gradient-no-id.svg");
        char const* expected_errors[] = {"SVG parse error: failed to locate fill with <id> \"MyGradient\"",
                                         "SVG parse error: failed to locate stroke with <id> \"MyGradient\""};

        std::ifstream in(svg_name.c_str());
        std::string svg_str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

        {
            test_parser p;
            p->parse_from_string(svg_str);
            REQUIRE(join(p->err_handler().error_messages()) == join(expected_errors));
        }
        {
            test_parser p(true);
            try
            {
                p->parse_from_string(svg_str);
            }
            catch (std::exception const& ex)
            {
                REQUIRE(ex.what() == std::string(expected_errors[0]));
            }
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

        REQUIRE(
          bbox ==
          mapnik::box2d<double>(0.3779527559055118, 0.3779527559055118, 264.1889763779527698, 75.2125984251968447));
        auto storage = svg.get_data();
        REQUIRE(storage);
        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(storage->source());

        auto const& group_attrs = storage->svg_group();

        REQUIRE(check_equal_attributes(svg_name, group_attrs));
        mapnik::svg::svg_path_adapter path(stl_storage);
        double x, y;
        unsigned cmd;
        std::vector<std::tuple<double, double, unsigned>> vec;
        std::size_t num_vertices = path.total_vertices();
        for (std::size_t i = 0; i < num_vertices; ++i)
        {
            cmd = path.vertex(&x, &y);
            vec.emplace_back(x, y, cmd);
        }

        std::vector<std::tuple<double, double, unsigned>> expected = {std::make_tuple(1, 1, 1),
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

        REQUIRE(std::equal(expected.begin(), expected.end(), vec.begin()));
    }

    SECTION("SVG <gradient> with transformations")
    {
        std::string svg_name("./test/data/svg/gradient-transform.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(
          bbox ==
          mapnik::box2d<double>(75.7795275590551114, 0.1889763779527559, 226.5826771653543119, 113.1968503937007853));
        auto storage = svg.get_data();
        REQUIRE(storage);

        auto const& group_attrs = storage->svg_group();
        REQUIRE(check_equal_attributes(svg_name, group_attrs));
    }

    SECTION("SVG <gradient> with xlink:href")
    {
        std::string svg_name("./test/data/svg/gradient-xhref.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(20, 20, 460, 230));
        auto storage = svg.get_data();
        REQUIRE(storage);

        auto const& group_attrs = storage->svg_group();
        REQUIRE(check_equal_attributes(svg_name, group_attrs));
    }

    SECTION("SVG <gradient> with radial percents")
    {
        std::string svg_name("./test/data/svg/gradient-radial-percents.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
        auto bbox = svg.bounding_box();
        REQUIRE(bbox == mapnik::box2d<double>(0, 0, 200, 200));
        auto storage = svg.get_data();
        REQUIRE(storage);
        REQUIRE(check_equal_attributes(svg_name, storage->svg_group()));
    }

    SECTION("SVG <clipPath>")
    {
        std::string svg_name("./test/data/svg/clippath.svg");
        std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
        REQUIRE(marker);
        REQUIRE(marker->is<mapnik::marker_svg>());
        mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);

        // Check whether the clipPath doesn't add to the bounding box.
        auto bbox = svg.bounding_box();
        CHECK(bbox.width() == Approx(100));
        CHECK(bbox.height() == Approx(100));
    }
}
