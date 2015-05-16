#include "catch.hpp"

#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/box2d.hpp>

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

} // END SECTION

} // END TEST CASE
