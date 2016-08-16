
#include "catch.hpp"

#include <mapnik/coord.hpp>
#include <mapnik/box2d.hpp>
#include "agg_trans_affine.h"

TEST_CASE("box2d") {
SECTION("coord init") {
  auto c = mapnik::coord2d(100, 100);

  REQUIRE(c.x == 100);
  REQUIRE(c.y == 100);
}

SECTION("coord multiplication") {
  auto c = mapnik::coord2d(100, 100);
  c *= 2;

  REQUIRE(c.x == 200);
  REQUIRE(c.y == 200);
}

SECTION("envelope init") {
  auto e = mapnik::box2d<double>(100, 100, 200, 200);

  REQUIRE(e.contains(100, 100));
  REQUIRE(e.contains(100, 200));
  REQUIRE(e.contains(200, 200));
  REQUIRE(e.contains(200, 100));

  REQUIRE(e.contains(e.center()));

  REQUIRE(!e.contains(99.9, 99.9));
  REQUIRE(!e.contains(99.9, 200.1));
  REQUIRE(!e.contains(200.1, 200.1));
  REQUIRE(!e.contains(200.1, 99.9));

  REQUIRE(e.width() == 100);
  REQUIRE(e.height() == 100);

  REQUIRE(e.minx() == 100);
  REQUIRE(e.miny() == 100);

  REQUIRE(e.maxx() == 200);
  REQUIRE(e.maxy() == 200);

  REQUIRE(e[0] == 100);
  REQUIRE(e[1] == 100);
  REQUIRE(e[2] == 200);
  REQUIRE(e[3] == 200);
  REQUIRE(e[0] == e[-4]);
  REQUIRE(e[1] == e[-3]);
  REQUIRE(e[2] == e[-2]);
  REQUIRE(e[3] == e[-1]);

  auto c = e.center();

  REQUIRE(c.x == 150);
  REQUIRE(c.y == 150);
}

SECTION("envelope static init") {
  auto e = mapnik::box2d<double>(100, 100, 200, 200);

  mapnik::box2d<double> e1, e2, e3;
  REQUIRE(e1.from_string("100 100 200 200"));
  REQUIRE(e2.from_string("100,100,200,200"));
  REQUIRE(e3.from_string("100 , 100 , 200 , 200"));

  REQUIRE(e == e1);
  REQUIRE(e == e2);
  REQUIRE(e == e3);
}

SECTION("envelope multiplication") {
  // no width then no impact of multiplication
  {
    auto a = mapnik::box2d<int>(100, 100, 100, 100);
    a *= 5;

    REQUIRE(a.minx() == 100);
    REQUIRE(a.miny() == 100);
    REQUIRE(a.maxx() == 100);
    REQUIRE(a.maxy() == 100);
  }

  {
    auto a = mapnik::box2d<double>(100.0, 100.0, 100.0, 100.0);
    a *= 5;

    REQUIRE(a.minx() == 100);
    REQUIRE(a.miny() == 100);
    REQUIRE(a.maxx() == 100);
    REQUIRE(a.maxy() == 100);
  }

  {
    auto a = mapnik::box2d<double>(100.0, 100.0, 100.001, 100.001);
    a *= 5;

    REQUIRE(a.minx() == Approx( 99.9980));
    REQUIRE(a.miny() == Approx( 99.9980));
    REQUIRE(a.maxx() == Approx(100.0030));
    REQUIRE(a.maxy() == Approx(100.0030));
  }

  {
    auto e = mapnik::box2d<double>(100, 100, 200, 200);
    e *= 2;

    REQUIRE(e.minx() == 50);
    REQUIRE(e.miny() == 50);
    REQUIRE(e.maxx() == 250);
    REQUIRE(e.maxy() == 250);

    REQUIRE(e.contains(50, 50));
    REQUIRE(e.contains(50, 250));
    REQUIRE(e.contains(250, 250));
    REQUIRE(e.contains(250, 50));

    REQUIRE(!e.contains(49.9, 49.9));
    REQUIRE(!e.contains(49.9, 250.1));
    REQUIRE(!e.contains(250.1, 250.1));
    REQUIRE(!e.contains(250.1, 49.9));

    REQUIRE(e.contains(e.center()));

    REQUIRE(e.width() == 200);
    REQUIRE(e.height() == 200);

    REQUIRE(e.minx() == 50);
    REQUIRE(e.miny() == 50);

    REQUIRE(e.maxx() == 250);
    REQUIRE(e.maxy() == 250);

    auto c = e.center();

    REQUIRE(c.x == 150);
    REQUIRE(c.y == 150);
  }
}

SECTION("envelope clipping") {
  auto e1 = mapnik::box2d<double>(-180,-90,180,90);
  auto e2 = mapnik::box2d<double>(-120,40,-110,48);
  e1.clip(e2);
  REQUIRE(e1 == e2);

  // madagascar in merc
  e1 = mapnik::box2d<double>(4772116.5490, -2744395.0631, 5765186.4203, -1609458.0673);
  e2 = mapnik::box2d<double>(5124338.3753, -2240522.1727, 5207501.8621, -2130452.8520);
  e1.clip(e2);
  REQUIRE(e1 == e2);

  // nz in lon/lat
  e1 = mapnik::box2d<double>(163.8062, -47.1897, 179.3628, -33.9069);
  e2 = mapnik::box2d<double>(173.7378, -39.6395, 174.4849, -38.9252);
  e1.clip(e2);
  REQUIRE(e1 == e2);
}

SECTION("mapnik::box2d intersects")
{
    mapnik::box2d<double> b0(0,0,100,100);
    // another box2d
    mapnik::box2d<double> b1(100,100,200,200);
    CHECK(b0.intersects(b1));
    CHECK(b1.intersects(b0));
    mapnik::box2d<double> b2(100.001,100,200,200);
    CHECK(!b0.intersects(b2));
    CHECK(!b2.intersects(b0));
    // coord
    CHECK(b0.intersects(mapnik::coord<double,2>(100,100)));
    CHECK(!b0.intersects(mapnik::coord<double,2>(100.001,100)));
}

SECTION("mapnik::box2d intersect")
{
    mapnik::box2d<double> b0(0,0,100,100);
    mapnik::box2d<double> b1(100,100,200,200);
    CHECK(b0.intersect(b1) == mapnik::box2d<double>(100,100,100,100));
    CHECK(b1.intersect(b0) == mapnik::box2d<double>(100,100,100,100));
    mapnik::box2d<double> b2(100.001,100,200,200);
    CHECK(b0.intersect(b2) == mapnik::box2d<double>());
    CHECK(b2.intersect(b0) == mapnik::box2d<double>());
}

SECTION("mapnik::box2d re_center")
{
    mapnik::box2d<double> b(0, 0, 100, 100);
    b.re_center(0, 0);
    CHECK(b == mapnik::box2d<double>(-50, -50, 50, 50));
    b.re_center(mapnik::coord2d(50,50));
    CHECK(b == mapnik::box2d<double>(0, 0, 100, 100));
}

SECTION("mapnik::box2d operator+=")
{
    mapnik::box2d<double> b(0, 0, 50, 50);
    b += mapnik::box2d<double>(100, 100, 200, 200);
    CHECK(b == mapnik::box2d<double>(0, 0, 200, 200));
    b += 100;
    CHECK(b == mapnik::box2d<double>(-100, -100, 300, 300));
}

SECTION("mapnik::box2d operator*=  operator=/ ")
{
    mapnik::box2d<double> b(0, 0, 100, 100);
    b *= 2.0;
    CHECK(b == mapnik::box2d<double>(-50, -50, 150, 150));
    b /= 2.0;
    CHECK(b == mapnik::box2d<double>(0, 0, 100, 100));

    agg::trans_affine tr;
    tr.translate(-50,-50);
    tr.scale(2.0);
    b *= tr;
    CHECK(b == mapnik::box2d<double>(-100, -100, 100, 100));
}
} // TEST_CASE
