
#include "catch.hpp"

#include <mapnik/geometry_impl.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/json/geometry_parser.hpp>
//#include <mapnik/util/geometry_to_geojson.hpp>
#include <mapnik/json/geometry_generator_grammar.hpp>
#include <mapnik/json/geometry_generator_grammar_impl.hpp>

TEST_CASE("geometry") {

SECTION("json") {
    mapnik::util::file input("./tests/data/json/fixtures/point1.json");
    auto json = input.data();
    mapnik::new_geometry::geometry geom;
    std::string json_string(json.get());
    REQUIRE( mapnik::json::from_geojson(json_string,geom) );
    if (geom.is<mapnik::new_geometry::point>()) {
        auto const& point = mapnik::util::get<mapnik::new_geometry::point>(geom);
        REQUIRE( point.x == 30 );
        REQUIRE( point.y == 10 );
        using adapter_type = mapnik::new_geometry::point_vertex_adapter;
        adapter_type va(point);
        std::string new_json;
        using sink_type = std::back_insert_iterator<std::string>;
        // TODO: need to round trip, but does not compile yet
        /*
        static const mapnik::json::geometry_generator_grammar<sink_type, adapter_type> grammar;
        sink_type sink(new_json);
        REQUIRE( boost::spirit::karma::generate(sink, grammar, va) );
        */
        //REQUIRE( mapnik::util::to_geojson(new_json,va) );
    }
}

}
