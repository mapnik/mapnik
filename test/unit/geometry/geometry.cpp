#include "catch.hpp"

#include <mapnik/geometry.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/json/geometry_parser.hpp>
#include <mapnik/util/geometry_to_geojson.hpp>

TEST_CASE("geometry") {

SECTION("json point") {
    mapnik::util::file input("./test/data/json/point1.json");
    REQUIRE( input.open() );
    mapnik::geometry::geometry<double> geom;
    REQUIRE( input.data() );
    std::string json_string(input.data().get(), input.size());
    REQUIRE( mapnik::json::from_geojson(json_string, geom) );
    REQUIRE( geom.is<mapnik::geometry::point<double> >() );
    auto const& point = mapnik::util::get<mapnik::geometry::point<double> >(geom);
    REQUIRE( point.x == 30 );
    REQUIRE( point.y == 10 );
    std::string new_json;
    REQUIRE( mapnik::util::to_geojson(new_json, geom) );
}

SECTION("json point reversed") {
    mapnik::util::file input("./test/data/json/point2.json");
    REQUIRE( input.open() );
    mapnik::geometry::geometry<double> geom;
    REQUIRE( input.data() );
    std::string json_string(input.data().get(), input.size());
    REQUIRE( mapnik::json::from_geojson(json_string,geom) );
    REQUIRE( geom.is<mapnik::geometry::point<double> >() );
    auto const& point = mapnik::util::get<mapnik::geometry::point<double> >(geom);
    REQUIRE( point.x == 30 );
    REQUIRE( point.y == 10 );
}

}
