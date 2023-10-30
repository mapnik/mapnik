
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

    double x0, y0, x1, y1;
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

    double x0, y0, x1, y1;
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
    const double r = (1.0 + offset);

    std::vector<double> pos, off_pos;
    const size_t max_i = 1000;
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

    double x0, y0, x1, y1;
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
    const double r = (1.0 + offset);
    const double r2 = (1.0 - offset);

    std::vector<double> pos, off_pos;
    const size_t max_i = 1000;
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

    double x0, y0, x1, y1;
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
        const double offset = 0.2;

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
}
