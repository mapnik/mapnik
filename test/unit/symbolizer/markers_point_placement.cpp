
#include "catch.hpp"

#include <mapnik/vertex_adapters.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/markers_placements/point.hpp>

using namespace mapnik;

TEST_CASE("marker placement point")
{
    SECTION("empty geometry")
    {
        mapnik::geometry::line_string<double> g;
        using va_type = mapnik::geometry::line_string_vertex_adapter<double>;
        va_type va(g);

        using detector_type = mapnik::label_collision_detector4;
        detector_type detector(mapnik::box2d<double>(0, 0, 100, 100));

        using placement_type = mapnik::markers_point_placement<va_type, detector_type>;

        mapnik::markers_placement_params params{mapnik::box2d<double>(0, 0, 10, 10),
                                                agg::trans_affine(),
                                                0,
                                                NAN,
                                                0,
                                                false,
                                                false,
                                                DIRECTION_AUTO,
                                                1.0};

        placement_type placement(va, detector, params);

        double x, y, angle;
        CHECK(!placement.get_point(x, y, angle, true));
    }

    SECTION("point")
    {
        mapnik::geometry::point<double> g(2.0, 3.0);
        using va_type = mapnik::geometry::point_vertex_adapter<double>;
        va_type va(g);

        using detector_type = mapnik::label_collision_detector4;
        detector_type detector(mapnik::box2d<double>(0, 0, 100, 100));

        using placement_type = mapnik::markers_point_placement<va_type, detector_type>;

        mapnik::markers_placement_params params{mapnik::box2d<double>(0, 0, 10, 10),
                                                agg::trans_affine(),
                                                0,
                                                NAN,
                                                0,
                                                false,
                                                false,
                                                DIRECTION_AUTO,
                                                1.0};

        placement_type placement(va, detector, params);

        double x, y, angle;

        CHECK(placement.get_point(x, y, angle, true));
        CHECK(x == Approx(2.0));
        CHECK(y == Approx(3.0));
        CHECK(angle == Approx(0.0));

        CHECK(!placement.get_point(x, y, angle, true));
    }

    SECTION("line string")
    {
        mapnik::geometry::line_string<double> g;
        g.emplace_back(1.0, 1.0);
        g.emplace_back(2.0, 2.0);
        using va_type = mapnik::geometry::line_string_vertex_adapter<double>;
        va_type va(g);

        using detector_type = mapnik::label_collision_detector4;
        detector_type detector(mapnik::box2d<double>(0, 0, 100, 100));

        using placement_type = mapnik::markers_point_placement<va_type, detector_type>;

        mapnik::markers_placement_params params{mapnik::box2d<double>(0, 0, 10, 10),
                                                agg::trans_affine(),
                                                0,
                                                NAN,
                                                0,
                                                false,
                                                false,
                                                DIRECTION_AUTO,
                                                1.0};

        placement_type placement(va, detector, params);

        double x, y, angle;

        CHECK(placement.get_point(x, y, angle, true));
        CHECK(x == Approx(1.5));
        CHECK(y == Approx(1.5));
        CHECK(angle == Approx(0));

        CHECK(!placement.get_point(x, y, angle, true));
    }

    SECTION("line string zero length")
    {
        mapnik::geometry::line_string<double> g;
        g.emplace_back(1.0, 1.0);
        g.emplace_back(1.0, 1.0);
        using va_type = mapnik::geometry::line_string_vertex_adapter<double>;
        va_type va(g);

        using detector_type = mapnik::label_collision_detector4;
        detector_type detector(mapnik::box2d<double>(0, 0, 100, 100));

        using placement_type = mapnik::markers_point_placement<va_type, detector_type>;

        mapnik::markers_placement_params params{mapnik::box2d<double>(0, 0, 10, 10),
                                                agg::trans_affine(),
                                                0,
                                                NAN,
                                                0,
                                                false,
                                                false,
                                                DIRECTION_AUTO,
                                                1.0};

        placement_type placement(va, detector, params);

        double x, y, angle;

        CHECK(placement.get_point(x, y, angle, true));
        CHECK(x == Approx(1.0));
        CHECK(y == Approx(1.0));
        CHECK(angle == Approx(0));

        CHECK(!placement.get_point(x, y, angle, true));
    }
}
