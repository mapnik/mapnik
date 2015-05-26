#include "catch.hpp"
#include "geometry_equal.hpp"

#include <mapnik/geometry.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/geometry_transform.hpp>
#include <mapnik/geometry_strategy.hpp>
#include <mapnik/proj_strategy.hpp>
#include <mapnik/view_strategy.hpp>

TEST_CASE("geometry strategy tests") {

SECTION("proj and view strategy") {
    using namespace mapnik::geometry;
    mapnik::box2d<double> e(-20037508.342789,-20037508.342789,20037508.342789,20037508.342789);
    mapnik::view_transform vt(256, 256, e);
    mapnik::view_strategy vs(vt);
    mapnik::unview_strategy uvs(vt);
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans(source, dest);
    mapnik::proj_transform proj_trans_rev(dest, source);
    mapnik::proj_strategy ps(proj_trans);
    mapnik::proj_strategy ps_rev(proj_trans_rev);
    {
        // Test first that proj strategy works properly
        point<double> p1(-97.553098,35.523105);
        point<double> r1(-1.08596e+07, 4.2352e+06);
        point<double> p3 = transform<double>(p1, ps);
        assert_g_equal(r1, p3);
    }
    {
        // Test next that view_strategy works
        point<double> p1(-1.08596e+07, 4.2352e+06);
        point<double> r1(58.6287 , 100.945);
        point<double> p3 = transform<double>(p1, vs);
        assert_g_equal(r1, p3);

    }
    {
        // Test next that view_strategy works as single process in strategy group
        point<double> p1(-1.08596e+07, 4.2352e+06);
        point<double> r1(58.6287 , 100.945);
        using sg_type = strategy_group<mapnik::view_strategy>;
        sg_type sg(vs);
        point<double> p3 = transform<double>(p1, sg);
        assert_g_equal(r1, p3);

    }
    {
        // Test that both work grouped together in strategy group
        using sg_type = strategy_group<mapnik::proj_strategy, mapnik::view_strategy >;
        sg_type sg(ps, vs);
        point<double> p1(-97.553098,35.523105);
        point<double> r1(58.6287 , 100.945);
        point<double> p3 = transform<double>(p1, sg);
        assert_g_equal(r1, p3);
    }
    {
        // Test that both work grouped together passing in geometry
        using sg_type = strategy_group<mapnik::proj_strategy, mapnik::view_strategy >;
        sg_type sg(ps, vs);
        geometry<double> p1(std::move(point<double>(-97.553098,35.523105)));
        point<double> r1(58.6287 , 100.945);
        geometry<double> p2 = transform<double>(p1, sg);
        REQUIRE(p2.is<point<double> >());
        point<double> p3 = mapnik::util::get<point<double> >(p2);
        assert_g_equal(r1, p3);
    }
    {
        // Test that it works pulling back int
        using sg_type = strategy_group<mapnik::proj_strategy, mapnik::view_strategy >;
        sg_type sg(ps, vs);
        geometry<double> p1(std::move(point<double>(-97.553098,35.523105)));
        point<std::int64_t> r1(58 , 100);
        geometry<std::int64_t> p2 = transform<std::int64_t>(p1, sg);
        REQUIRE(p2.is<point<std::int64_t> >());
        point<std::int64_t> p3 = mapnik::util::get<point<std::int64_t> >(p2);
        assert_g_equal(r1, p3);
    }
    {
        // Test with scaling as well. This would be like projection from 4326 to a vector tile.
        mapnik::geometry::scale_rounding_strategy ss(16);
        using sg_type = strategy_group<mapnik::proj_strategy, mapnik::view_strategy, mapnik::geometry::scale_rounding_strategy >;
        sg_type sg(ps, vs, ss);
        geometry<double> p1(std::move(point<double>(-97.553098,35.523105)));
        point<std::int64_t> r1(938 , 1615);
        geometry<std::int64_t> p2 = transform<std::int64_t>(p1, sg);
        REQUIRE(p2.is<point<std::int64_t> >());
        point<std::int64_t> p3 = mapnik::util::get<point<std::int64_t> >(p2);
        assert_g_equal(r1, p3);
    }
    {
        // Test the entire process in reverse! This would be like converting a vector tile coordinate to 4326.
        mapnik::geometry::scale_strategy ss(1.0/16.0);
        using sg_type = strategy_group_first<mapnik::geometry::scale_strategy, mapnik::unview_strategy, mapnik::proj_strategy >;
        sg_type sg(ss, uvs, ps_rev);
        geometry<std::int64_t> p1(std::move(point<std::int64_t>(938 , 1615)));
        point<double> r1(-97.5586 , 35.5322);
        geometry<double> p2 = transform<double>(p1, sg);
        REQUIRE(p2.is<point<double> >());
        point<double> p3 = mapnik::util::get<point<double> >(p2);
        assert_g_equal(r1, p3);
    }
    {
        // Test with scaling + offset as well. This would be like projection from 4326 to a vector tile.
        mapnik::geometry::scale_rounding_strategy ss(16, 20);
        using sg_type = strategy_group<mapnik::proj_strategy, mapnik::view_strategy, mapnik::geometry::scale_rounding_strategy >;
        sg_type sg(ps, vs, ss);
        geometry<double> p1(std::move(point<double>(-97.553098,35.523105)));
        point<std::int64_t> r1(958 , 1635);
        geometry<std::int64_t> p2 = transform<std::int64_t>(p1, sg);
        REQUIRE(p2.is<point<std::int64_t> >());
        point<std::int64_t> p3 = mapnik::util::get<point<std::int64_t> >(p2);
        assert_g_equal(r1, p3);
    }
    {
        // Test the entire scaling plus offset in reverse process in reverse! This would be like converting a vector tile coordinate to 4326.
        mapnik::geometry::scale_strategy ss(1.0/16.0, -20.0/16.0);
        using sg_type = strategy_group_first<mapnik::geometry::scale_strategy, mapnik::unview_strategy, mapnik::proj_strategy >;
        sg_type sg(ss, uvs, ps_rev);
        geometry<std::int64_t> p1(std::move(point<std::int64_t>(958 , 1635)));
        point<double> r1(-97.5586 , 35.5322);
        geometry<double> p2 = transform<double>(p1, sg);
        REQUIRE(p2.is<point<double> >());
        point<double> p3 = mapnik::util::get<point<double> >(p2);
        assert_g_equal(r1, p3);
    }

} // END SECTION

SECTION("scaling strategies - double to double") {
    using namespace mapnik::geometry;
 
    {   
        scale_strategy ss(10.0);
        point<double> p(-90.3, 35.5);
        point<double> r(-903.0, 355.0);
        point<double> o = transform<double>(p, ss);
        assert_g_equal(r, o);
    }
    {   
        scale_strategy ss(0.5, -2.0);
        point<double> p(-90.3, 35.5);
        point<double> r(-47.15, 15.75);
        point<double> o = transform<double>(p, ss);
        assert_g_equal(r, o);
    }
    {   
        scale_rounding_strategy ss(0.5, -2.0);
        point<double> p(-90.3, 35.5);
        point<double> r(-47.0, 16.0);
        point<double> o = transform<double>(p, ss);
        assert_g_equal(r, o);
    }

} // END SECTION

SECTION("scaling strategies - double to int64") {
    using namespace mapnik::geometry;
 
    {   
        scale_strategy ss(10.0);
        point<double> p(-90.31, 35.58);
        point<std::int64_t> r(-903, 355);
        point<std::int64_t> o = transform<std::int64_t>(p, ss);
        assert_g_equal(r, o);
    }
    {   
        scale_strategy ss(0.5, -2.0);
        point<double> p(-90.3, 35.5);
        point<std::int64_t> r(-47, 15);
        point<std::int64_t> o = transform<std::int64_t>(p, ss);
        assert_g_equal(r, o);
    }
    {   
        scale_rounding_strategy ss(0.5, -2.0);
        point<double> p(-90.3, 35.5);
        point<std::int64_t> r(-47, 16);
        point<std::int64_t> o = transform<std::int64_t>(p, ss);
        assert_g_equal(r, o);
    }
} // END SECTION

} // END TEST CASE


