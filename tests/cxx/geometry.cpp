#include "catch.hpp"

#include <mapnik/geometry.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/json/geometry_parser.hpp>
#include <mapnik/util/geometry_to_geojson.hpp>

TEST_CASE("geometry") {

SECTION("json point") {
    mapnik::util::file input("./tests/data/json/fixtures/point1.json");
    auto json = input.data();
    mapnik::new_geometry::geometry geom;
    std::string json_string(json.get());
    REQUIRE( mapnik::json::from_geojson(json_string, geom) );
    REQUIRE( geom.is<mapnik::new_geometry::point>() );
    auto const& point = mapnik::util::get<mapnik::new_geometry::point>(geom);
    REQUIRE( point.x == 30 );
    REQUIRE( point.y == 10 );
    std::string new_json;
    REQUIRE( mapnik::util::to_geojson(new_json, geom) );
}

SECTION("json point reversed") {
    mapnik::util::file input("./tests/data/json/fixtures/point2.json");
    auto json = input.data();
    mapnik::new_geometry::geometry geom;
    std::string json_string(json.get());
    REQUIRE( mapnik::json::from_geojson(json_string,geom) );
    REQUIRE( geom.is<mapnik::new_geometry::point>() );
    auto const& point = mapnik::util::get<mapnik::new_geometry::point>(geom);
    REQUIRE( point.x == 30 );
    REQUIRE( point.y == 10 );
}

}
