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
    mapnik::view_strategy<double> vs(vt);
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans(source, dest);
    mapnik::proj_strategy ps(proj_trans);
    {
        // Test first that proj strategy works properly
        point<double> p1(-97.553098,35.523105);
        point<double> r1(-1.08596e+07, 4.2352e+06);
        geometry<double> p2 = transform<double>(p1, ps);
        REQUIRE(p2.is<point<double> >());
        point<double> p3 = mapnik::util::get<point<double> >(p2);
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
        // Test that both work streamed together
        using sg_type = strategy_group<mapnik::proj_strategy, mapnik::view_strategy<double> >;
        sg_type sg(ps, vs);
        point<double> p1(-97.553098,35.523105);
        point<double> r1(58.6287 , 100.945);
        geometry<double> p2 = transform<double>(p1, sg);
        REQUIRE(p2.is<point<double> >());
        point<double> p3 = mapnik::util::get<point<double> >(p2);
        std::cout << p3.x << " , " << p3.y << std::endl;
        assert_g_equal(r1, p3);
    }

}

}


