#include "catch.hpp"
#include "geometry_equal.hpp"

#include <mapnik/geometry.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/geometry_transform.hpp>
#include <mapnik/geometry_strategy.hpp>

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
        geometry<double> p2 = transform<double>(p1, ps);
        REQUIRE(p2.is<point<double> >());
        point<double> p3 = mapnik::util::get<point<double> >(p2);
        //std::cout << p3.x << " , " << p3.y << std::endl;
        assert_g_equal(r1, p3);
    }
    {
        // Test next that view_strategy works
        point<double> p1(-1.08596e+07, 4.2352e+06);
        point<double> r1(58.6287 , 100.945);
        geometry<double> p2 = transform<double>(p1, vs);
        REQUIRE(p2.is<point<double> >());
        point<double> p3 = mapnik::util::get<point<double> >(p2);
        //std::cout << p3.x << " , " << p3.y << std::endl;
        assert_g_equal(r1, p3);

    }
    {
        // Test next that view_strategy works as single process in strategy group
        point<double> p1(-1.08596e+07, 4.2352e+06);
        point<double> r1(58.6287 , 100.945);
        using sg_type = strategy_group<mapnik::view_strategy>;
        sg_type sg(vs);
        geometry<double> p2 = transform<double>(p1, sg);
        REQUIRE(p2.is<point<double> >());
        point<double> p3 = mapnik::util::get<point<double> >(p2);
        //std::cout << p3.x << " , " << p3.y << std::endl;
        assert_g_equal(r1, p3);

    }
    {
        // Test that both work grouped together in strategy group
        using sg_type = strategy_group<mapnik::proj_strategy, mapnik::view_strategy >;
        sg_type sg(ps, vs);
        point<double> p1(-97.553098,35.523105);
        point<double> r1(58.6287 , 100.945);
        geometry<double> p2 = transform<double>(p1, sg);
        REQUIRE(p2.is<point<double> >());
        point<double> p3 = mapnik::util::get<point<double> >(p2);
        //std::cout << p3.x << " , " << p3.y << std::endl;
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
        //std::cout << p3.x << " , " << p3.y << std::endl;
        assert_g_equal(r1, p3);
    }
    {
        // Test that it works pulling back int
        using sg_type = strategy_group<mapnik::proj_strategy, mapnik::view_strategy >;
        sg_type sg(ps, vs);
        geometry<double> p1(std::move(point<double>(-97.553098,35.523105)));
        point<int> r1(58 , 100);
        geometry<int> p2 = transform<int>(p1, sg);
        REQUIRE(p2.is<point<int> >());
        point<int> p3 = mapnik::util::get<point<int> >(p2);
        //std::cout << p3.x << " , " << p3.y << std::endl;
        assert_g_equal(r1, p3);
    }
    {
        // Test with scaling as well. This would be like projection from 4326 to a vector tile.
        mapnik::geometry::scale_strategy ss(16, 0.5);
        using sg_type = strategy_group<mapnik::proj_strategy, mapnik::view_strategy, mapnik::geometry::scale_strategy >;
        sg_type sg(ps, vs, ss);
        geometry<double> p1(std::move(point<double>(-97.553098,35.523105)));
        point<int> r1(938 , 1615);
        geometry<int> p2 = transform<int>(p1, sg);
        REQUIRE(p2.is<point<int> >());
        point<int> p3 = mapnik::util::get<point<int> >(p2);
        //std::cout << p3.x << " , " << p3.y << std::endl;
        assert_g_equal(r1, p3);
    }
    {
        // Test the entire process in reverse! This would be like converting a vector tile coordinate to 4326.
        mapnik::geometry::scale_strategy ss(1.0/16.0);
        using sg_type = strategy_group_first<mapnik::geometry::scale_strategy, mapnik::unview_strategy, mapnik::proj_strategy >;
        sg_type sg(ss, uvs, ps_rev);
        geometry<int> p1(std::move(point<int>(938 , 1615)));
        point<double> r1(-97.5586 , 35.5322);
        geometry<double> p2 = transform<double>(p1, sg);
        REQUIRE(p2.is<point<double> >());
        point<double> p3 = mapnik::util::get<point<double> >(p2);
        std::cout << p3.x << " , " << p3.y << std::endl;
        assert_g_equal(r1, p3);
    }

}

}


