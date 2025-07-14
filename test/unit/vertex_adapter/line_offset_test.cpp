
#include "catch.hpp"
#include "fake_path.hpp"

// mapnik
#include <mapnik/util/math.hpp>
#include <mapnik/vertex_cache.hpp>

// stl
#include <iostream>

double dist(mapnik::pixel_position const& a, mapnik::pixel_position const& b)
{
    mapnik::pixel_position d = a - b;
    return std::sqrt(d.x * d.x + d.y * d.y);
}

void test_simple_segment(double const& offset)
{
    double const dx = 0.01;
    fake_path path = {0, 0, 1, 0}, off_path = {0, offset, 1, offset};
    mapnik::vertex_cache vc(path), off_vc(off_path);

    vc.reset();
    vc.next_subpath();
    off_vc.reset();
    off_vc.next_subpath();

    while (vc.move(dx))
    {
        double pos = vc.linear_position();
        double off_pos = off_vc.position_closest_to(vc.current_position());
        REQUIRE(std::abs(pos - off_pos) < 1.0e-6);
    }
}

void test_straight_line(double const& offset)
{
    double const dx = 0.01;
    fake_path path = {0, 0, 0.1, 0, 0.9, 0, 1, 0}, off_path = {0, offset, 0.4, offset, 0.6, offset, 1, offset};
    mapnik::vertex_cache vc(path), off_vc(off_path);

    vc.reset();
    vc.next_subpath();
    off_vc.reset();
    off_vc.next_subpath();

    while (vc.move(dx))
    {
        double pos = vc.linear_position();
        double off_pos = off_vc.position_closest_to(vc.current_position());
        REQUIRE(std::abs(pos - off_pos) < 1.0e-6);
    }
}

void test_offset_curve(double const& offset)
{
    double const dx = 0.01;
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
    mapnik::vertex_cache vc(path), off_vc(off_path);

    vc.reset();
    vc.next_subpath();
    off_vc.reset();
    off_vc.next_subpath();

    while (vc.move(dx))
    {
        double mpos = vc.linear_position();
        double moff_pos = off_vc.position_closest_to(vc.current_position());
        {
            mapnik::vertex_cache::scoped_state s(off_vc);
            off_vc.move(moff_pos);
            auto eps = (1.001 * offset);
            auto actual = dist(vc.current_position(), off_vc.current_position());
            REQUIRE(actual < eps);
        }
        REQUIRE(std::abs((mpos / vc.length()) - (moff_pos / off_vc.length())) < 1.0e-3);
    }
}

void test_s_shaped_curve(double const& offset)
{
    double const dx = 0.01;
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
    mapnik::vertex_cache vc(path), off_vc(off_path);

    vc.reset();
    vc.next_subpath();
    off_vc.reset();
    off_vc.next_subpath();

    while (vc.move(dx))
    {
        double moff_pos = off_vc.position_closest_to(vc.current_position());
        {
            mapnik::vertex_cache::scoped_state s(off_vc);
            off_vc.move(moff_pos);
            REQUIRE(dist(vc.current_position(), off_vc.current_position()) < (1.002 * offset));
        }
    }
}

TEST_CASE("offsets")
{
    SECTION("line")
    {
        try
        {
            std::vector<double> offsets = {0.01, 0.02, 0.1, 0.2};
            for (double offset : offsets)
            {
                // test simple straight line segment - should be easy to
                // find the correspondance here.
                test_simple_segment(offset);

                // test straight line consisting of more than one segment.
                test_straight_line(offset);

                // test an offset outer curve
                test_offset_curve(offset);

                // test an offset along an S-shaped curve, which is harder
                // because the positions along the offset are no longer
                // linearly related to the positions along the original
                // curve.
                test_s_shaped_curve(offset);
            }
        }
        catch (std::exception const& ex)
        {
            std::cerr << ex.what() << "\n";
            REQUIRE(false);
        }
    }
}
