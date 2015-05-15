#include "catch.hpp"

#include <mapnik/palette.hpp>

TEST_CASE("palette")
{

SECTION("rgb")
{
    mapnik::rgb a(1,2,3);
    mapnik::rgba b_(1,2,3,4);
    mapnik::rgb b(b_);
    mapnik::rgb c(3,4,5);
    CHECK(a == b);
    CHECK_FALSE(a == c);

} // END SECTION


SECTION("rgba")
{
    mapnik::rgba a(1,2,3,4);
    mapnik::rgba a_copy(1,2,3,4);
    mapnik::rgba b(255,255,255,255);
    mapnik::rgba c(4,3,2,1);
    mapnik::rgb d_(1,2,3);
    mapnik::rgba d(d_);

    mapnik::rgba e(16909060);

    CHECK(e.r == 4);
    CHECK(e.g == 3);
    CHECK(e.b == 2);
    CHECK(e.a == 1);

    CHECK(a == a_copy);
    CHECK_FALSE(a == c);
    CHECK(c == e);
    
    mapnik::rgba::mean_sort_cmp msc;

    CHECK_FALSE(msc(a,a_copy));
    CHECK(msc(a,b));
    CHECK_FALSE(msc(a,c));
    CHECK(msc(a,d));

} // END SECTION

} // END TEST CASE
