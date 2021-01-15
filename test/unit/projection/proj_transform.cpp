#include "catch.hpp"

#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/geometry/box2d.hpp>

#ifdef MAPNIK_USE_PROJ4
// proj4
#include <proj_api.h>
#endif


TEST_CASE("projection transform")
{

SECTION("Test bounding box transforms - 4326 to 3857")
{
    mapnik::projection proj_4326("epsg:4326");
    mapnik::projection proj_3857("epsg:3857");
    mapnik::proj_transform prj_trans(proj_4326, proj_3857);

    double minx = -45.0;
    double miny = 55.0;
    double maxx = -40.0;
    double maxy = 75.0;

    mapnik::box2d<double> bbox(minx, miny, maxx, maxy);
    INFO(bbox.to_string());

    CHECKED_IF(prj_trans.forward(bbox))
    {
        CHECK(bbox.minx() == Approx(-5009377.085697311));
        CHECK(bbox.miny() == Approx(7361866.1130511891));
        CHECK(bbox.maxx() == Approx(-4452779.631730943));
        CHECK(bbox.maxy() == Approx(12932243.1119920239));
    }

    CHECKED_IF(prj_trans.backward(bbox))
    {
        CHECK(bbox.minx() == Approx(minx));
        CHECK(bbox.miny() == Approx(miny));
        CHECK(bbox.maxx() == Approx(maxx));
        CHECK(bbox.maxy() == Approx(maxy));
    }
}


#if defined(MAPNIK_USE_PROJ4) && PJ_VERSION >= 480
SECTION("test pj_transform failure behavior")
{
    mapnik::projection proj_4269("epsg:4269");
    mapnik::projection proj_3857("epsg:3857");
    mapnik::proj_transform prj_trans(proj_4269, proj_3857);
    mapnik::proj_transform prj_trans2(proj_3857, proj_4269);

    auto proj_ctx0 = pj_ctx_alloc();
    REQUIRE( proj_ctx0 != nullptr );
    auto proj0 = pj_init_plus_ctx(proj_ctx0, proj_4269.params().c_str());
    REQUIRE( proj0 != nullptr );

    auto proj_ctx1 = pj_ctx_alloc();
    REQUIRE( proj_ctx1 != nullptr );
    auto proj1 = pj_init_plus_ctx(proj_ctx1, proj_3857.params().c_str());
    REQUIRE( proj1 != nullptr );

    // first test valid values directly against proj
    double x = -180.0;
    double y = -60.0;
    x *= DEG_TO_RAD;
    y *= DEG_TO_RAD;
    CHECK( x == Approx(-3.1415926536) );
    CHECK( y == Approx(-1.0471975512) );
    CHECK( 0 == pj_transform(proj0, proj1, 1, 0, &x, &y, nullptr) );
    CHECK( x == Approx(-20037508.3427892439) );
    CHECK( y == Approx(-8399737.8896366451) );

    // now test mapnik class
    double x0 = -180.0;
    double y0 = -60.0;
    CHECK( prj_trans.forward(&x0,&y0,nullptr,1,1) );
    CHECK( x0 == Approx(-20037508.3427892439) );
    CHECK( y0 == Approx(-8399737.8896366451) );
    double x1 = -180.0;
    double y1 = -60.0;
    CHECK( prj_trans2.backward(&x1,&y1,nullptr,1,1) );
    CHECK( x1 == Approx(-20037508.3427892439) );
    CHECK( y1 == Approx(-8399737.8896366451) );

    // longitude value outside the value range for mercator
    x = -181.0;
    y = -91.0;
    x *= DEG_TO_RAD;
    y *= DEG_TO_RAD;
    CHECK( x == Approx(-3.1590459461) );
    CHECK( y == Approx(-1.5882496193) );
    CHECK( 0 == pj_transform(proj0, proj1, 1, 0, &x, &y, nullptr) );
    CHECK( std::isinf(x) );
    CHECK( std::isinf(y) );

    // now test mapnik class
    double x2 = -181.0;
    double y2 = -91.0;
    CHECK( false == prj_trans.forward(&x2,&y2,nullptr,1,1) );
    CHECK( std::isinf(x2) );
    CHECK( std::isinf(y2) );
    double x3 = -181.0;
    double y3 = -91.0;
    CHECK( false == prj_trans2.backward(&x3,&y3,nullptr,1,1) );
    CHECK( std::isinf(x3) );
    CHECK( std::isinf(y3) );

    // cleanup
    pj_ctx_free(proj_ctx0);
    proj_ctx0 = nullptr;
    pj_free(proj0);
    proj0 = nullptr;
    pj_ctx_free(proj_ctx1);
    proj_ctx1 = nullptr;
    pj_free(proj1);
    proj1 = nullptr;
}

#endif

// Github Issue https://github.com/mapnik/mapnik/issues/2648
SECTION("Test proj antimeridian bbox")
{
    mapnik::projection prj_geog("epsg:4326");
    mapnik::projection prj_proj("epsg:2193");

    mapnik::proj_transform prj_trans_fwd(prj_proj, prj_geog);
    mapnik::proj_transform prj_trans_rev(prj_geog, prj_proj);

    // reference values taken from proj4 command line tool:
    // (non-corner points assume PROJ_ENVELOPE_POINTS == 20)
    //
    //  cs2cs -Ef %.10f epsg:2193 +to epsg:4326 <<END
    //        2105800 3087000 # left-most
    //        1495200 3087000 # bottom-most
    //        2105800 7173000 # right-most
    //        3327000 7173000 # top-most
    //  END
    //
    // wrong = mapnik.Box2d(-177.3145325044, -62.3337481525,
    //                       178.0277836332, -24.5845974912)
    const mapnik::box2d<double> better(-180.0, -62.3337481525,
                                        180.0, -24.5845974912);

    {
        mapnik::box2d<double> ext(274000, 3087000, 3327000, 7173000);
        CHECKED_IF(prj_trans_fwd.forward(ext, PROJ_ENVELOPE_POINTS))
        {
            CHECK(ext.minx() == Approx(better.minx()));
            CHECK(ext.miny() == Approx(better.miny()));
            CHECK(ext.maxx() == Approx(better.maxx()));
            CHECK(ext.maxy() == Approx(better.maxy()));
        }
    }

    {
        // check the same logic works for .backward()
        mapnik::box2d<double> ext(274000, 3087000, 3327000, 7173000);
        CHECKED_IF(prj_trans_rev.backward(ext, PROJ_ENVELOPE_POINTS))
        {
            CHECK(ext.minx() == Approx(better.minx()));
            CHECK(ext.miny() == Approx(better.miny()));
            CHECK(ext.maxx() == Approx(better.maxx()));
            CHECK(ext.maxy() == Approx(better.maxy()));
        }
    }

    // reference values taken from proj4 command line tool:
    //
    //  cs2cs -Ef %.10f epsg:2193 +to epsg:4326 <<END
    //        274000 3087000 # left-most
    //        276000 3087000 # bottom-most
    //        276000 7173000 # right-most
    //        274000 7173000 # top-most
    //  END
    //
    const mapnik::box2d<double> normal(148.7639922894, -60.1222810241,
                                       159.9548489296, -24.9771195155);

    {
        // checks for not being snapped (ie. not antimeridian)
        mapnik::box2d<double> ext(274000, 3087000, 276000, 7173000);
        CHECKED_IF(prj_trans_fwd.forward(ext, PROJ_ENVELOPE_POINTS))
        {
            CHECK(ext.minx() == Approx(normal.minx()));
            CHECK(ext.miny() == Approx(normal.miny()));
            CHECK(ext.maxx() == Approx(normal.maxx()));
            CHECK(ext.maxy() == Approx(normal.maxy()));
        }
    }

    {
        // check the same logic works for .backward()
        mapnik::box2d<double> ext(274000, 3087000, 276000, 7173000);
        CHECKED_IF(prj_trans_rev.backward(ext, PROJ_ENVELOPE_POINTS))
        {
            CHECK(ext.minx() == Approx(normal.minx()));
            CHECK(ext.miny() == Approx(normal.miny()));
            CHECK(ext.maxx() == Approx(normal.maxx()));
            CHECK(ext.maxy() == Approx(normal.maxy()));
        }
    }
}

SECTION("proj_transform of coordinate arrays with stride > 1")
{
    mapnik::projection const proj_4326("epsg:4326");
    mapnik::projection const proj_3857("epsg:3857");
    mapnik::projection const proj_2193("epsg:2193");

    SECTION("lonlat <-> Web Mercator")
    {
        //  cs2cs -Ef %.10f epsg:4326 +to epsg:3857 <<END
        //      170.142139 -43.595056
        //      175.566667 -39.283333
        //  END
        //
        //  170.142139 -43.595056   18940136.2759583741 -5402988.5324898539
        //  175.566667 -39.283333   19543991.9707122259 -4762338.2380718365
        //
        mapnik::geometry::point<double> points[] = {{ 170.142139, -43.595056 },
                                                    { 175.566667, -39.283333 }};
        // this transform is calculated by Mapnik (well_known_srs.cpp)
        mapnik::proj_transform lonlat_to_webmerc(proj_4326, proj_3857);
        CHECKED_IF(lonlat_to_webmerc.forward(&points[0].x, &points[0].y, nullptr, 2, 2))
        {
            CHECK(points[0].x == Approx(18940136.2759583741));
            CHECK(points[0].y == Approx(-5402988.5324898539));
            CHECK(points[1].x == Approx(19543991.9707122259));
            CHECK(points[1].y == Approx(-4762338.2380718365));
        }
        CHECKED_IF(lonlat_to_webmerc.backward(&points[0].x, &points[0].y, nullptr, 2, 2))
        {
            CHECK(points[0].x == Approx(170.142139));
            CHECK(points[0].y == Approx(-43.595056));
            CHECK(points[1].x == Approx(175.566667));
            CHECK(points[1].y == Approx(-39.283333));
        }
    }

    #ifdef MAPNIK_USE_PROJ4
    SECTION("lonlat <-> New Zealand Transverse Mercator 2000")
    {
        //  cs2cs -Ef %.10f epsg:4326 +to epsg:2193 <<END
        //      170.142139 -43.595056
        //      175.566667 -39.283333
        //  END
        //
        //  170.142139 -43.595056   1369316.0970041484  5169132.9750701785
        //  175.566667 -39.283333   1821377.9170061364  5648640.2106032455
        //
        mapnik::geometry::point<double> points[] = {{ 170.142139, -43.595056 },
                                                    { 175.566667, -39.283333 }};
        // this transform is not calculated by Mapnik (needs Proj4)
        mapnik::proj_transform lonlat_to_nztm(proj_4326, proj_2193);
        CHECKED_IF(lonlat_to_nztm.forward(&points[0].x, &points[0].y, nullptr, 2, 2))
        {
            CHECK(points[0].x == Approx(1369316.0970041484));
            CHECK(points[0].y == Approx(5169132.9750701785));
            CHECK(points[1].x == Approx(1821377.9170061364));
            CHECK(points[1].y == Approx(5648640.2106032455));
        }
        CHECKED_IF(lonlat_to_nztm.backward(&points[0].x, &points[0].y, nullptr, 2, 2))
        {
            CHECK(points[0].x == Approx(170.142139));
            CHECK(points[0].y == Approx(-43.595056));
            CHECK(points[1].x == Approx(175.566667));
            CHECK(points[1].y == Approx(-39.283333));
        }
    }
    #endif // MAPNIK_USE_PROJ4
}

}
