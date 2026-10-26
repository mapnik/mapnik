#include "catch.hpp"
#include "fake_path.hpp"

// mapnik
#include <mapnik/offset_converter.hpp>
#include <mapnik/util/math.hpp>

// stl
#include <iostream>

namespace offset_test {

static double DELTA_BUFF = 0.5;

double dist(double x0, double y0, double x1, double y1)
{
    double dx = x0 - x1;
    double dy = y0 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

void test_null_segment(double const& offset)
{
    fake_path path = {};
    mapnik::offset_converter<fake_path> off_path_new(path);
    off_path_new.set_offset(offset);
    double x0 = 0;
    double y0 = 0;
    REQUIRE(off_path_new.vertex(&x0, &y0) == mapnik::SEG_END);
    REQUIRE(off_path_new.vertex(&x0, &y0) == mapnik::SEG_END);
    REQUIRE(off_path_new.vertex(&x0, &y0) == mapnik::SEG_END);
}

void test_invalid_segment(double const& offset)
{
    std::vector<double> v_path = {1, 1, 1, 2};
    fake_path path(v_path, true);
    mapnik::offset_converter<fake_path> off_path_new(path);
    off_path_new.set_offset(offset);
    double x0 = 0;
    double y0 = 0;
    REQUIRE(off_path_new.vertex(&x0, &y0) == mapnik::SEG_END);
    REQUIRE(off_path_new.vertex(&x0, &y0) == mapnik::SEG_END);
    REQUIRE(off_path_new.vertex(&x0, &y0) == mapnik::SEG_END);
}

void test_simple_segment(double const& offset)
{
    fake_path path = {0, 0, 1, 0}, off_path = {0, offset, 1, offset};
    mapnik::offset_converter<fake_path> off_path_new(path);
    off_path_new.set_offset(offset);
    double x0 = 0.0;
    double y0 = 0.0;
    double x1 = 0.0;
    double y1 = 0.0;
    unsigned cmd0 = off_path_new.vertex(&x0, &y0);
    unsigned cmd1 = off_path.vertex(&x1, &y1);
    double d = dist(x0, y0, x1, y1);
    while (true)
    {
        if (d > (std::abs(offset) + DELTA_BUFF))
        {
            cmd0 = off_path_new.vertex(&x0, &y0);
            REQUIRE(cmd0 != mapnik::SEG_END);
            d = dist(x0, y0, x1, y1);
            REQUIRE(d <= (std::abs(offset) + DELTA_BUFF));
        }
        else
        {
            REQUIRE(d <= (std::abs(offset) + DELTA_BUFF));
        }

        cmd1 = off_path.vertex(&x1, &y1);
        if (cmd1 == mapnik::SEG_END)
            break;
        d = dist(x0, y0, x1, y1);
        bool done = false;
        while (d <= (std::abs(offset) + DELTA_BUFF))
        {
            CHECK(true);
            cmd0 = off_path_new.vertex(&x0, &y0);
            if (cmd0 == mapnik::SEG_END)
            {
                done = true;
                break;
            }
        }
        if (done)
            break;
    }
}

void test_straight_line(double const& offset)
{
    fake_path path = {0, 0, 1, 0, 9, 0, 10, 0}, off_path = {0, offset, 1, offset, 9, offset, 10, offset};
    mapnik::offset_converter<fake_path> off_path_new(path);
    off_path_new.set_offset(offset);
    double x0 = 0.0;
    double y0 = 0.0;
    double x1 = 0.0;
    double y1 = 0.0;
    unsigned cmd0 = off_path_new.vertex(&x0, &y0);
    [[maybe_unused]] unsigned cmd1 = off_path.vertex(&x1, &y1);
    double d = dist(x0, y0, x1, y1);
    while (true)
    {
        if (d > (std::abs(offset) + DELTA_BUFF))
        {
            cmd0 = off_path_new.vertex(&x0, &y0);
            REQUIRE(cmd0 != mapnik::SEG_END);
            d = dist(x0, y0, x1, y1);
            REQUIRE(d <= (std::abs(offset) + DELTA_BUFF));
        }
        else
        {
            REQUIRE(d <= (std::abs(offset) + DELTA_BUFF));
        }

        cmd1 = off_path.vertex(&x1, &y1);
        d = dist(x0, y0, x1, y1);
        bool done = false;
        while (d <= (std::abs(offset) + DELTA_BUFF))
        {
            CHECK(true);
            cmd0 = off_path_new.vertex(&x0, &y0);
            if (cmd0 == mapnik::SEG_END)
            {
                done = true;
                break;
            }
        }
        if (done)
            break;
    }
}

void test_offset_curve(double const& offset)
{
    double const r = (1.0 + offset);

    std::vector<double> pos, off_pos;
    size_t const max_i = 1000;
    for (size_t i = 0; i <= max_i; ++i)
    {
        double x = mapnik::util::pi * double(i) / max_i;
        pos.push_back(-std::cos(x));
        pos.push_back(std::sin(x));
        off_pos.push_back(-r * std::cos(x));
        off_pos.push_back(r * std::sin(x));
    }

    fake_path path(pos), off_path(off_pos);
    mapnik::offset_converter<fake_path> off_path_new(path);
    off_path_new.set_offset(offset);
    double x0 = 0.0;
    double y0 = 0.0;
    double x1 = 0.0;
    double y1 = 0.0;
    unsigned cmd0 = off_path_new.vertex(&x0, &y0);
    [[maybe_unused]] unsigned cmd1 = off_path.vertex(&x1, &y1);
    double d = dist(x0, y0, x1, y1);
    while (true)
    {
        if (d > (std::abs(offset) + DELTA_BUFF))
        {
            cmd0 = off_path_new.vertex(&x0, &y0);
            REQUIRE(cmd0 != mapnik::SEG_END);
            d = dist(x0, y0, x1, y1);
            REQUIRE(d <= (std::abs(offset) + DELTA_BUFF));
        }
        else
        {
            REQUIRE(d <= (std::abs(offset) + DELTA_BUFF));
        }

        cmd1 = off_path.vertex(&x1, &y1);
        d = dist(x0, y0, x1, y1);
        bool done = false;
        while (d <= (std::abs(offset) + DELTA_BUFF))
        {
            CHECK(true);
            cmd0 = off_path_new.vertex(&x0, &y0);
            if (cmd0 == mapnik::SEG_END)
            {
                done = true;
                break;
            }
        }
        if (done)
            break;
    }
}

void test_s_shaped_curve(double const& offset)
{
    double const r = (1.0 + offset);
    double const r2 = (1.0 - offset);

    std::vector<double> pos, off_pos;
    size_t const max_i = 1000;
    for (size_t i = 0; i <= max_i; ++i)
    {
        double x = mapnik::util::pi * double(i) / max_i;
        pos.push_back(-std::cos(x) - 1);
        pos.push_back(std::sin(x));
        off_pos.push_back(-r * std::cos(x) - 1);
        off_pos.push_back(r * std::sin(x));
    }
    for (size_t i = 0; i <= max_i; ++i)
    {
        double x = mapnik::util::pi * double(i) / max_i;
        pos.push_back(-std::cos(x) + 1);
        pos.push_back(-std::sin(x));
        off_pos.push_back(-r2 * std::cos(x) + 1);
        off_pos.push_back(-r2 * std::sin(x));
    }

    fake_path path(pos), off_path(off_pos);
    mapnik::offset_converter<fake_path> off_path_new(path);
    off_path_new.set_offset(offset);
    double x0 = 0.0;
    double y0 = 0.0;
    double x1 = 0.0;
    double y1 = 0.0;
    unsigned cmd0 = off_path_new.vertex(&x0, &y0);
    [[maybe_unused]] unsigned cmd1 = off_path.vertex(&x1, &y1);
    double d = dist(x0, y0, x1, y1);
    while (true)
    {
        if (d > (std::abs(offset) + DELTA_BUFF))
        {
            cmd0 = off_path_new.vertex(&x0, &y0);
            REQUIRE(cmd0 != mapnik::SEG_END);
            d = dist(x0, y0, x1, y1);
            REQUIRE(d <= (std::abs(offset) + DELTA_BUFF));
        }
        else
        {
            REQUIRE(d <= (std::abs(offset) + DELTA_BUFF));
        }

        cmd1 = off_path.vertex(&x1, &y1);
        d = dist(x0, y0, x1, y1);
        bool done = false;
        while (d <= (std::abs(offset) + DELTA_BUFF))
        {
            CHECK(true);
            cmd0 = off_path_new.vertex(&x0, &y0);
            if (cmd0 == mapnik::SEG_END)
            {
                done = true;
                break;
            }
        }
        if (done)
            break;
    }
}

using points = std::vector<std::pair<double, double>>;

// the outlines (moveto and lineto points of each subpath) of a polygon offset by the converter
std::vector<points> offset_outlines(std::vector<points> const& rings, double offset)
{
    fake_path path = {};
    for (auto const& ring : rings)
    {
        path.vertices_.emplace_back(ring[0].first, ring[0].second, mapnik::SEG_MOVETO);
        for (std::size_t i = 1; i < ring.size(); ++i)
            path.vertices_.emplace_back(ring[i].first, ring[i].second, mapnik::SEG_LINETO);
        path.vertices_.emplace_back(ring[0].first, ring[0].second, mapnik::SEG_CLOSE);
    }
    path.rewind(0);

    mapnik::offset_converter<fake_path> off_path(path);
    off_path.set_offset(offset);

    std::vector<points> outlines;
    unsigned cmd;
    double x, y;
    while ((cmd = off_path.vertex(&x, &y)) != mapnik::SEG_END)
    {
        if (cmd == mapnik::SEG_MOVETO)
            outlines.emplace_back();
        if (cmd == mapnik::SEG_MOVETO || cmd == mapnik::SEG_LINETO)
            outlines.back().emplace_back(x, y);
    }
    return outlines;
}

void check_outline(points const& outline, points const& expected)
{
    REQUIRE(outline.size() == expected.size());
    for (std::size_t i = 0; i < outline.size(); ++i)
    {
        CHECK(outline[i].first == Approx(expected[i].first));
        CHECK(outline[i].second == Approx(expected[i].second));
    }
}

void test_ring_outline(points const& ring, double offset, points const& expected)
{
    auto const outlines = offset_outlines({ring}, offset);
    REQUIRE(outlines.size() == 1);
    check_outline(outlines[0], expected);
}

} // namespace offset_test

TEST_CASE("offset converter")
{
    SECTION("null segment")
    {
        try
        {
            std::vector<double> offsets = {1, -1};
            for (double offset : offsets)
            {
                // test simple straight line segment - should be easy to
                // find the correspondance here.
                offset_test::test_null_segment(offset);
            }
        }
        catch (std::exception const& ex)
        {
            std::cerr << ex.what() << "\n";
            REQUIRE(false);
        }
    }

    SECTION("invalid segment")
    {
        try
        {
            std::vector<double> offsets = {1, -1};
            for (double offset : offsets)
            {
                // test simple straight line segment - should be easy to
                // find the correspondance here.
                offset_test::test_invalid_segment(offset);
            }
        }
        catch (std::exception const& ex)
        {
            std::cerr << ex.what() << "\n";
            REQUIRE(false);
        }
    }

    SECTION("simple segment")
    {
        try
        {
            std::vector<double> offsets = {1, -1};
            for (double offset : offsets)
            {
                // test simple straight line segment - should be easy to
                // find the correspondance here.
                offset_test::test_simple_segment(offset);
            }
        }
        catch (std::exception const& ex)
        {
            std::cerr << ex.what() << "\n";
            REQUIRE(false);
        }
    }

    SECTION("straight line")
    {
        try
        {
            std::vector<double> offsets = {1, -1};
            for (double offset : offsets)
            {
                // test straight line consisting of more than one segment.
                offset_test::test_straight_line(offset);
            }
        }
        catch (std::exception const& ex)
        {
            std::cerr << ex.what() << "\n";
            REQUIRE(false);
        }
    }

    SECTION("curve")
    {
        try
        {
            std::vector<double> offsets = {1, -1};
            for (double offset : offsets)
            {
                offset_test::test_offset_curve(offset);
            }
        }
        catch (std::exception const& ex)
        {
            std::cerr << ex.what() << "\n";
            REQUIRE(false);
        }
    }

    SECTION("s curve")
    {
        try
        {
            std::vector<double> offsets = {1, -1};
            for (double offset : offsets)
            {
                offset_test::test_s_shaped_curve(offset);
            }
        }
        catch (std::exception const& ex)
        {
            std::cerr << ex.what() << "\n";
            REQUIRE(false);
        }
    }

    SECTION("offsect converter does not skip SEG_MOVETO or SEG_CLOSE vertices")
    {
        double const offset = 0.2;

        fake_path path = {};
        path.vertices_.emplace_back(-2, -2, mapnik::SEG_MOVETO);
        path.vertices_.emplace_back(2, -2, mapnik::SEG_LINETO);
        path.vertices_.emplace_back(2, 2, mapnik::SEG_LINETO);
        path.vertices_.emplace_back(-2, 2, mapnik::SEG_LINETO);
        path.vertices_.emplace_back(-2, -1.9, mapnik::SEG_LINETO);
        path.vertices_.emplace_back(0, 0, mapnik::SEG_CLOSE);
        path.vertices_.emplace_back(-1.9, -1.9, mapnik::SEG_MOVETO);
        path.vertices_.emplace_back(1, -1, mapnik::SEG_LINETO);
        path.vertices_.emplace_back(1, 1, mapnik::SEG_LINETO);
        path.vertices_.emplace_back(-1, 1, mapnik::SEG_LINETO);
        path.vertices_.emplace_back(-1, -1, mapnik::SEG_LINETO);
        path.vertices_.emplace_back(0, 0, mapnik::SEG_CLOSE);
        path.rewind(0);

        mapnik::offset_converter<fake_path> off_path(path);
        off_path.set_offset(offset);

        unsigned cmd;
        double x, y;

        unsigned move_to_count = 0;
        unsigned close_count = 0;

        while ((cmd = off_path.vertex(&x, &y)) != mapnik::SEG_END)
        {
            switch (cmd)
            {
                case mapnik::SEG_MOVETO:
                    move_to_count++;
                    break;
                case mapnik::SEG_CLOSE:
                    close_count++;
                    break;
            }
        }

        CHECK(move_to_count == 2);
        CHECK(close_count == 2);
    }

    SECTION("offset converter keeps the outline of a small ring")
    {
        // A ring's closing joint is built around the ring's first vertex, where it touches the segments emitted
        // to and from that vertex. That must not be taken for a curl, which would drop the whole outline.
        offset_test::test_ring_outline({{0, 0}, {4, 0}, {4, 4}, {0, 4}}, 1, {{1, 1}, {3, 1}, {3, 3}, {1, 3}});
    }

    SECTION("offset converter keeps the outline of a small ring closing a rounding error away")
    {
        // The closing edge runs in the -x direction, so the angle of the first vertex's incoming edge is computed
        // as -pi for the first vertex and pi for the closing one, which ends up a rounding error away.
        double const d = 3.5 - 0.5 * std::sqrt(2.0);
        offset_test::test_ring_outline({{0, 0}, {0, 4}, {4, 0}}, -0.5, {{0.5, 0.5}, {0.5, d}, {d, 0.5}});
    }

    SECTION("offset converter keeps the outline of a small ring closing with an outside turn")
    {
        // A rectangle starting on its long side, within a rounding error of straight: the closing joint is an
        // outside turn of nearly zero angle, whose first point is a rounding error away from the first vertex.
        offset_test::test_ring_outline({{163.46596286471714, 11.613331031632043},
                                        {158.80935995325447, 10.926990668000229},
                                        {157.85496848465331, 17.402235789599246},
                                        {169.19064004674553, 19.073009331999771},
                                        {170.14503151534669, 12.597764210400754}},
                                       -1.5,
                                       {{163.2472, 13.0973},
                                        {160.0746, 12.6297},
                                        {159.5577, 16.137},
                                        {167.9254, 17.3703},
                                        {168.4423, 13.863},
                                        {163.2472, 13.0973}});
    }

    SECTION("offset converter keeps the outline of a small hole")
    {
        // Outlined outside the polygon, and so inside its hole. The hole is a ring after the first: the vertex
        // emitted before its first vertex is the outer ring's last, and its own closing joint still touches the
        // segments emitted to and from its first vertex.
        auto const outlines =
          offset_test::offset_outlines({{{0, 0}, {20, 0}, {20, 20}, {0, 20}}, {{6, 6}, {6, 14}, {14, 14}, {14, 6}}},
                                       -1.5);
        REQUIRE(outlines.size() == 2);
        CHECK(outlines[0].size() > 4); // the outer ring's, with rounded corners
        offset_test::check_outline(outlines[1], {{7.5, 7.5}, {7.5, 12.5}, {12.5, 12.5}, {12.5, 7.5}});
    }

    SECTION("offset converter cuts a curl that reaches a ring's closing joint")
    {
        // The narrow end of this ring, just before it closes, makes a curl that ends in the closing joint. Only the
        // segments emitted to and from a ring's first vertex must not be tested against that joint.
        offset_test::test_ring_outline({{10.29, 8.03}, {11.76, 0.7}, {0.2, 0.62}, {0.11, 0.77}},
                                       -1.5,
                                       {{9.274809, 5.463626}, {9.931845, 2.187384}, {4.629396, 2.150689}});
    }
}
