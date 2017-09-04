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
    mapnik::projection proj_4326("+init=epsg:4326");
    mapnik::projection proj_3857("+init=epsg:3857");
    mapnik::proj_transform prj_trans(proj_4326, proj_3857);

    double minx = -45.0;
    double miny = 55.0;
    double maxx = -40.0;
    double maxy = 75.0;

    mapnik::box2d<double> bbox(minx, miny, maxx, maxy);

    prj_trans.forward(bbox);
    INFO(bbox.to_string());
    CHECK(bbox.minx() == Approx(-5009377.085697311));
    CHECK(bbox.miny() == Approx(7361866.1130511891));
    CHECK(bbox.maxx() == Approx(-4452779.631730943));
    CHECK(bbox.maxy() == Approx(12932243.1119920239));

    prj_trans.backward(bbox);
    CHECK(bbox.minx() == Approx(minx));
    CHECK(bbox.miny() == Approx(miny));
    CHECK(bbox.maxx() == Approx(maxx));
    CHECK(bbox.maxy() == Approx(maxy));

}


#if defined(MAPNIK_USE_PROJ4) && PJ_VERSION >= 480
SECTION("test pj_transform failure behavior")
{
    mapnik::projection proj_4269("+init=epsg:4269");
    mapnik::projection proj_3857("+init=epsg:3857");
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
    mapnik::projection prj_geog("+init=epsg:4326");
    mapnik::projection prj_proj("+init=epsg:2193");

    mapnik::proj_transform prj_trans_fwd(prj_proj, prj_geog);
    mapnik::proj_transform prj_trans_rev(prj_geog, prj_proj);

    // bad = mapnik.Box2d(-177.31453250437079, -62.33374815225163, 178.02778363316355, -24.584597490955804)
    const mapnik::box2d<double> better(-180.0, -62.33374815225163,
                                        180.0, -24.584597490955804);

    {
        mapnik::box2d<double> ext(274000, 3087000, 3327000, 7173000);
        prj_trans_fwd.forward(ext, PROJ_ENVELOPE_POINTS);
        CHECK(ext.minx() == Approx(better.minx()));
        CHECK(ext.miny() == Approx(better.miny()));
        CHECK(ext.maxx() == Approx(better.maxx()));
        CHECK(ext.maxy() == Approx(better.maxy()));
    }

    {
        // check the same logic works for .backward()
        mapnik::box2d<double> ext(274000, 3087000, 3327000, 7173000);
        prj_trans_rev.backward(ext, PROJ_ENVELOPE_POINTS);
        CHECK(ext.minx() == Approx(better.minx()));
        CHECK(ext.miny() == Approx(better.miny()));
        CHECK(ext.maxx() == Approx(better.maxx()));
        CHECK(ext.maxy() == Approx(better.maxy()));
    }

    {
        // checks for not being snapped (ie. not antimeridian)
        mapnik::box2d<double> ext(274000, 3087000, 3327000, 7173000);
        prj_trans_rev.backward(ext, PROJ_ENVELOPE_POINTS);
        CHECK(ext.minx() == Approx(better.minx()));
        CHECK(ext.miny() == Approx(better.miny()));
        CHECK(ext.maxx() == Approx(better.maxx()));
        CHECK(ext.maxy() == Approx(better.maxy()));
    }

    const mapnik::box2d<double> normal(148.766759749, -60.1222810238,
                                       159.95484893, -24.9774643167);

    {
        // checks for not being snapped (ie. not antimeridian)
        mapnik::box2d<double> ext(274000, 3087000, 276000, 7173000);
        prj_trans_fwd.forward(ext, PROJ_ENVELOPE_POINTS);
        CHECK(ext.minx() == Approx(normal.minx()));
        CHECK(ext.miny() == Approx(normal.miny()));
        CHECK(ext.maxx() == Approx(normal.maxx()));
        CHECK(ext.maxy() == Approx(normal.maxy()));
    }

    {
        // check the same logic works for .backward()
        mapnik::box2d<double> ext(274000, 3087000, 276000, 7173000);
        prj_trans_rev.backward(ext, PROJ_ENVELOPE_POINTS);
        CHECK(ext.minx() == Approx(normal.minx()));
        CHECK(ext.miny() == Approx(normal.miny()));
        CHECK(ext.maxx() == Approx(normal.maxx()));
        CHECK(ext.maxy() == Approx(normal.maxy()));
    }
}

}
